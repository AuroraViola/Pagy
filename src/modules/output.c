#include "stdio.h"
#include "../includes/output.h"

void create_file(char *file_path, file_content *content, struct Page *pages, struct page_format *format) {
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
                                    printf("%c", content->bytes[k]);
                                }
                                for (k = 0; k < spaces[word_count]; k++) {
                                    printf(" ");
                                }
                                current_word = current_word->next_word;
                                word_count++;
                            }
                            for (k = 0; k < format->column_space; k++) {
                                printf(" ");
                            }
                        }
                        break;
                    case END_PARAGRAPH:
                        while (current_word != NULL) {
                            for (k = current_word->start; k < current_word->end; k++) {
                                printf("%c", content->bytes[k]);
                            }
                            if (current_word->next_word != NULL) {
                                printf(" ");
                            }
                            current_word = current_word->next_word;
                        }
                        for (k = 0; k < (format->column_space + (format->row_length - get_row_charlength(current_row))); k++) {
                            printf(" ");
                        }
                        break;
                    case EMPTY:
                        for (k = 0; k < (format->row_length + format->column_space); k++) {
                            printf(" ");
                        }
                        break;
                    case EMPTY_END:
                        break;
                }
            }
            printf("\n");
        }
        current_page = current_page->next_page;
        if (current_page != NULL) {
            printf("\n");
            for (i = 0; i < ((format->row_length * format->columns) + (format->column_space * (format->columns-1))); i++)
                printf("-");
            printf("\n\n");
        }
    }
}