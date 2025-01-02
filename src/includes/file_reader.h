#ifndef SO2_FILE_READER_H
#define SO2_FILE_READER_H

#include <stdint.h>
#include <stdbool.h>

// Struttura usata per gestire il contenuto di un file letto
typedef struct {
    uint8_t *bytes; // Il contenuto del file in byte
    int64_t length; // Dimensione del file
    uint64_t index; // Indice corrente del file_content
}file_content;

// Vedere file_reader.c
file_content get_file_content(char *file_path);

#endif //SO2_FILE_READER_H
