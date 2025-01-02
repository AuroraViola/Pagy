#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#include "includes/file_reader.h"
#include "includes/formatting.h"
#include "includes/output.h"

int main(int argc, char **argv) {
    // Inizializzazione dei valori di default
    char *input_file = NULL;
    char *output_file = NULL;
    struct page_format format = {3, 30, 25, 8};
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
        int p[2];
        pipe(p);

        int bytes_pipe[2];

        int pid = fork();
        if (pid == 0) {
            close(p[0]);
            file_content content = get_file_content(input_file);
            if (content.bytes == NULL) {
                fprintf(stderr, "pagy: cannot access '%s': No such file or directory\n", input_file);
                return (-2);
            }

            write(p[1], &content, sizeof(file_content));
            write(p[1], content.bytes, content.length);
            close(p[1]);
            exit(EXIT_SUCCESS);
        }
        else if (pid < 0) {
            fprintf(stderr, "Error during fork()\n");
        }
        close(p[1]);

        file_content content;
        read(p[0], &content, sizeof(file_content));
        content.bytes = malloc(sizeof(uint8_t) * content.length);
        read(p[0], content.bytes, content.length);
        close(p[0]);

        struct Page *pages = get_formatted_text(&content, &format);

        int ret_value = create_file(output_file, &content, pages, &format);
        if (ret_value == -2) {
            fprintf(stderr, "pagy: cannot access '%s': No such file or directory\n", output_file);
            return (-2);
        }
    }
    return 0;
}
