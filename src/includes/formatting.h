#include "file_reader.h"

#ifndef SO2_FORMATTING_H
#define SO2_FORMATTING_H

struct page_format {
    int columns;
    int column_height;
    int row_length;
    int column_space;
};

typedef enum {
    NORMAL = 0,
    END_PARAGRAPH,
    EMPTY,
    EMPTY_END,
}ROW_TYPE;

struct Word {
    uint64_t start;
    uint64_t end;
    uint16_t length;
    struct Word *next_word;
};

struct Row {
    ROW_TYPE type;
    struct Word *words;
};

struct Page {
    struct Row **rows;
    struct Page *next_page;
};

int get_word_count(struct Row *row);
int get_row_length(struct Row *row);
int get_row_charlength(struct Row *row);

struct Page *get_formatted_text(file_content *content, struct page_format *format);

#endif //SO2_FORMATTING_H
