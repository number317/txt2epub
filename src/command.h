#ifndef COMMAND_H
#define COMMAND_H

#include "config.h"

int init_project(char *dir);
int create_config(char *dir);

int split_txt_to_md(char *txt_file, book_config *config);
int filter_check(char *str, book_config *config);
void generate_epub(book_config *config, char *txt_file);

#endif
