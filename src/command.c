#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <sys/stat.h>
#include "command.h"
#include "config.h"
#include "utils.h"
#include "epub.h"

int init_project(char *dir) {
    if (create_dir(dir)) {
        exit(EXIT_FAILURE);
    }

    if (create_config(dir)) {
        exit(EXIT_FAILURE);
    }
    return 0;
}

int create_config(char *dir) {
    char config_path[MAX_PATH_LENGTH];
    snprintf(config_path, sizeof(config_path), "%s/config.cfg", dir);

    FILE *file = fopen(config_path, "w");
    if (file == NULL) {
        perror("Error creating config file");
        exit(EXIT_FAILURE);
    }

    fprintf(file, DEFAULT_CONFIG, dir);
    fclose(file);

    return 0;
}

chapter *init_chapter() {
    chapter *chapter = malloc(sizeof(struct chapter));
    chapter->title = malloc(sizeof(char) * MAX_PATH_LENGTH);
    chapter->md_name = malloc(sizeof(char) * MAX_PATH_LENGTH);
    chapter->md_path = malloc(sizeof(char) * MAX_PATH_LENGTH);
    chapter->epub_name = malloc(sizeof(char) * MAX_PATH_LENGTH);
    chapter->epub_path = malloc(sizeof(char) * MAX_PATH_LENGTH);
    chapter->next = NULL;
    return chapter;
};

void free_chatper(chapter *chapter) {
    free(chapter->title);
    free(chapter->md_name);
    free(chapter->md_path);
    free(chapter->epub_name);
    free(chapter->epub_path);
    free(chapter);
}

