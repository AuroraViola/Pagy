#include "formatting.h"
#include "file_reader.h"
#ifndef SO2_OUTPUT_H
#define SO2_OUTPUT_H

int create_file(char *file_path, file_content *content, struct Page *pages, struct page_format *format);

#endif //SO2_OUTPUT_H
