#include <stdio.h>
#include <stdlib.h>
#include <argp.h>
#include <string.h>
#include "param.h"

const char *argp_program_bug_address = "cheon0112358d@gmail.com";
const char *argp_program_version = "txt2epub 1.0.0";

static const char argp_doc[] = "txt2epub - A tool to generate epub from txt\n\n"
                       "Commands:\n\n"
                       "  init <novel_dir>     Initialize the novel directory\n"
                       "  generate <txt_file>  Generate epub from txt file\n"
                       "  showconf             Print current config file\n\n"
                       "Options:";

static struct argp_option options[] = {
    {"config", 'c', "config_path", 0, "Override the default config (only for generate and showconf)", 0},
    {0}
};

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
    arguments *args = state->input;
    switch (key) {
        case 'c':
            if (arg == NULL) {
                fprintf(stderr, "Error: -c requires a configuration file path.\n");
                exit(EXIT_FAILURE);
            }
            args->config_file = arg;
            break;
        case ARGP_KEY_ARG:
            args->cmdline = arg;
            if (state->arg_num == 0) {
                if (strcmp(arg, "init") == 0) {
                    args->command = CMD_INIT;
                } else if (strcmp(arg, "generate") == 0) {
                    args->command = CMD_GENERATE;
                } else if (strcmp(arg, "showconf") == 0) {
                    args->command = CMD_SHOWCONF;
                } else {
                    args->command = CMD_UNKNOWN;
                }
            } else if (state->arg_num == 1) {
                if (args->command == CMD_INIT) {
                    args->novel_dir = arg;
                } else if (args->command == CMD_GENERATE) {
                    args->txt_file = arg;
                }
            }
            break;
        case ARGP_KEY_END:
            if (args->config_file == NULL) {
                args->config_file = "./config.cfg";
            }
            if (args->cmdline == NULL) {
                args->cmdline = "";
            }
            break;
        default:
            return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

static struct argp argp = {options, parse_opt, "<command> [args...]", argp_doc, NULL, NULL, NULL};

int parse_arguments(int argc, char *argv[], arguments *args) {
    return argp_parse(&argp, argc, argv, 0, 0, args);
}
