#ifndef PARAM_H
#define PARAM_H

typedef enum {
    CMD_UNKNOWN,
    CMD_INIT,
    CMD_GENERATE,
    CMD_SHOWCONF
} command_t;

typedef struct {
    command_t command;
    char *cmdline;
    char *novel_dir;
    char *txt_file;
    char *config_file;
} arguments;

int parse_arguments(int argc, char *argv[], arguments *args);
void usage();

#endif
