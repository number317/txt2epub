#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>
#include <libconfig.h>
#include <regex.h>

#define DEFAULT_CONFIG \
    "title = \"%s\"\n" \
    "author = \"Noname\"\n" \
    "cover = \"./cover.jpg\"\n" \
    "prefaceReg = \"^ *(序|序章|楔子|前奏|前言|作品相关)( .*)?$\"\n" \
    "level1Reg = \"^ *(第[零一二三四五六七八九十百千万壹贰叁肆伍陆柒捌玖拾1234567890]*(卷|部|集)|序卷)( .*)?$\"\n" \
    "level2Reg = \"^ *第[零一二三四五六七八九十百千万壹贰叁肆伍陆柒捌玖拾1234567890]*章( .*)?$\"\n" \
    "epilogueReg = \"^ *(后序|终章)( .*)?$\"\n" \
    "filterList = [\n" \
        "\t\"如果您使用第三方APP或各种浏览器插件打开此网站可能.*\",\n" \
        "\t\"第.*章没码完.*\",\n" \
        "\t\"\\(本章完\\)\"\n" \
    "]\n" \
    "chapterNameLengthLimit = 60\n" \
    "compress = \"7z\""

#define MAX_LINE_LENGTH 1024
#define MAX_PATH_LENGTH 256

#define OUT_PATH "./out"
#define BOOK_PATH "./book"
#define EPUB_PATH "./book/epub"
#define TOC_MD_PATH "./out/TOC.md"
#define PREFIX_PATH "./out/preface.md"
#define SUFFIX_PATH "./out/epilogue.md"

typedef struct regex {
    regex_t *reg;
    char *reg_pattern;
} regex;

typedef struct chapter {
    /*
     * level
     * 1: h1
     * 2: h2
     */
    int level;
    int index;
    char *title;
    char *md_path;
    char *md_name;
    char *epub_path;
    char *epub_name;
    struct chapter *next;
} chapter;

typedef struct {
    char *title;
    char *author;
    char *cover;
    regex *preface_reg;
    regex *level1_reg;
    regex *level2_reg;
    regex *epilogue_reg;
    regex **filter_list;
    int filter_count;
    int chapter_name_length_limit;
    char *compress;
    chapter *first_chapter;
} book_config;

void config_load(const char *filepath, book_config *config);
void config_free(book_config *config);
void config_print(book_config *config);
void chapter_print(chapter *chapter);

char *load_string_setting(config_t *cfg, const char *name);
regex *load_regex_setting(config_t *cfg, const char *name);
bool load_bool_setting(config_t *cfg, const char *name);
int load_int_setting(config_t *cfg, const char *name);
regex **load_regex_array(config_t *cfg, const char *name, int *count);

#endif
