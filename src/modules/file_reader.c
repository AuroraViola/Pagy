#include <stdio.h>
#include <malloc.h>
#include "../includes/file_reader.h"

int64_t get_file_size(FILE *file) {
    if (file != NULL) {
        int64_t current_pos = ftell(file);
        fseek(file, 0, SEEK_END);
        int64_t file_size = ftell(file);
        fseek(file, current_pos, SEEK_SET);
        return file_size;
    }
    return -1;
}

file_content get_file_content(char *file_path) {
    file_content content;
    content.bytes = NULL;

    FILE *file = fopen(file_path, "rb");
    if (file != NULL) {
        content.length = get_file_size(file);
        content.bytes = malloc(sizeof(uint8_t) * content.length);
        int i;
        for (i = 0; i < content.length; i++) {
            fread(&content.bytes[i], 1, 1, file);
        }
        fclose(file);
    }
    return content;
}