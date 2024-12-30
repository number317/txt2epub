#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "utils.h"
#include "config.h"

int create_epub_structure(char *book_dir) { 
    if (create_dir(book_dir)) return 1;
    char epub_dir[MAX_PATH_LENGTH];
    snprintf(epub_dir, sizeof(epub_dir), "%s/epub", book_dir);
    if (create_dir(epub_dir)) return 1;

    int base_length = strlen(epub_dir);
    char mimetype_file[base_length + strlen("/mimetype") + 1];
    snprintf(mimetype_file, sizeof(mimetype_file), "%s/mimetype", epub_dir);
    if (write_file_once(mimetype_file, "application/epub+zip")) return 1;

    char meta_inf_dir[base_length + strlen("/META-INF") + 1];
    snprintf(meta_inf_dir, sizeof(meta_inf_dir), "%s/META-INF", epub_dir);
    if (create_dir(meta_inf_dir)) return 1;

    char container_xml[base_length + strlen("/META-INF/container.xml") + 1];
    snprintf(container_xml, sizeof(container_xml), "%s/META-INF/container.xml", epub_dir);
    char *container_xml_content =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?> \n"
        "<container version=\"1.0\" xmlns=\"urn:oasis:names:tc:opendocument:xmlns:container\">\n"
        "    <rootfiles>\n"
        "        <rootfile full-path=\"OEBPS/content.opf\" media-type=\"application/oebps-package+xml\"/>\n"
        "    </rootfiles>\n"
        "</container>\n";
    if (write_file_once(container_xml, container_xml_content)) return 1;

    char oebps_dir[base_length + strlen("/OEBPS") + 1];
    snprintf(oebps_dir, sizeof(oebps_dir), "%s/OEBPS", epub_dir);
    if (create_dir(oebps_dir)) return 1;

    return 0;
}

void escape_ampersand(char *str) {
    char *pos = str;
    while ((pos = strchr(pos, '&')) != NULL) {
        memmove(pos + 5, pos + 1, strlen(pos));
        memcpy(pos, "&amp;", 5);
        pos += 5;
    }
}

void convert_md_to_xhtml(const char *md_file, const char *xhtml_file) {
    FILE *md = fopen(md_file, "r");
    if (!md) {
        fprintf(stderr, "Failed to open .md file: %s\n", md_file);
        exit(EXIT_FAILURE);
    }

    FILE *xhtml = fopen(xhtml_file, "w");
    if (!xhtml) {
        fprintf(stderr, "Failed to open .xhtml file: %s\n", xhtml_file);
        exit(EXIT_FAILURE);
    }

    fprintf(xhtml, "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n");
    fprintf(xhtml, "<html xmlns=\"http://www.w3.org/1999/xhtml\" xmlns:epub=\"http://www.idpf.org/2007/ops\" xml:lang=\"en\" lang=\"en\">\n");
    fprintf(xhtml, "  <head>\n");
    fprintf(xhtml, "    <meta http-equiv=\"Content-Type\" content=\"application/xhtml+xml; charset=UTF-8\"/>\n");
    fprintf(xhtml, "    <title>Chapter</title>\n");
    // fprintf(xhtml, "    <link rel=\"stylesheet\" type=\"text/css\" href=\"style.css\"/>\n");
    fprintf(xhtml, "  </head>\n");
    fprintf(xhtml, "  <body>\n");

    char line[MAX_LINE_LENGTH];
    int in_paragraph = 0;
    char paragraph[MAX_LINE_LENGTH * 10];
    memset(paragraph, 0, sizeof(paragraph));

    while (fgets(line, sizeof(line), md)) {
        escape_ampersand(line);
        line[strcspn(line, "\n")] = 0;

        if (strlen(line) == 0) {
            if (in_paragraph) {
                fprintf(xhtml, "    <p>%s</p>\n", paragraph);
                memset(paragraph, 0, sizeof(paragraph));
                in_paragraph = 0;
            }
        } else {
            if (line[0] == '#') {
                int level = 0;
                while (line[level] == '#') {
                    level++;
                }
                char *content = line + level;
                while (*content == ' ') content++;

                if (level == 1) {
                    fprintf(xhtml, "    <h1>%s</h1>\n", content);
                } else if (level == 2) {
                    fprintf(xhtml, "    <h2>%s</h2>\n", content);
                } else if (level == 3) {
                    fprintf(xhtml, "    <h3>%s</h3>\n", content);
                }
                in_paragraph = 0;
            } else if (strcmp(line, "---") == 0 || strcmp(line, "***") == 0) {
                fprintf(xhtml, "    <hr />\n");
                in_paragraph = 0;
            } else {
                if (in_paragraph) {
                    strcat(paragraph, line);
                    strcat(paragraph, " ");
                } else {
                    strcpy(paragraph, line);
                    strcat(paragraph, " ");
                    in_paragraph = 1;
                }
            }
        }
    }
    if (in_paragraph) {
        fprintf(xhtml, "    <p>%s</p>\n", paragraph);
    }

    fprintf(xhtml, "  </body>\n");
    fprintf(xhtml, "</html>\n");

    fclose(md);
    fclose(xhtml);
}

