#include <malloc.h>
#include "../includes/formatting.h"

int get_word_count(struct Row *row) {
    int word_count = 0;
    struct Word *word = row->words;
    while (word != NULL) {
        word_count++;
        word = word->next_word;
    }
    return word_count;
}

int get_row_length(struct Row *row) {
    int length = 0;
    struct Word *word = row->words;
    while (word != NULL) {
        length += word->length;
        word = word->next_word;
    }
    return length;
}

int get_row_charlength(struct Row *row) {
    int row_char = 0;
    int words = get_word_count(row);
    if (words > 0) {
        row_char = get_row_length(row) + (words-1);
    }
    return row_char;
}

bool is_utf8_char(uint8_t byte) {
    return ((byte != 0x20) && (byte != 0xd) && (byte != 0xa));
}

uint8_t get_char_length(uint8_t byte) {
    switch(byte) {
        case 0x00 ... 0x7f:
            return 1;
        case 0xc0 ... 0xdf:
            return 2;
        case 0xe0 ... 0xef:
            return 3;
        case 0xf0 ... 0xf4:
            return 4;
        default:
            return 1;
    }
}

uint16_t get_word_length(struct Word *word, file_content *content) {
    uint16_t length = 0;
    uint64_t i = word->start;
    while (i < word->end) {
        length += get_char_length(content->bytes[i]);
        i++;
    }
    return length;
}

struct Word *get_word(file_content *content, int row_length) {
    struct Word *word = malloc(sizeof(struct Word));
    word->next_word = NULL;
    word->start = content->index;
    word->end = word->start;

    while (is_utf8_char(content->bytes[word->end])) {
        word->end++;
    }
    word->length = get_word_length(word, content);
    if (word->length > row_length) {
        word->length = row_length;
    }

    return word;
}

void add_word_to_row(struct Row *row, struct Word *word) {
    struct Word *current_word = row->words;
    if (row->words == NULL) {
        row->words = word;
    }
    else {
        while (current_word->next_word != NULL) {
            current_word = current_word->next_word;
        }
        current_word->next_word = word;
    }
}

bool go_to_next_word(file_content *content, int row_length) {
    bool is_endl = false;

    int i;
    for (i = 0; i < row_length; i++) {
        if (content->bytes[content->index] == 0x20) {
            break;
        }
        if (content->bytes[content->index] == 0xa || content->bytes[content->index] == 0xd) {
            is_endl = true;
            break;
        }
        content->index++;
    }
    while(!is_utf8_char(content->bytes[content->index])) {
        content->index++;
    }
    return is_endl;
}

struct Row **get_page_rows(file_content *content, struct page_format *format) {
    struct Row **rows = malloc(sizeof(struct Row*) * format->columns * format->column_height);
    int i;
    for (i = 0; i < (format->column_height * format->columns); i++) {
        rows[i] = malloc(sizeof(struct Row) * format->row_length);
        rows[i]->words = NULL;
        rows[i]->type = NORMAL;
        bool can_fit = true;
        do {
            struct Word *word = get_word(content, format->row_length);
            if ((get_row_charlength(rows[i]) + word->length + 1) >= format->row_length) {
                can_fit = false;
                free(word);
            }
            else {
                add_word_to_row(rows[i], word);
                if (go_to_next_word(content, format->row_length)) {
                    rows[i]->type = END_PARAGRAPH;
                }
            }
        } while (can_fit);
    }

    return rows;
}

struct Page *get_formatted_text(file_content *content, struct page_format *format) {
    struct Page *page = malloc(sizeof(struct Page));
    struct Page *current_page = page;

    content->index = 0;
    while (content->index < content->length) {
        current_page->next_page = NULL;
        current_page->rows = get_page_rows(content, format);
        if (content->index < content->length) {
            current_page->next_page = malloc(sizeof(struct Page));
            current_page = current_page->next_page;
        }
    }

    return page;
}
