#include "unicode8.h"

#ifndef SO2_FORMATTING_H
#define SO2_FORMATTING_H

struct page_format {
    int columns;
    int column_height;
    int row_length;
    int column_space;
};

struct Page {
    unicode_char ***page_text;
    struct Page *next_page;
};

struct Page *get_formatted_text(unicode_file_content *content, struct page_format *format);

#endif //SO2_FORMATTING_H