chapter* generate_nav_item(chapter *current_chapter, FILE *ncx_file, int level) {
    chapter *new_chapter = NULL;
    while (current_chapter != NULL && current_chapter->level == level) {
        convert_md_to_xhtml(current_chapter->md_path, current_chapter->epub_path);
        int indent = (current_chapter->level + 1) * 4;
        fprintf(ncx_file, "%*s<navPoint id=\"navpoint-%d\" playOrder=\"%d\">\n", indent, "", current_chapter->index, current_chapter->index);
        fprintf(ncx_file, "%*s<navLabel>\n", indent + 4, "");
        fprintf(ncx_file, "%*s<text>%s</text>\n", indent + 8, "", current_chapter->title);
        fprintf(ncx_file, "%*s</navLabel>\n", indent + 4, "");
        fprintf(ncx_file, "%*s<content src=\"%s\"/>\n", indent + 4, "", current_chapter->epub_name);

        if (current_chapter->next != NULL && current_chapter->next->level > current_chapter->level && current_chapter->index > 0) {
            current_chapter = current_chapter->next;
            int sub_level = current_chapter->level;
            new_chapter = generate_nav_item(current_chapter, ncx_file, sub_level);
        }
        fprintf(ncx_file, "%*s</navPoint>\n", indent, "");
        if (new_chapter != NULL) {
            current_chapter = new_chapter; 
            new_chapter = NULL;
        } else {
            current_chapter = current_chapter->next;
        }
    }
    return current_chapter;
}

