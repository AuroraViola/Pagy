#include <malloc.h>
#include "../includes/formatting.h"

// Prende in input un puntatore ad una struttura Row e ritorna il numero di parole che contiene
int get_word_count(struct Row *row) {
    int word_count = 0;
    struct Word *word = row->words;
    while (word != NULL) {
        word_count++;
        word = word->next_word;
    }
    return word_count;
}

// Prende in input un puntatore ad una struttura Row e ritorna la somma della lunghezza di ogni parola nella riga
// Viene usata da get_row_charlength
int get_row_length(struct Row *row) {
    int length = 0;
    struct Word *word = row->words;
    while (word != NULL) {
        length += word->length;
        word = word->next_word;
    }
    return length;
}

// Prende in input un puntatore ad una struttura Row e ritorna la somma della lunghezza di ogni parola nella riga + il
// numero di spazi, come se dovesse ritornare la lunghezza di una stringa composta dalle parole delle riga separate da uno spazio
int get_row_charlength(struct Row *row) {
    int row_char = 0;
    int words = get_word_count(row);
    if (words > 0) {
        row_char = get_row_length(row) + (words-1);
    }
    return row_char;
}

/* Questa funziona restituisce false se il primo byte UTF-8 è uno dei seguenti:
 * " "
 * "\r"
 * "\n"
 * "\t"
 * Altrimenti restituisce true
 * Viene usata principalemente nella funzione go_to_next_word per muovere l'index del file content alla prossima parola
 */
bool is_utf8_char(uint8_t byte) {
    return ((byte != 0x20) && (byte != 0xd) && (byte != 0xa) && (byte != 0x9));
}

// Prende in input il primo byte di un carattere UTF-8 e ritorna la sua lunghezza in byte
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

// Questa funziona prende in input una struttura file_content e la lunghezza della riga
// La funzione restituirà un puntatore ad una struttura di tipo Word
struct Word *get_word(file_content *content, int row_length) {
    // Viene allocata la parola
    struct Word *word = malloc(sizeof(struct Word));

    if (content->index < content->length) {
        // Inizializza la parola
        word->next_word = NULL;
        word->start = content->index;
        word->end = word->start;
        word->length = 0;

        // Questo ciclo serve per trovare dove finisce la parola nel file e avere la sua lunghezza in caratteri.
        // Viene anche controllato che la parola non sia più grande della lunghezza della riga in modo da dividere le parole
        // troppo lunghe su più righe
        while (is_utf8_char(content->bytes[word->end]) && word->length < row_length) {
            word->end += get_char_length(content->bytes[word->end]);
            word->length++;
        }
        return word;
    }
    else {
        // Viene deallocata la parola se il file è finito e ritorna un puntatore NULL
        free(word);
        return NULL;
    }
}

// Questa funziona prende in input una struttura di tipo Row e una di tipo Word
// Il suo scopo è di aggiungere la parola alla fine della row
void add_word_to_row(struct Row *row, struct Word *word) {
    struct Word *current_word = row->words;
    // Se la riga è vuota la parola viene inserita come primo elemento
    if (row->words == NULL) {
        row->words = word;
    }
    // Altrimenti cicla per tutte le parole all'interno della riga fino ad arrivare all'ultima
    else {
        while (current_word->next_word != NULL) {
            current_word = current_word->next_word;
        }
        // Aggiunge la parola alla riga
        current_word->next_word = word;
    }
}

// Questa funzione prende in input una struttura di tipo file_content e la lunghezza della riga
// Il suo compito è quello di spostare l'indice del file_content all'inizio della prossima parola
// Restituisce true se nell'andare alla prossima parola trova un carattere "\n" o "\r" altrimenti restituisce false
bool go_to_next_word(file_content *content, int row_length) {
    bool is_endl = false;
    int i;
    // Finché non finisce la lunghezza della riga muove l'indice del file avanti
    // Nel caso in cui il carattere sia uno spazio, una tabulazione o un "a capo", il ciclo viene interrotto
    for (i = 0; i < row_length; i++) {
        if (content->bytes[content->index] == 0x20 || content->bytes[content->index] == 0x9) {
            break;
        }
        if (content->bytes[content->index] == 0xa || content->bytes[content->index] == 0xd) {
            is_endl = true;
            break;
        }
        content->index++;
    }

    // Muove l'index del file_content all'inizio della prossima parola
    // NB: Nel caso il ciclo precedente non sia stato interrotto da un break questo ciclo potrebbe non essere eseguito.
    // Ciò è fatto apposta per poter dividere al meglio la parola su più linee
    while(!is_utf8_char(content->bytes[content->index])) {
        if (content->bytes[content->index] == 0xa || content->bytes[content->index] == 0xd) {
            is_endl = true;
        }
        content->index++;
    }

    return is_endl;
}


