#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include <stdlib.h>

#include "includes/file_reader.h"
#include "includes/formatting.h"
#include "includes/output.h"

#if defined(MULTI_PROCESS)
#include <unistd.h>
#include <sys/wait.h>

/* Parte di codice eseguita dal processo "producer"
 * Si occuperà di formattare le righe di una pagina dato il contenuto di un file e il formato desiderato
 * Una volta eseguita la formattazione verrà scritta la pagina formattata sulla pipe in modo da mandarla al processo consumatore
 */
void formatter(int write_fd, file_content *content, struct page_format *format) {
    /* Questi byte vengono scritti prima del contenuto della pagina
     * 0 servirà al processo consumatore per capire quando il produttore ha finito il suo lavoro
     */
    uint8_t print_next_page = 1;
    uint8_t stop = 0;

    while (content->index < content->length) {
        write(write_fd, &print_next_page, sizeof(uint8_t));
        /* Viene generata la pagina */
        struct Row **rows = get_page_rows(content, format);

        /* Viene Scritta sulla pipe la pagina generata e deallocata */
        int i;
        for (i = 0; i < (format->columns * format->column_height); i++) {
            write(write_fd, rows[i], sizeof(struct Row));
            struct Word *current_word = rows[i]->words;
            while (current_word != NULL) {
                write(write_fd, current_word, sizeof(struct Word));
                struct Word *prev_word = current_word;
                current_word = current_word->next_word;
                free(prev_word);
            }
            free(rows[i]);
        }
        free(rows);
    }
    write(write_fd, &stop, sizeof(uint8_t));

    close(write_fd);
}

/* Parte di codice eseguita dal processo "consumer"
 * Si occuperà di stampare su file o stdout le pagine che riceve dal produttore attraverso la pipe
 *
 */
void outputter(int read_fd, file_content *content, struct page_format *format, char *file_path) {
    /* Sceglie dove scrivere l'output
     * In caso il valore di file_path sia NULL scrive su stdout
     */
    FILE *file;
    if (file_path != NULL) {
        file = fopen(file_path, "w");
        if (file == NULL) {
            exit(EXIT_FAILURE);
        }
    }
    else {
        file = stdout;
    }

    /* Byte usato per capire se il produttore ha finito le pagine da produrre*/
    uint8_t next_page;
    read(read_fd, &next_page, sizeof(uint8_t));

    struct Row **rows = malloc(sizeof(struct Row *) * format->columns * format->column_height);
    while (next_page != 0) {
        /* Legge dalla pipe la pagina e la "ricrea" */
        int i;
        for (i = 0; i < (format->columns * format->column_height); i++) {
            rows[i] = malloc(sizeof(struct Row));
            read(read_fd, rows[i], sizeof(struct Row));
            if (rows[i]->words != NULL) {
                rows[i]->words = malloc(sizeof(struct Word));
                read(read_fd, rows[i]->words, sizeof(struct Word));
                struct Word *current_word = rows[i]->words;
                while (current_word != NULL) {
                    if (current_word->next_word != NULL) {
                        current_word->next_word = malloc(sizeof(struct Word));
                        read(read_fd, current_word->next_word, sizeof(struct Word));
                    }
                    current_word = current_word->next_word;
                }
            }
        }
        /* Stampa la pagina */
        print_page(file, content, rows, format);

        /* Libera le righe della pagina dato che non servono più una volta stampate */
        for (i = 0; i < (format->columns * format->column_height); i++) {
            if (rows[i]->words != NULL) {
                struct Word *current_word = rows[i]->words;
                while (current_word != NULL) {
                    struct Word *prev_word = current_word;
                    current_word = current_word->next_word;
                    free(prev_word);
                }
            }
            free(rows[i]);
        }

        /* Viene letto il byte di fine pagina e in caso viene stampato il separatore */
        read(read_fd, &next_page, sizeof(uint8_t));
        if (next_page != 0)
            fprintf (file, "\n %%%%%%\n");
    }
    free(rows);
    close(read_fd);
}
#endif