void generate_toc_ncx(book_config *config, char *epub_dir) {
    chapter *current_chapter = config->first_chapter;

    char ncx_file_path[MAX_PATH_LENGTH];
    snprintf(ncx_file_path, sizeof(ncx_file_path), "%s/OEBPS/toc.ncx", epub_dir);
    FILE *ncx_file = fopen(ncx_file_path, "w, ccs=utf-8");
    if (!ncx_file) {
        perror("Failed to open toc.ncx file for writing");
        exit(EXIT_FAILURE);
    }

    fprintf(ncx_file, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    fprintf(ncx_file, "<ncx xmlns=\"http://www.daisy.org/z3986/2005/ncx/\" version=\"2005-1\">\n");
    fprintf(ncx_file, "    <head>\n");
    fprintf(ncx_file, "        <meta name=\"dtb:depth\" content=\"1\"/>\n");
    fprintf(ncx_file, "        <meta name=\"dtb:totalPageCount\" content=\"0\"/>\n");
    fprintf(ncx_file, "        <meta name=\"dtb:maxPageNumber\" content=\"0\"/>\n");
    fprintf(ncx_file, "    </head>\n");
    fprintf(ncx_file, "    <docTitle>\n");
    fprintf(ncx_file, "        <text>%s</text>\n", config->title);
    fprintf(ncx_file, "    </docTitle>\n");
    fprintf(ncx_file, "    <navMap>\n");

    generate_nav_item(current_chapter, ncx_file, 1);

    fprintf(ncx_file, "    </navMap>\n");
    fprintf(ncx_file, "</ncx>\n");

    fclose(ncx_file);
}

void generate_opf(book_config *config) {
    FILE *toc_file = fopen(TOC_MD_PATH, "r");
    if (!toc_file) {
        perror("Failed to open TOC.md file");
        return;
    }
    FILE *opf_file = fopen("./book/epub/OEBPS/content.opf", "w");
    if (!opf_file) {
        perror("Failed to open content.opf for writing");
        return;
    }
    char *current_time = get_current_time();

    fprintf(opf_file, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    fprintf(opf_file, "<package xmlns=\"http://www.idpf.org/2007/opf\" version=\"2.0\" unique-identifier=\"txt2epub\">\n");

    fprintf(opf_file, "  <metadata xmlns:dc=\"http://purl.org/dc/elements/1.1/\" xmlns:opf=\"http://www.idpf.org/2007/opf\">\n");
    fprintf(opf_file, "    <dc:title>%s</dc:title>\n", config->title);
    fprintf(opf_file, "    <dc:creator opf:role=\"aut\">%s</dc:creator>\n", config->author);
    fprintf(opf_file, "    <dc:date>%s</dc:date>\n", current_time);
    fprintf(opf_file, "    <dc:identifier id=\"txt2epub\">urn:isbn:txt2epub</dc:identifier>\n");
    fprintf(opf_file, "    <dc:language>zh</dc:language>\n");
    fprintf(opf_file, "    <dc:publisher>txt2epub</dc:publisher>\n");
    fprintf(opf_file, "    <dc:description>txt2epub by cheon</dc:description>\n");
    fprintf(opf_file, "    <meta name=\"cover\" content=\"cover-image\"/>\n");
    fprintf(opf_file, "  </metadata>\n");

    fprintf(opf_file, "  <manifest>\n");
    fprintf(opf_file, "    <item id=\"toc\" href=\"toc.ncx\" media-type=\"application/x-dtbncx+xml\"/>\n");

    chapter *current_chapter = config->first_chapter;

    while (current_chapter != NULL) {
        fprintf(opf_file, "    <item id=\"chapter-%d\" href=\"%s\" media-type=\"application/xhtml+xml\"/>\n", current_chapter->index, current_chapter->epub_name);
        current_chapter = current_chapter->next;
    }
    fprintf(opf_file, "    <item id=\"cover-image\" href=\"cover.jpg\" media-type=\"image/jpeg\"/>\n");
    fprintf(opf_file, "  </manifest>\n");
    fprintf(opf_file, "  <spine toc=\"toc\">\n");

    current_chapter = config->first_chapter;
    while (current_chapter != NULL) {
        fprintf(opf_file, "    <itemref idref=\"chapter-%d\"/>\n", current_chapter->index);
        current_chapter = current_chapter->next;
    }
    fprintf(opf_file, "  </spine>\n");
    fprintf(opf_file, "</package>\n");
    free(current_time);
    fclose(toc_file);
    fclose(opf_file);
};

void copy_cover(book_config *config) {
    char buffer[1024];
    size_t bytes_read;

    FILE *source = fopen(config->cover, "rb");
    if (source == NULL) {
        fprintf(stderr, "%s not exists!", config->cover);
        exit(EXIT_FAILURE);
    }

    char cover_path[MAX_PATH_LENGTH];
    sprintf(cover_path, "%s/OEBPS/cover.jpg", EPUB_PATH);
    FILE *destination = fopen(cover_path, "wb");
    if (destination == NULL) {
        fclose(source);
        fprintf(stderr, "can't create cover image");
        exit(EXIT_FAILURE);
    }

    while ((bytes_read = fread(buffer, 1, sizeof(buffer), source)) > 0) {
        if (fwrite(buffer, 1, bytes_read, destination) != bytes_read) {
            fprintf(stderr, "can't create cover image");
            fclose(source);
            fclose(destination);
            exit(EXIT_FAILURE);
        }
    }


    fclose(source);
    fclose(destination);
};

void create_zip(book_config *config) {
    char cmd_zip[] =
        "cd book/epub && "
        "zip -X -r9 output.epub mimetype META-INF OEBPS > /dev/null && "
        "mv output.epub ../%s.epub";

    char cmd_7z[] =
        "cd book/epub && "
        "cp mimetype 1mimetype && "
        "7z a -tzip output.epub -mx=0 1mimetype > /dev/null && "
        "7z a -tzip output.epub META-INF OEBPS > /dev/null && "
        "7z rn output.epub 1mimetype mimetype > /dev/null && "
        "rm 1mimetype && "
        "mv output.epub ../%s.epub";

    char *cmd;
    int result;
    if (strcmp(config->compress, "zip") == 0) {
        cmd = malloc(sizeof(char) * (strlen(cmd_zip) + strlen(config->title) + 1));
        snprintf(cmd, strlen(cmd_zip) + strlen(config->title) + 1, cmd_zip, config->title);
        result = system(cmd);
    } else {
        cmd = malloc(sizeof(char) * (strlen(cmd_7z) + strlen(config->title) + 1));
        snprintf(cmd, strlen(cmd_7z) + strlen(config->title) + 1, cmd_7z, config->title);
        result = system(cmd);
    }

    if (result != 0) {
        fprintf(stderr, "Failed to create EPUB file.\n");
        exit(EXIT_FAILURE);
    }
    free(cmd);
};