int split_txt_to_md(char *txt_file, book_config *config) {
    if (create_dir(OUT_PATH)) {
        return 1;
    }
    FILE *txt = fopen(txt_file, "r");
    if (!txt) {
        perror("Error opening txt file");
        return 1;
    }
    FILE *toc = fopen(TOC_MD_PATH, "w");
    if (!toc) {
        perror("Error opening ./out/TOC.md");
        fclose(txt);
        return 1;
    }

    int index_level1 = 0;
    int index_level2 = 0;
    char line[MAX_LINE_LENGTH];

    int chapter_index = 0;
    int line_count = 0;
    FILE *temp_file = NULL;

    chapter *current_chapter = init_chapter();
    current_chapter->level = 1;
    current_chapter->index = chapter_index;
    config->first_chapter = current_chapter;

    while (fgets(line, sizeof(line), txt)) {
        line_count++;
        line[strcspn(line, "\n")] = '\0';
        if (filter_check(line, config)) {
            continue;
        }
        if (regexec(config->preface_reg->reg, line, 0, NULL, 0) == 0) {
            if ((int) strlen(line) > config->chapter_name_length_limit) {
                fprintf(stderr, "preface chapter title is too long: %d %s\n", line_count, line);
                exit(EXIT_FAILURE);
            }
            fclose(temp_file);
            chapter_index++;
            chapter *new_chapter = init_chapter();
            current_chapter->next = new_chapter;
            current_chapter = new_chapter;
            current_chapter->level = 1;
            current_chapter->index = chapter_index;
            sprintf(current_chapter->title, "%s", line);
            sprintf(current_chapter->md_name, "preface.md");
            sprintf(current_chapter->md_path, "%s/preface.md", OUT_PATH);
            sprintf(current_chapter->epub_name, "preface.xhtml");
            sprintf(current_chapter->epub_path, "%s/OEBPS/preface.xhtml", EPUB_PATH);
            fprintf(toc, "[%s](%s)\n", current_chapter->title, current_chapter->md_name);
            temp_file = fopen(current_chapter->md_path, "w");
            fprintf(temp_file, "# %s\n\n", line);
            continue;
        }
        if (regexec(config->level1_reg->reg, line, 0, NULL, 0) == 0) {
            if ((int) strlen(line) > config->chapter_name_length_limit) {
                fprintf(stderr, "level1 chapter title is too long: %d %s\n", line_count, line);
                exit(EXIT_FAILURE);
            }
            fclose(temp_file);
            chapter_index++;
            index_level1++;
            chapter *new_chapter = init_chapter();
            current_chapter->next = new_chapter;
            current_chapter = new_chapter;
            current_chapter->level = 1;
            current_chapter->index = chapter_index;
            sprintf(current_chapter->title, "%s", line);
            sprintf(current_chapter->md_name, "chapter-%d.md", index_level1);
            sprintf(current_chapter->md_path, "%s/chapter-%d.md", OUT_PATH, index_level1);
            sprintf(current_chapter->epub_name, "chapter-%d.xhtml", index_level1);
            sprintf(current_chapter->epub_path, "%s/OEBPS/chapter-%d.xhtml", EPUB_PATH, index_level1);
            fprintf(toc, "- [%s](%s)\n", current_chapter->title, current_chapter->md_name);
            temp_file = fopen(current_chapter->md_path, "w");
            fprintf(temp_file, "# %s\n\n", line);
            continue;
        }
        if (regexec(config->level2_reg->reg, line, 0, NULL, 0) == 0) {
            if ((int) strlen(line) > config->chapter_name_length_limit) {
                fprintf(stderr, "level2 chapter title is too long: %d %s\n", line_count, line);
                exit(EXIT_FAILURE);
            }
            fclose(temp_file);
            chapter_index++;
            index_level2++;
            chapter *new_chapter = init_chapter();
            current_chapter->next = new_chapter;
            current_chapter = new_chapter;
            current_chapter->level = index_level1 > 0 ? 2 : 1;
            current_chapter->index = chapter_index;
            sprintf(current_chapter->title, "%s", line);
            if (index_level1 > 0) {
                sprintf(current_chapter->md_name, "chapter-%d.%d.md", index_level1, index_level2);
                sprintf(current_chapter->md_path, "%s/chapter-%d.%d.md", OUT_PATH, index_level1, index_level2);
                sprintf(current_chapter->epub_name, "chapter-%d.%d.xhtml", index_level1, index_level2);
                sprintf(current_chapter->epub_path, "%s/OEBPS/chapter-%d.%d.xhtml", EPUB_PATH, index_level1, index_level2);
            } else {
                sprintf(current_chapter->md_name, "chapter-%d.md", index_level2);
                sprintf(current_chapter->md_path, "%s/chapter-%d.md", OUT_PATH, index_level2);
                sprintf(current_chapter->epub_name, "chapter-%d.xhtml", index_level2);
                sprintf(current_chapter->epub_path, "%s/OEBPS/chapter-%d.xhtml", EPUB_PATH, index_level2);
            }
            fprintf(toc, "    - [%s](%s)\n", line, current_chapter->md_name);
            temp_file = fopen(current_chapter->md_path, "w");
            fprintf(temp_file, "## %s\n\n", line);
            continue;
        }
        if (regexec(config->epilogue_reg->reg, line, 0, NULL, 0) == 0) {
            if ((int) strlen(line) > config->chapter_name_length_limit) {
                fprintf(stderr, "epilogue chapter title is too long: %d %s\n", line_count, line);
                exit(EXIT_FAILURE);
            }
            fclose(temp_file);
            chapter_index++;
            chapter *new_chapter = init_chapter();
            current_chapter->next = new_chapter;
            current_chapter = new_chapter;
            current_chapter->level = 1;
            current_chapter->index = chapter_index;
            sprintf(current_chapter->title, "%s", line);
            sprintf(current_chapter->md_name, "epilogue.md");
            sprintf(current_chapter->md_path, "./out/epilogue.md");
            sprintf(current_chapter->epub_name, "epilogue.xhtml");
            sprintf(current_chapter->epub_path, "%s/OEBPS/epilogue.xhtml", EPUB_PATH);
            fprintf(toc, "[%s](epilogue.md)\n", line);
            temp_file = fopen(current_chapter->md_path, "w");
            fprintf(temp_file, "# %s\n\n", line);
            continue;
        }

        if (temp_file == NULL) {
            sprintf(current_chapter->title, "简介");
            sprintf(current_chapter->md_name, "summary.md");
            sprintf(current_chapter->md_path, "./out/summary.md");
            sprintf(current_chapter->epub_name, "summary.xhtml");
            sprintf(current_chapter->epub_path, "%s/OEBPS/summary.xhtml", EPUB_PATH);
            fprintf(toc, "[%s](%s)\n", line, current_chapter->md_name);
            temp_file = fopen(current_chapter->md_path, "w");
            fprintf(temp_file, "%s\n\n", line);
            continue;
        } else {
            fprintf(temp_file, "%s\n\n", line);
            continue;
        }
    }
    if (temp_file != NULL) fclose(temp_file);
    fclose(toc);
    fclose(txt);
    return 0;
}

int filter_check(char *str, book_config *config) {
    for (int i=0; i < config->filter_count; i++) {
        if (regexec(config->filter_list[i]->reg, str, 0, NULL, 0) == 0) {
            return 1;
        }
    }
    return 0;
}

void generate_epub(book_config *config, char *txt_path) {
    split_txt_to_md(txt_path, config);
    create_epub_structure("./book");
    generate_toc_ncx(config, "book/epub");
    generate_opf(config);
    copy_cover(config);
    create_zip(config);
}
