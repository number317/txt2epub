#ifndef EPUB_H
#define EPUB_H

#include "config.h"

int create_epub_structure(char *book_dir);
void generate_toc_ncx(book_config *config, char *epub_dir);
void generate_opf(book_config *config);
void copy_cover(book_config *config);
void create_zip(book_config *config);

#endif
