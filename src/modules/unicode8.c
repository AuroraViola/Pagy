#include <stdio.h>
#include <malloc.h>
#include "../includes/unicode8.h"

unicode_char decode_utf8(const uint8_t *bytes) {
    unicode_char ret;
    int i;
    for (i = 0; i < 4; i++) {
        ret.byte[i] = bytes[i];
    }
    ret.length = 1;
    switch (bytes[0]) {
        case 0x0d:
            if (bytes[1] == 0x0a) {
                ret.length = 2;
            }
            break;
        case 0xc2 ... 0xdf:
            ret.length = 2;
            break;
        case 0xe0 ... 0xef:
            ret.length = 3;
            break;
        case 0xf0 ... 0xf4:
            ret.length = 4;
            break;
        default:
            break;
    }
    return ret;
}

bool is_endl(unicode_char unicode_char) {
    if (unicode_char.byte[0] == 0xd) {
        if ((unicode_char.length == 1) || (unicode_char.length == 2 && unicode_char.byte[1] == 0xa)) {
            return true;
        }
    }
    return false;
}

bool is_space(unicode_char unicode_char) {
    return (unicode_char.byte[0] == 0x20 && unicode_char.length == 1);
}

bool is_valid(unicode_char unicode_char) {
    switch (unicode_char.byte[0]) {
        case 0x80 ... 0xc1:
        case 0xf5 ... 0xff:
            return false;
        case 0xc2 ... 0xdf:
            if (unicode_char.length != 2) {
                return false;
            }
            break;
        case 0xe0 ... 0xef:
            if (unicode_char.length != 3) {
                return false;
            }
            break;
        case 0xf0 ... 0xf4:
            if (unicode_char.length != 4) {
                return false;
            }
            break;
        default:
            break;
    }
    return true;
}

int64_t get_file_size(char *file_path) {
    FILE *file = fopen(file_path, "rb");
    if (file != NULL) {
        fseek(file, 0, SEEK_END);

        int64_t file_size = ftell(file);
        fclose(file);
        return file_size;
    }
    return -1;
}

uint8_t *get_file_bytes(char *file_path, int64_t file_size) {
    FILE *file = fopen(file_path, "rb");
    uint8_t *file_content = NULL;
    if (file != NULL) {
        file_content = malloc(sizeof(uint8_t) * file_size);
        int i;
        for (i = 0; i < file_size; i++) {
            fread(&file_content[i], 1, 1, file);
        }
        fclose(file);
    }
    return file_content;
}

int64_t get_unicode_length(uint8_t *bytes, int64_t length) {
    int64_t unicode_length = 0;
    int i = 0;
    while (i < length) {
        unicode_char current_char = decode_utf8(&bytes[i]);
        i += current_char.length;
        unicode_length++;
    }
    return unicode_length;
}

unicode_file_content get_file_content(char *file_path) {
    unicode_file_content file_content;
    file_content.length = -1;
    file_content.index = 0;

    int64_t file_size = get_file_size(file_path);
    uint8_t *file_bytes = get_file_bytes(file_path, file_size);
    if (file_size != -1 && file_bytes != NULL) {
        file_content.length = get_unicode_length(file_bytes, file_size);
        file_content.chars = malloc(sizeof(unicode_char)*file_content.length);
        int i = 0;
        int j = 0;
        while (i < file_size) {
            file_content.chars[j] = decode_utf8(&file_bytes[i]);
            i += file_content.chars[j].length;
            j++;
        }
    }
    free(file_bytes);
    return file_content;
}