#include <malloc.h>
#include "../includes/formatting.h"

bool move_pointer_to_next_word(unicode_file_content *content) {
    bool was_endl = false;

    bool found_space = false;
    while (!found_space && (!is_space(content->chars[content->index]) || !is_endl(content->chars[content->index]))) {
        if (is_space(content->chars[content->index])) {
            found_space = true;
        }
        if (is_endl(content->chars[content->index])) {
            found_space = true;
            was_endl = true;
        }
        if (content->index > content->length) {
            break;
        }
        content->index++;
    }
    return was_endl;
}

uint16_t get_word_length(unicode_file_content *content) {
    uint16_t word_length = 0;
    uint64_t prev_index = content->index;

    while ((content->index < content->length) &&
           !is_space(content->chars[content->index + word_length]) &&
           !is_endl(content->chars[content->index + word_length])) {
        word_length++;
    }
    content->index = prev_index;

    return word_length;
}

void justify(unicode_char *string, int length){
    if (is_space(string[length-1])) {

    }
}

unicode_char ***get_page_text_formatted(unicode_file_content *content, struct page_format *format) {
    int i, j, k, l;
    unicode_char ***page_text = malloc(sizeof(unicode_char**) * format->columns);
    for (i = 0; i < format->columns; i++) {
        page_text[i] = malloc(sizeof(unicode_char *) * format->column_height);
        for (j = 0; j < format->column_height; j++) {
            page_text[i][j] = malloc(sizeof(unicode_char) * format->row_length);
            for (k = 0; k < format->row_length; k++) {
                unicode_char space;
                space.length = 1;
                space.byte[0] = 0x20;
                page_text[i][j][k] = space;
            }
        }
    }

    for (i = 0; i < format->columns; i++) {
        for (j = 0; j < format->column_height; j++) {
            for (k = 0; k < format->row_length; k++) {
                int word_length = get_word_length(content);
                // La parola entra
                if (word_length <= (format->row_length - k)) {
                    for (l = 0; l < word_length; l++) {
                        page_text[i][j][k + l] = content->chars[content->index + l];
                    }
                    if (move_pointer_to_next_word(content)) {
                        k = format->row_length;
                    }
                    k += word_length;
                }
                // La parola non entra ed è all'inizio della riga
                else if (k == 0) {
                    for (l = 0; l < format->row_length; l++) {
                        page_text[i][j][k + l] = content->chars[content->index + l];
                    }
                    content->index += format->row_length;
                    k = format->row_length;
                }
                // La parola non entra in questa riga
                else {
                    k = format->row_length;
                }
            }
        }
    }

    return page_text;
}

struct Page *get_formatted_text(unicode_file_content *content, struct page_format *format) {
    struct Page *page = malloc(sizeof(struct Page));

    struct Page *current_page = page;

    content->index = 0;
    while (content->index < content->length) {
        current_page->next_page = NULL;
        current_page->page_text = get_page_text_formatted(content, format);
        if (content->index < content->length) {
            current_page->next_page = malloc(sizeof(struct Page));
            current_page = current_page->next_page;
        }
    }

    return page;
}