int main(int argc, char **argv) {
    /* Inizializzazione dei valori di default */
    char *input_file = NULL;
    char *output_file = NULL;
    struct page_format format = {3, 32, 25, 8};

    /* Parsing delle opzioni */
    const char *short_opt = "o:c:r:s:l:hv";
    struct option long_opt[] = {
            {"output",  required_argument, NULL, 'o'},
            {"columns", required_argument, NULL, 'c'},
            {"rows",    required_argument, NULL, 'r'},
            {"spaces",  required_argument, NULL, 's'},
            {"length",  required_argument, NULL, 'l'},
            {"help",    no_argument,       NULL, 'h'},
            {"version", no_argument,       NULL, 'v'},
            {NULL, 0,                      NULL, 0}
    };

    int arg_count;
    while ((arg_count = getopt_long(argc, argv, short_opt, long_opt, NULL)) != -1) {
        switch (arg_count) {
            case -1:
            case 0:
                break;

            case 'o':
                output_file = optarg;
                break;

            case 'c':
                format.columns = atoi(optarg);
                if (format.columns <= 0) {
                    return(-2);
                }
                break;

            case 'r':
                format.column_height = atoi(optarg);
                if (format.column_height <= 0) {
                    return(-2);
                }
                break;

            case 's':
                format.column_space = atoi(optarg);
                if (format.column_space < 0) {
                    return(-2);
                }
                break;

            case 'l':
                format.row_length = atoi(optarg);
                if (format.row_length <= 0) {
                    return(-2);
                }
                break;

            case 'v':
                printf("pagy 0.1\n");
                #ifdef MULTI_PROCESS
                printf("Multi-process version\n");
                #else
                printf("Mono-process version\n");
                #endif
                printf("Copyright © 2024 AuroraViola <https://github.com/AuroraViola>.\n");
                printf("License GPLv3+: GNU GPL version 3 or later <https://gnu.org/licenses/gpl.html>.\n");
                printf("This is free software: you are free to change and redistribute it.\n");
                printf("There is NO WARRANTY, to the extent permitted by law.\n\n");
                printf("Written by Aurora Arcidiacono\n");
                return(0);

            case 'h':
                printf("Usage: pagy [FILE] [OPTION]...\n");
                printf("  -c, --columns, n_ticks            number of columns per page\n");
                printf("  -h, --help                        display this help and exit\n");
                printf("  -l, --length, n_ticks             number of characters per row\n");
                printf("  -o, --output filename             write the output to filename instead of stdout\n");
                printf("  -r, --rows, n_ticks               number of rows per columns\n");
                printf("  -s, --spaces, n_ticks             number of spaces between columns\n");
                printf("  -v, --version                     output version information and exit\n");
                return (0);

            case ':':
            case '?':
                fprintf(stderr, "Try `%s --help' for more information.\n", argv[0]);
                return (-2);

            default:
                fprintf(stderr, "%s: invalid option -- %c\n", argv[0], arg_count);
                return (-2);
        }
    }

    if (optind < argc) {
        input_file = argv[optind];
    } else {
        input_file = NULL;
    }

#ifndef MULTI_PROCESS
    /* Generazione del file content con gestione degli errori */
    file_content content = get_file_content(input_file);
    if (content.bytes == NULL) {
        fprintf(stderr, "pagy: cannot access '%s': No such file or directory\n", input_file);
        return (-2);
    }

    /* Formattazione del contenuto del file */
    struct Page *pages = get_formatted_text(&content, &format);

    /* Scrittura nel file di destinazione con gestione degli errori */
    int ret_value = create_file(output_file, &content, pages, &format);
    if (ret_value == -2) {
        fprintf(stderr, "pagy: cannot access '%s': No such file or directory\n", output_file);
        return (-2);
    }
#else
    /* Creo la pipe per far comunicare il file_parser con il processo padre */
    int file_pipe[2];
    if (pipe(file_pipe) < 0) {
        fprintf(stderr, "pagy: pipe error\n");
        return -2;
    }

    /* Viene creato il processo "file_parser" */
    pid_t file_parser = fork();
    if (file_parser == 0) {
        close(file_pipe[0]);
        file_content content = get_file_content(input_file);
        if (content.bytes == NULL) {
            fprintf(stderr, "pagy: cannot access '%s': No such file or directory\n", input_file);
            exit(EXIT_FAILURE);
        }

        write(file_pipe[1], &content, sizeof(file_content));

        /* Scrive in blocchi da 128 byte sulla pipe essendo questa di dimensioni limitata */
        int block_size = 128;
        int blocks = content.length / 128;
        int last_block_size = content.length % 128;
        int i;
        for (i = 0; i < blocks; i++) {
            write(file_pipe[1], &content.bytes[i*block_size], block_size);
        }
        write(file_pipe[1], &content.bytes[blocks * block_size], last_block_size);

        close(file_pipe[1]);
        exit(EXIT_SUCCESS);
    }
    else if (file_parser < 0) {
        fprintf(stderr, "Error: can't fork\n");
        return -2;
    }

    close(file_pipe[1]);
    file_content content;
    read(file_pipe[0], &content, sizeof(file_content));
    content.bytes = malloc(sizeof(uint8_t) * content.length);

    /* Legge in blocchi da 128 byte sulla pipe essendo questa di dimensioni limitata */
    int block_size = 128;
    int blocks = content.length / 128;
    int last_block_size = content.length % 128;
    int i;
    for (i = 0; i < blocks; i++) {
        read(file_pipe[0], &content.bytes[i*block_size], block_size);
    }
    read(file_pipe[0], &content.bytes[blocks * block_size], last_block_size);

    close(file_pipe[0]);

    /* Vengono create le pipe per far comunicare i processi produttore e consumatore */
    int page_pipe[2];
    if (pipe(page_pipe) < 0) {
        fprintf(stderr, "pagy: pipe error\n");
        return -2;
    }

    /* Viene creato il processo "producer" che si occuperà di formattare il file */
    pid_t producer = fork();
    if (producer == 0) {
        close(page_pipe[0]);
        formatter(page_pipe[1], &content, &format);
        exit(EXIT_SUCCESS);
    }
    else if (producer < 0) {
        fprintf(stderr, "Error: can't fork\n");
        return -2;
    }

    /* Viene creato il processo "consumer" che si occuperà di stampare su file ciò che è stato prodotto dal produttore */
    pid_t consumer = fork();
    if (consumer == 0) {
        close(page_pipe[1]);
        outputter(page_pipe[0], &content, &format, output_file);
        exit(EXIT_SUCCESS);
    }
    else if (consumer < 0) {
        fprintf(stderr, "Error: can't fork\n");
        return -2;
    }

    /* Il processo padre attende il termine dei processi creati*/
    int status;
    for (i = 0; i < 3; i++) {
        wait(&status);
    }
    #endif
    return 0;
}
