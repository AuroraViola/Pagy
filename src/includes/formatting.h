#ifndef SO2_FORMATTING_H
#define SO2_FORMATTING_H

#include "file_reader.h"

//Struttura che indica in che modo formattare la pagina:
struct page_format {
    int columns;        // Quante colonne per pagina
    int column_height;  // Quante righe per colonna
    int row_length;     // Quanto è lunga la riga di una colonna
    int column_space;   // Quanto spazio da una colonna all'altra
};

// enum che indica i tipi di formattazione possibili
typedef enum {
    NORMAL = 0,         // La riga è giustificata
    END_PARAGRAPH,      // La riga è di fine paragrafo, pertanto non deve essere giustificata
    EMPTY,              // La riga contiene solo spazi
    EMPTY_END,          // La riga non contiene nulla. Verrà assegnata alla riga alla fine del file
}ROW_TYPE;

// Struttura che indica il token delle parole in una riga
struct Word {
    uint64_t start;     // È un indice nel file dove inizia la parola
    uint64_t end;       // È un indice nel file dove finisce la parola.
    uint16_t length;    // Indica la lunghezza in caratteri unicode della parola
    struct Word *next_word;     // Prossima parola
};

// Struttura che definisce la riga
struct Row {
    ROW_TYPE type;      // tipo della riga
    struct Word *words; // parole all'interno della riga
};

// Struttura che definisce la pagina
struct Page {
    struct Row **rows;          // L'insieme delle righe
    struct Page *next_page;     // La prossima pagina
};

// Vedere formatting.c
int get_word_count(struct Row *row);
int get_row_charlength(struct Row *row);
struct Row **get_page_rows(file_content *content, struct page_format *format);
struct Page *get_formatted_text(file_content *content, struct page_format *format);

#endif //SO2_FORMATTING_H
