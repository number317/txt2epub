#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <sys/stat.h>

int create_dir(char *dir) {
    if (mkdir(dir, 0755) == -1) {
        if (errno != EEXIST) {
            perror("Error creating directory");
            return 1;
        }
    }
    return 0;
}

int write_file_once(char *filename, char *buf) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        perror("Error opening file for writing");
        return 1;
    }
    fprintf(file, "%s", buf);
    fclose(file);
    return 0;
}

char *get_current_time() {
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    char *time_str = malloc(sizeof(char) * 21);
    strftime(time_str, 21, "%Y-%m-%dT%H:%M:%SZ", tm);
    return time_str;
}
