#include "stdio.h"
#include "../includes/output.h"

void create_file(char *file_path, file_content *content, struct Page *pages, struct page_format *format) {
    struct Page *current_page = pages;
    uint64_t j;
    while (current_page != NULL) {
        int i;
        for (i = 0; i < format->column_height * format->columns; i++) {
            struct Row *current_row = current_page->rows[i];
            struct Word *current_word = current_row->words;

            while (current_word != NULL) {
                for (j = current_word->start; j < current_word->end; j++) {
                    printf("%c", content->bytes[j]);
                }
                printf(" ");
                current_word = current_word->next_word;
            }
            printf("\n");
        }
        current_page = current_page->next_page;
    }
}