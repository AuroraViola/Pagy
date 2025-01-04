#include "stdio.h"
#include "../includes/output.h"

/* Questa funzione prende in input:
 * - il file dove stampare l'output
 * - una struttura file_content che serve per avere a portata di mano il contenuto del file
 * - una struttura row che indica le singole righe della pagina
 * - una struttura page_format per indicare i parametri di come stampare la pagina
 * Lo scopo della funzione è quello di stampare una singola pagina
 */
void print_page(FILE *file, file_content *content, struct Row **rows, struct page_format *format) {
    int i, j;
    uint64_t k;
    for (i = 0; i < format->column_height; i++) {
        for (j = 0; j < format->columns; j++) {
            /* Viene selezionata la riga corrente */
            struct Row *current_row = rows[(format->column_height * j) + i];
            struct Word *current_word = current_row->words;

            /* Queste variabili serviranno quando bisognerà giustificare il testo */
            int words = get_word_count(current_row);
            int spaces[words];
            int spaces_left;
            int word_count;

            /* Stampa gli spazi di separazione delle colonne nel caso la riga selezionata non appartiene alla prima colonna */
            if (current_row->type != EMPTY_END && j != 0) {
                for (k = 0; k < format->column_space; k++) {
                    fprintf(file ," ");
                }
            }
            /* Stampa in base al tipo di riga */
            switch (current_row->type) {
                case NORMAL:
                    if (current_row->words != NULL) {
                        /* Qui vengono gestiti gli spazi tra le parole della riga per avere il testo giustificato */
                        spaces_left = format->row_length - (get_row_charlength(current_row) - words + 1);
                        if (words <= 1) {
                            spaces[0] = spaces_left;
                        } else {
                            /* l'ultima parola dovrà stampare 0 spazi */
                            spaces[words - 1] = 0;
                            /* gli spazi vengono distribuiti tra le parole */
                            for (k = 0; k < words - 1; k++) {
                                spaces[k] = spaces_left / (words - 1);
                            }
                            /* la prima parola avrà in più aggiunto il resto della divisione tra gli spazi */
                            spaces[0] += spaces_left % (words - 1);
                        }

                        /* word_count serve per indicare l'indice della parola stampata */
                        word_count = 0;
                        /* Viene stampata ogni parola nella riga */
                        while (current_word != NULL) {
                            for (k = current_word->start; k < current_word->end; k++) {
                                fprintf(file, "%c", content->bytes[k]);
                            }
                            /* Vengono stampati N spazi tra una parola e l'altra per avere una riga giustificata */
                            for (k = 0; k < spaces[word_count]; k++) {
                                fprintf(file, " ");
                            }
                            current_word = current_word->next_word;
                            word_count++;
                        }
                    }
                    break;
                case END_PARAGRAPH:
                    /* stampa le parole nella riga. Le parole sono separate da spazi */
                    while (current_word != NULL) {
                        /* Stampa della parola */
                        for (k = current_word->start; k < current_word->end; k++) {
                            fprintf(file, "%c", content->bytes[k]);
                        }
                        /* Stampa lo spazio solo se ci sta una prossima parola */
                        if (current_word->next_word != NULL) {
                            fprintf(file, " ");
                        }
                        current_word = current_word->next_word;
                    }
                    /* Vengono inseriti gli spazi alla fine della riga in modo da rendere la formattazione più semplice */
                    for (k = 0; k < (format->row_length - get_row_charlength(current_row)); k++) {
                        fprintf(file, " ");
                    }
                    break;
                case EMPTY:
                    /* Stampa una riga di soli spazi */
                    for (k = 0; k < (format->row_length); k++) {
                        fprintf(file, " ");
                    }
                    break;
                case EMPTY_END:
                    /* Stampa il carattere "a capo" solo se si sta nella prima colonna in modo da rendere le pagine sempre di altezza uguale */
                    if (j == 0) {
                        fprintf(file, "\n");
                    }
                    break;
            }
        }
        fprintf(file, "\n");
    }
}

/*
 * Questa funzione prende in input:
 * - la path del file dove salvare il testo finale
 * - una struttura file_content che serve per avere a portata di mano il contenuto del file
 * - una struttura Page che indica come stampare i byte del file_content
 * - una struttura page_format per indicare i parametri di come stampare la pagina
 * Restituisce -2 nel caso non sia possibile creare o accedere al file di output altrimenti restituisce 0
 * Lo scopo della funzione è quello di stampare ogni pagina
 */
int create_file(char *file_path, file_content *content, struct Page *pages, struct page_format *format) {
    /* Viene scelto dove salvare l'output, nel caso il file_path è NULL (quindi non è stato specificato) verrà stampato su stdout */
    FILE *file;
    if (file_path != NULL) {
        file = fopen(file_path, "w");
        if (file == NULL) {
            return(-2);
        }
    }
    else {
        file = stdout;
    }

    /* Viene stampata pagina per pagina */
    struct Page *current_page = pages;
    while (current_page != NULL) {
        print_page(file, content, current_page->rows, format);
        current_page = current_page->next_page;
        /* Qui viene stampato il separatore */
        if (current_page != NULL) {
            fprintf(file, "\n %%%\n");
        }
    }
    return 0;
}