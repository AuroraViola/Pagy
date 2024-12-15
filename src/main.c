#include <stdio.h>
#include <stdint.h>
#include "includes/file_reader.h"
#include "includes/formatting.h"
#include "includes/output.h"

int main(void) {
    char *input_file = "../example.txt";
    char *output_file = "../example-output.txt";
    struct page_format format = {3, 40, 25, 8};

    file_content content = get_file_content(input_file);
    struct Page *pages = get_formatted_text(&content, &format);
    create_file(output_file, &content, pages, &format);

    return 0;
}
