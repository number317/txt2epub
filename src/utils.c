#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

char *read_file_alloc(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    if (size <= 0) {
        fclose(f);
        return NULL;
    }

    rewind(f);
    char *buf = malloc((size_t)size + 1);
    if (!buf) {
        fclose(f);
        return NULL;
    }

    size_t n = fread(buf, 1, (size_t)size, f);
    fclose(f);
    buf[n] = '\0';
    return buf;
}

char *get_current_time() {
    time_t t = time(NULL);
    struct tm *tm = gmtime(&t);
    if (!tm) {
        fprintf(stderr, "Failed to get current time\n");
        exit(EXIT_FAILURE);
    }
    char *time_str = malloc(sizeof(char) * 21);
    if (!time_str) {
        fprintf(stderr, "Memory allocation failed for time string\n");
        exit(EXIT_FAILURE);
    }
    strftime(time_str, 21, "%Y-%m-%dT%H:%M:%SZ", tm);
    return time_str;
}
