#include "stdio.h"
#include "../includes/output.h"

int create_file(char *file_path, file_content *content, struct Page *pages, struct page_format *format) {
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
    struct Page *current_page = pages;
    uint64_t k;
    while (current_page != NULL) {
        int i, j;
        for (i = 0; i < format->column_height; i++) {
            for (j = 0; j < format->columns; j++) {
                struct Row *current_row = current_page->rows[(format->column_height * j) + i];
                struct Word *current_word = current_row->words;

                int words = get_word_count(current_row);
                int spaces[words];
                int spaces_left;
                int word_count;
                switch (current_row->type) {
                    case NORMAL:
                        if (current_row->words != NULL) {
                            if (j != 0) {
                                for (k = 0; k < format->column_space; k++) {
                                    fprintf(file ," ");
                                }
                            }
                            spaces_left = format->row_length - (get_row_charlength(current_row) - words + 1);
                            if (words <= 1) {
                                spaces[0] = spaces_left;
                            } else {
                                spaces[words - 1] = 0;
                                for (k = 0; k < words - 1; k++) {
                                    spaces[k] = spaces_left / (words - 1);
                                }

                                spaces[0] += spaces_left % (words - 1);
                            }

                            word_count = 0;
                            while (current_word != NULL) {
                                for (k = current_word->start; k < current_word->end; k++) {
                                    fprintf(file, "%c", content->bytes[k]);
                                }
                                for (k = 0; k < spaces[word_count]; k++) {
                                    fprintf(file, " ");
                                }
                                current_word = current_word->next_word;
                                word_count++;
                            }
                        }
                        break;
                    case END_PARAGRAPH:
                        if (j != 0) {
                            for (k = 0; k < format->column_space; k++) {
                                fprintf(file, " ");
                            }
                        }
                        while (current_word != NULL) {
                            for (k = current_word->start; k < current_word->end; k++) {
                                fprintf(file, "%c", content->bytes[k]);
                            }
                            if (current_word->next_word != NULL) {
                                fprintf(file, " ");
                            }
                            current_word = current_word->next_word;
                        }
                        for (k = 0; k < (format->row_length - get_row_charlength(current_row)); k++) {
                            fprintf(file, " ");
                        }
                        break;
                    case EMPTY:
                        if (j != 0) {
                            for (k = 0; k < (format->column_space); k++) {
                                fprintf(file, " ");
                            }
                        }
                        for (k = 0; k < (format->row_length); k++) {
                            fprintf(file, " ");
                        }
                        break;
                    case EMPTY_END:
                        break;
                }
            }
            fprintf(file, "\n");
        }
        current_page = current_page->next_page;
        if (current_page != NULL) {
            fprintf(file, "\n");
            for (i = 0; i < ((format->row_length * format->columns) + (format->column_space * (format->columns-1))); i++)
                fprintf(file, "-");
            fprintf(file, "\n\n");
        }
    }
    return 0;
}