// Questa funzione prende in input una struttura di tipo file_content e una struttura di tipo page_format
// La funzione restituirà un puntatore che punta alle righe della pagina
// In questa funzione vengono create le righe che poi comporranno una singola pagina
struct Row **get_page_rows(file_content *content, struct page_format *format) {
    int rows_num = format->columns * format->column_height;
    // Alloca i puntatori alle righe
    struct Row **rows = malloc(sizeof(struct Row*) * rows_num);
    int i;
    bool ended_paragraph = false;

    for (i = 0; i < rows_num; i++) {
        // Alloca la riga
        rows[i] = malloc(sizeof(struct Row) * format->row_length);
        rows[i]->words = NULL;
        // Se la riga precedente era una riga di fine paragrafo, allora questa riga sarà una riga vuota
        if (ended_paragraph) {
            rows[i]->type = EMPTY;
            ended_paragraph = false;
        }
        // Altrimenti procedi normalmente
        else {
            rows[i]->type = NORMAL;
            // La variabile can_fit verrà settata a false nei seguenti casi:
            // - Non c'è più spazio nella riga
            // - Si è arrivati alla fine del paragrafo
            // - Non ci sono più parole da aggiungere perché il file è finito
            bool can_fit = true;
            // Questo ciclo verrà eseguito finché non c'è più spazio per inserire parole nella riga
            do {
                // Viene creata e alloca la parola
                struct Word *word = get_word(content, format->row_length);
                if (word != NULL) {
                    // la parola verrà deallocata e can_fit settato a false nel caso non riesce a entrare all'interno della riga
                    if (((get_row_charlength(rows[i]) + word->length + 1) >= format->row_length) && rows[i]->words != NULL) {
                        can_fit = false;
                        free(word);
                    }
                    // Se invece riesce ad entrarci, oppure la riga non ha ancora nessuna parola, la parola verrà aggiunta
                    else {
                        // Aggiunge la parola alla riga
                        add_word_to_row(rows[i], word);
                        // Va alla prossima parola e se nell'andarci va "a capo" segna la riga come "di fine paragrafo"
                        if (go_to_next_word(content, format->row_length)) {
                            rows[i]->type = END_PARAGRAPH;
                            ended_paragraph = true;
                            can_fit = false;
                        }
                    }
                }
                // Se la creazione della parola fallisce significa che il contenuto del file è finito. Quindi le
                // righe finali saranno di tipo EMPTY_END
                else {
                    rows[i]->type = EMPTY_END;
                    can_fit = false;
                }
            } while (can_fit);
        }
    }

    return rows;
}

// Questa funzione prende in input una struttura di tipo file_content e una struttura di tipo page_format
// La funzione restituisce un puntatore ad una struttura di tipo Page
// Il compito di questa funzione è quello di avere una struttura Page che sarà più comoda per gestire l'output successivamente
struct Page *get_formatted_text(file_content *content, struct page_format *format) {
    // Alloca la prima pagina
    struct Page *page = malloc(sizeof(struct Page));
    struct Page *current_page = page;

    content->index = 0;
    // Formatta le righe della pagina corrente finché il file non finisce
    while (content->index < content->length) {
        current_page->next_page = NULL;
        current_page->rows = get_page_rows(content, format);
        // crea una pagina successiva nel caso non abbia ancora terminato il suo lavoro
        if (content->index < content->length) {
            current_page->next_page = malloc(sizeof(struct Page));
            current_page = current_page->next_page;
        }
    }

    return page;
}
