#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint8_t byte[4];
    uint8_t length;
    bool is_endl;
}uni_char;

uni_char decode_utf8(FILE *file) {
    uni_char ret;
    fread(&ret.byte[0], 1, 1, file);
    ret.is_endl = false;
    switch (ret.byte[0]) {
        case 0x0d:
            fread(&ret.byte[1], 1, 1, file);
            if (ret.byte[1] == 0x0a) {
                ret.is_endl = true;
            }
            ret.length = 2;
            break;
        case 0x20 ... 0x7f:
            ret.length = 1;
            break;
        case 0xc0 ... 0xdf:
            fread(&ret.byte[1], 1, 1, file);
            ret.length = 2;
            break;
        case 0xe0 ... 0xef:
            fread(&ret.byte[1], 1, 1, file);
            fread(&ret.byte[2], 1, 1, file);
            ret.length = 3;
            break;
        case 0xf0 ... 0xf4:
            fread(&ret.byte[1], 1, 1, file);
            fread(&ret.byte[2], 1, 1, file);
            fread(&ret.byte[3], 1, 1, file);
            ret.length = 4;
            break;
        default:
            ret.length = 0;
            break;
    }
    return ret;
}

void columns(char *filename, int col_num, int row_num, int row_len, int col_space) {
    int i, j, k, l;
    uni_char text[col_num][row_num][row_len];

    FILE *file = fopen(filename, "rb");
    if (file != NULL) {
        for (i = 0; i < col_num; i++) {
            for (j = 0; j < row_num; j++) {
                for (k = 0; k < row_len; k++) {
                    text[i][j][k] = decode_utf8(file);
                    if (text[i][j][k].is_endl) {
                        for (l = k; l < row_len; l++) {
                            text[i][j][l].byte[0] = 0x20;
                            text[i][j][l].is_endl = false;
                            text[i][j][l].length = 1;
                        }
                        k = row_len;
                    }
                }
            }
        }
        fclose(file);
    }

    file = fopen("../output.txt", "wb");
    if (file != NULL) {
        for (i = 0; i < row_num; i++) {
            for (j = 0; j < col_num; j++) {
                for (k = 0; k < row_len; k++) {
                    if (!text[j][i][k].is_endl) {
                        for (l = 0; l < text[j][i][k].length; l++) {
                            fwrite(&text[j][i][k].byte[l], 1, 1, file);
                        }
                    }
                }
                for (k = 0; k < col_space; k++) {
                    fwrite(" ", 1, 1, file);
                }
            }
            fwrite("\n", 1, 1, file);
        }
        fclose(file);
    }
}

int main(void) {
    columns("../example.txt", 3, 35, 15, 8);
    return 0;
}
