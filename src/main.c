#include <stdio.h>
#include <stdint.h>
#include "includes/unicode8.h"
#include "includes/formatting.h"
#include "includes/output.h"

int main(void) {
    char *input_file = "../example.txt";
    char *output_file = "../example-output.txt";
    struct page_format format = {3, 40, 20, 8};

    unicode_file_content content = get_file_content(input_file);
    struct Page *pages = get_formatted_text(&content, &format);
    create_file(output_file, pages, &format);

    return 0;
}
