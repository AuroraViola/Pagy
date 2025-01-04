#include <stdio.h>
#include <malloc.h>
#include "../includes/file_reader.h"

/*
 * Questa funzione restituisce la grandezza in byte di una variabile di tipo FILE
 * La posizione del file viene preservata
 * Se il file non è valido restituisce -1
 */
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

/*
 * Questa funzione prende come input il percorso di un file e restituisce il suo contenuto come struttura file_content
 */
file_content get_file_content(char *file_path) {
    file_content content;
    content.bytes = NULL;
    content.index = 0;

    FILE *file = fopen(file_path, "rb");
    if (file != NULL) {
        content.length = get_file_size(file);

        /* Vengono allocati in memoria i byte del file per essere salvati nella struttura del file_content */
        content.bytes = malloc(sizeof(uint8_t) * content.length);
        int i;
        for (i = 0; i < content.length; i++) {
            fread(&content.bytes[i], 1, 1, file);
        }
        fclose(file);
    }
    return content;
}