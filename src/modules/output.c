#include "stdio.h"
#include "../includes/output.h"

void create_file(char *file_path, struct Page *pages, struct page_format *format) {
    FILE *file = fopen(file_path, "wb");
    int i, j, k, l;
    if (file != NULL) {
        while (pages != NULL) {
            for (i = 0; i < format->column_height; i++) {
                for (j = 0; j < format->columns; j++) {
                    for (k = 0; k < format->row_length; k++) {
                        for (l = 0; l < pages->page_text[j][i][k].length; l++) {
                            fwrite(&pages->page_text[j][i][k].byte[l], 1, 1, file);
                        }
                    }
                    if (j != format->columns-1)
                    for (k = 0; k < format->column_space; k++) {
                        fwrite(" ", 1, 1, file);
                    }
                }
                fwrite("\n", 1, 1, file);
            }
            pages = pages->next_page;
            if (pages != NULL) {
                fwrite("\n %%% \n\n", 8, 1, file);
            }
        }
    }
}