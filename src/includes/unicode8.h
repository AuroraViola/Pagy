#ifndef SO2_UNICODE8_H
#define SO2_UNICODE8_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint8_t byte[4];
    uint8_t length;
}unicode_char;

typedef struct {
    unicode_char *chars;
    int64_t length;
    uint64_t index;
}unicode_file_content;

unicode_file_content get_file_content(char *file_path);
bool is_endl(unicode_char unicode_char);
bool is_space(unicode_char unicode_char);

#endif //SO2_UNICODE8_H
