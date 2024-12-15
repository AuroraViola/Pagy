#ifndef SO2_FILE_READER_H
#define SO2_FILE_READER_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint8_t *bytes;
    int64_t length;
    uint64_t index;
}file_content;

file_content get_file_content(char *file_path);

#endif //SO2_FILE_READER_H
