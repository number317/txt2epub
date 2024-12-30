#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"
#include "param.h"
#include "command.h"

int main(int argc, char *argv[]) {
    arguments args = {0};

    book_config config;

    if (parse_arguments(argc, argv, &args) != 0) {
        return EXIT_FAILURE;
    }

    switch(args.command) {
        case CMD_UNKNOWN:
            fprintf(stderr, "txt2epub unknow command %s\n", args.cmdline);
            fprintf(stderr, "available commands:\n");
            fprintf(stderr, "  init <novel_dir>     Initialize the novel directory\n");
            fprintf(stderr, "  generate <txt_file>  Generate epub from txt file\n");
            fprintf(stderr, "  showconf             Print current config file\n\n");
            break;
        case CMD_INIT:
            if (args.novel_dir == NULL) {
                fprintf(stderr, "novel dir not specified\n\n");
            } else {
                init_project(args.novel_dir);
            }
            break;
        case CMD_GENERATE:
            config_load(args.config_file, &config);
            if (args.txt_file == NULL) {
                fprintf(stderr, "novel txt file not specified\n\n");
            } else {
                generate_epub(&config, args.txt_file); 
            }
            config_free(&config);
            break;
        case CMD_SHOWCONF:
            config_load(args.config_file, &config);
            config_print(&config);
            config_free(&config);
            break;
    }

    return 0;
}

