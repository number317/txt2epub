#include "config.h"
#include <libconfig.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

char *load_string_setting(config_t *cfg, const char *name) {
    const char *temp;
    if (config_lookup_string(cfg, name, &temp)) {
        char *result = malloc(strlen(temp) + 1);
        if (result) strcpy(result, temp);
        return result;
    }
    return NULL;
}

regex *load_regex_setting(config_t *cfg, const char *name) {
    regex *regex = malloc(sizeof(struct regex));
    if (regex == NULL) {
        fprintf(stderr, "Memory allocation failed for regex object\n");
        exit(EXIT_FAILURE);
    }

    regex->reg = NULL;
    regex->reg_pattern = NULL;

    regex->reg = malloc(sizeof(regex_t));
    if (regex->reg == NULL) {
        fprintf(stderr, "Memory allocation failed for regex_t object\n");
        exit(EXIT_FAILURE);
    }

    regex->reg_pattern = load_string_setting(cfg, name);
    if (regex->reg_pattern) {
        int ret = regcomp(regex->reg, regex->reg_pattern, REG_EXTENDED);
        if (ret) {
            fprintf(stderr, "Could not compile regex\n");
            exit(EXIT_FAILURE);
        }
    }

    return regex;
}

bool load_bool_setting(config_t *cfg, const char *name) {
    int temp;
    if (config_lookup_bool(cfg, name, &temp)) {
        return temp ? true : false;
    }
    return false;
}

int load_int_setting(config_t *cfg, const char *name) {
    int temp;
    if (config_lookup_int(cfg, name, &temp)) {
        return temp;
    }
    return 0;
}

regex **load_regex_array(config_t *cfg, const char *name, int *count) {
    regex **result = NULL;
    config_setting_t *array = config_lookup(cfg, name);
    if (!array) {
        *count = 0;
        return NULL;
    }

    int length = config_setting_length(array);
    *count = length;

    result = malloc(sizeof(regex *) * length);
    if (result == NULL) {
        fprintf(stderr, "Memory allocation failed for regex array\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < length; i++) {
        result[i] = malloc(sizeof(regex));
        if (result[i] == NULL) {
            fprintf(stderr, "Memory allocation failed for regex object at index %d\n", i);
            exit(EXIT_FAILURE);
        }

        result[i]->reg = NULL;
        result[i]->reg_pattern = NULL;

        result[i]->reg = malloc(sizeof(regex_t));
        if (result[i]->reg == NULL) {
            fprintf(stderr, "Memory allocation failed for regex_t object at index %d\n", i);
            exit(EXIT_FAILURE);
        }

        const char *elem = config_setting_get_string_elem(array, i);
        if (elem) {
            result[i]->reg_pattern = malloc(sizeof(char) * (strlen(elem) + 1));
            if (!result[i]->reg_pattern) {
                fprintf(stderr, "Memory allocation failed for element %d of array: %s\n", i, name);
                exit(EXIT_FAILURE);
            }
            strcpy(result[i]->reg_pattern, elem);

            int ret = regcomp(result[i]->reg, result[i]->reg_pattern, REG_EXTENDED);
            if (ret) {
                fprintf(stderr, "Could not compile regex %s\n", result[i]->reg_pattern);
                exit(EXIT_FAILURE);
            }
        } else {
            result[i] = NULL;
        }
    }

    return result;
}

void config_load(const char *filepath, book_config *config) {
    if (access(filepath, F_OK) == -1) {
        fprintf(stderr, "Config file %s does not exist.\n", filepath);
        exit(EXIT_FAILURE);
    }

    config_t cfg;
    config_init(&cfg);

    if (!config_read_file(&cfg, filepath)) {
        fprintf(stderr, "Error reading config file %s: %s (line %d)\n",
                filepath, config_error_text(&cfg), config_error_line(&cfg));
        config_destroy(&cfg);
        exit(EXIT_FAILURE);
    }

    config->title = load_string_setting(&cfg, "title");
    config->author = load_string_setting(&cfg, "author");
    config->cover = load_string_setting(&cfg, "cover");
    config->preface_reg = load_regex_setting(&cfg, "prefaceReg");
    config->level1_reg = load_regex_setting(&cfg, "level1Reg");
    config->level2_reg = load_regex_setting(&cfg, "level2Reg");
    config->epilogue_reg = load_regex_setting(&cfg, "epilogueReg");

    config->filter_list = load_regex_array(&cfg, "filterList", &config->filter_count);
    config->chapter_name_length_limit = load_int_setting(&cfg, "chapterNameLengthLimit");
    config->compress = load_string_setting(&cfg, "compress");
    config->first_chapter = NULL;

    config_destroy(&cfg);
}

void free_regex(regex *regex) {
    if (!regex) return;
    if (regex->reg) {
        regfree(regex->reg);
        free(regex->reg);
    }
    if (regex->reg_pattern) {
        free(regex->reg_pattern);
    }
    free(regex);
}

void config_free(book_config *config) {
    free(config->title);
    free(config->author);
    free(config->cover);
    free(config->compress);

    free_regex(config->preface_reg);
    free_regex(config->level1_reg);
    free_regex(config->level2_reg);
    free_regex(config->epilogue_reg);

    for (int i = 0; i < config->filter_count; i++) {
        free_regex(config->filter_list[i]);
    }

    free(config->filter_list);

    chapter *current_chapter = config->first_chapter;
    while (current_chapter) {
        chapter *next_chapter = current_chapter->next;

        free(current_chapter->title);
        free(current_chapter->md_name);
        free(current_chapter->md_path);
        free(current_chapter->epub_name);
        free(current_chapter->epub_path);
        free(current_chapter);

        current_chapter = next_chapter;
    }
}

void config_print(book_config *config) {
    printf("title: %s\n", config->title);
    printf("author: %s\n", config->author);
    printf("cover: %s\n", config->cover);
    printf("prefaceReg: %s\n", config->preface_reg->reg_pattern);
    printf("level1Reg: %s\n", config->level1_reg->reg_pattern);
    printf("level2Reg: %s\n", config->level2_reg->reg_pattern);
    printf("epilogueReg: %s\n", config->epilogue_reg->reg_pattern);
    printf("filterCount: %d\n", config->filter_count);
    printf("chapterNameLengthLimit: %d\n", config->chapter_name_length_limit);
    printf("compress: %s\n", config->compress);
    for (int i = 0; i < config->filter_count; i++) {
        printf("filterList[%d]: %s\n", i, config->filter_list[i]->reg_pattern);
    }
}

void chapter_print(chapter *chapter) {
    printf("-------\n");
    printf("level: %d\n", chapter->level);
    printf("index: %d\n", chapter->index);
    printf("title: %s\n", chapter->title);
    printf("md_name: %s\n", chapter->md_name);
    printf("md_path: %s\n", chapter->md_path);
    printf("epub_name: %s\n", chapter->epub_name);
    printf("epub_path: %s\n", chapter->epub_path);
}
