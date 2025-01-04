#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#include "includes/file_reader.h"
#include "includes/formatting.h"
#include "includes/output.h"

void formatter(int write_fd, file_content *content, struct page_format *format) {
    uint8_t print_next_page = 1;
    uint8_t stop = 0;
    while (content->index < content->length) {
        write(write_fd, &print_next_page, sizeof(uint8_t));
        struct Row **rows = get_page_rows(content, format);
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

void outputter(int read_fd, file_content *content, struct page_format *format, char *file_path) {
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

    uint8_t next_page;

    read(read_fd, &next_page, sizeof(uint8_t));
    while (next_page != 0) {
        struct Row **rows = malloc(sizeof(struct Row *) * format->columns * format->column_height);
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
        print_page(file, content, rows, format);
        free(rows);

        read(read_fd, &next_page, sizeof(uint8_t));
        if (next_page != 0)
            fprintf (file, "\n %%%\n");
    }

    close(read_fd);
}

int main(int argc, char **argv) {
    // Inizializzazione dei valori di default
    char *input_file = NULL;
    char *output_file = NULL;
    struct page_format format = {5, 400, 50, 8};
    bool multi_process = false;

    // Parsing delle opzioni
    const char *short_opt = "o:c:r:s:l:mhv";
    struct option long_opt[] = {
            {"output", required_argument, NULL, 'o'},
            {"columns", required_argument, NULL, 'c'},
            {"rows", required_argument, NULL, 'r'},
            {"spaces", required_argument, NULL, 's'},
            {"length", required_argument, NULL, 'l'},
            {"multi", no_argument, NULL, 'm'},
            {"help", no_argument, NULL, 'h'},
            {"version", no_argument, NULL, 'v'},
            {NULL, 0, NULL, 0  }
    };

    int arg_count;
    while((arg_count = getopt_long(argc, argv, short_opt, long_opt, NULL)) != -1) {
        switch(arg_count) {
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

            case 'm':
                multi_process = true;
                break;

            case 'v':
                printf("pagy 0.1\n");
                printf("Copyright © 2024 AuroraViola <https://github.com/AuroraViola>.\n");
                printf("License GPLv3+: GNU GPL version 3 or later <https://gnu.org/licenses/gpl.html>.\n");
                printf("This is free software: you are free to change and redistribute it.\n");
                printf("There is NO WARRANTY, to the extent permitted by law.\n\n");
                printf("Written by Aurora Arcidiacono\n");
                return(0);

            case 'h':
                printf("Usage: pagy {FILE} [OPTION]...\n");
                printf("  -c, --columns, n_ticks            number of columns per page\n");
                printf("  -h, --help                        display this help and exit\n");
                printf("  -l, --length, n_ticks             number of characters per row\n");
                printf("  -m, --multi                       use the multiprocess approach\n");
                printf("  -o, --output filename             write the output to filename instead of stdout\n");
                printf("  -r, --rows, n_ticks               number of rows per columns\n");
                printf("  -s, --spaces, n_ticks             number of spaces between columns\n");
                printf("  -v, --version                     output version information and exit\n");
                return(0);

            case ':':
            case '?':
                fprintf(stderr, "Try `%s --help' for more information.\n", argv[0]);
                return(-2);

            default:
                fprintf(stderr, "%s: invalid option -- %c\n", argv[0], arg_count);
                return(-2);
        }
    }

    if (optind < argc) {
        input_file = argv[optind];
    }
    else {
        fprintf(stderr, "Usage: pagy {FILE} [OPTION]...\n");
        fprintf(stderr, "Try `%s --help' for more information.\n", argv[0]);
        return -2;
    }

    // Versione mono-processo del programma
    if (!multi_process) {
        // Generazione del file content con gestione degli errori
        file_content content = get_file_content(input_file);
        if (content.bytes == NULL) {
            fprintf(stderr, "pagy: cannot access '%s': No such file or directory\n", input_file);
            return (-2);
        }

        // Formattazione del contenuto del file
        struct Page *pages = get_formatted_text(&content, &format);

        // Scrittura nel file di destinazione con gestione degli errori
        int ret_value = create_file(output_file, &content, pages, &format);
        if (ret_value == -2) {
            fprintf(stderr, "pagy: cannot access '%s': No such file or directory\n", output_file);
            return (-2);
        }
    }
    // Versione multi-processo del programma
    else {
        int file_pipe[2];
        pipe(file_pipe);

        pid_t file_parser = fork();
        if (file_parser == 0) {
            close(file_pipe[0]);
            file_content content = get_file_content(input_file);
            if (content.bytes == NULL) {
                fprintf(stderr, "pagy: cannot access '%s': No such file or directory\n", input_file);
                return (-2);
            }

            write(file_pipe[1], &content, sizeof(file_content));
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

        close(file_pipe[1]);
        file_content content;
        read(file_pipe[0], &content, sizeof(file_content));
        content.bytes = malloc(sizeof(uint8_t) * content.length);

        int block_size = 128;
        int blocks = content.length / 128;
        int last_block_size = content.length % 128;

        int i;
        for (i = 0; i < blocks; i++) {
            read(file_pipe[0], &content.bytes[i*block_size], block_size);
        }
        read(file_pipe[0], &content.bytes[blocks * block_size], last_block_size);

        close(file_pipe[0]);

        int page_pipe[2];
        pipe(page_pipe);

        pid_t producer = fork();
        if (producer == 0) {
            close(page_pipe[0]);
            formatter(page_pipe[1], &content, &format);
            exit(EXIT_SUCCESS);
        }
        pid_t consumer = fork();
        if (consumer == 0) {
            close(page_pipe[1]);
            outputter(page_pipe[0], &content, &format, output_file);
            exit(EXIT_SUCCESS);
        }

        int status;
        for (i = 0; i < 3; i++) {
            wait(&status);
            if (status != EXIT_SUCCESS) {
                printf("Process %i crashed\n", i+1);
                kill(file_parser, SIGKILL);
                kill(producer, SIGKILL);
                kill(consumer, SIGKILL);
            }
        }
    }
    return 0;
}
