# txt2epub &nbsp; [![ZH](https://img.shields.io/badge/ZH-green?style=flat-square)](./README.md)

A small and fast tool to convert TXT to EPUB.

## Dependencies

- libconfig to parse config
- zip or 7z  to create epub

This program focuses on Chinese web novels and only supports "h1", "h2", "hr" and "p" tags. It doesn't depend on any epub library and doesn't support image content yet.

## Usage

```bash
txt2epub init sample  # Creates a novel directory `sample` with default config.
cd sample
vim config.cfg        # Modify the config file.
cp /path/cover.jpg ./ # Copy the cover image.
cp /path/sample.txt ./ # Copy the txt file.
txt2epub generate sample.txt  # EPUB will be generated at `book/sample.epub`.
```

## Config

```conf
title = "sample"
author = "Noname"
description = ""
descriptionFile = ""
cover = "./cover.jpg"
prefaceReg = "^ *(序|序章|楔子|前奏|前言|作品相关)( .*)?$"
level1Reg = "^ *(第[零一二三四五六七八九十百千万壹贰叁肆伍陆柒捌玖拾1234567890]*(卷|部|集)|序卷)( .*)?$"
level2Reg = "^ *第[零一二三四五六七八九十百千万壹贰叁肆伍陆柒捌玖拾1234567890]*章( .*)?$"
epilogueReg = "^ *(后序|终章)( .*)?$"
filterList = []
chapterNameLengthLimit = 60
compress = "7z"
```

### Config Fields

| Field | Description |
|---|---|
| `title` | Book title |
| `author` | Author name |
| `description` | Book description (use `\n` for newlines) |
| `descriptionFile` | Path to a text file with description |
| `cover` | Cover image path |
| `prefaceReg` | Regex to match preface/foreword headers |
| `level1Reg` | Regex to match volume-level headers (h1) |
| `level2Reg` | Regex to match chapter-level headers (h2) |
| `epilogueReg` | Regex to match epilogue/afterword headers |
| `filterList` | Array of regex patterns to filter out junk lines |
| `chapterNameLengthLimit` | Max title length (default 60, ~20 Chinese chars). If matched title is too long, your regex may be wrong. |
| `compress` | Compression tool: `"zip"` or `"7z"` |

### Description Resolution Priority

1. **`description`** — Inline string, supports `\n` for newlines. Example: `description = "A great fantasy novel\nThe story begins..."`
2. **`descriptionFile`** — External text file with real newlines. Example: `descriptionFile = "./description.txt"`
3. **Auto-extract** — Text before the first chapter heading (first 500 chars). Metadata lines like `书名：`, `作者：`, `简介：` are automatically filtered out.
4. **Fallback** — `"txt2epub by cheon"`

### Novel Opening Examples

The following examples show common TXT formats that the default regex patterns can handle:

**Format 1 — Direct chapter start:**
```text
第一章 初入江湖
江湖险恶，人心叵测。
张三丰踏入这个陌生的世界。

第二章 拜师学艺
长老看着眼前的少年，微微点头。
```

**Format 2 — With volume and chapter:**
```text
第一卷 斗气大陆
第一章 异界重生
萧炎从梦中惊醒，发现自己回到了多年前。

第二章 测试
测试广场上人山人海。
```

**Format 3 — With preface and metadata:**
```text
书名：盘龙
作者：我吃西红柿
简介：
    大陆上传说中的四大神兽家族，
    实际上源自于主神界下凡的四位主神。

序章
    天地鸿蒙，混沌初开。
    一道雷霆划破长空。

第一章 陨落的天才
林雷盘膝坐在床上。
```

**Format 4 — Multiple volumes with chapters:**
```text
第1卷 校园篇
第1章 开学第一天
新的学期开始了。

第2章 新同学
教室里来了一位转学生。

第2卷 冒险篇
第3章 出发
冒险的旅程正式开启。
```

## Output

- `out/` — Generated chapter markdown files and TOC.
- `book/` — Generated EPUB file and intermediate files.
- `book/epub/` — Raw EPUB structure. You can modify and repack it manually.

## FAQ

- **How to create epub manually?**

  With zip:
  ```bash
  cd book/epub
  zip -X -r9 output.epub mimetype META-INF OEBPS
  mv output.epub ../output.epub
  ```

  With 7z:
  ```bash
  cd book/epub
  cp mimetype 1mimetype
  7z a -tzip output.epub -mx=0 1mimetype > /dev/null
  7z a -tzip output.epub META-INF OEBPS > /dev/null
  7z rn output.epub 1mimetype mimetype > /dev/null
  rm 1mimetype
  mv output.epub ../output.epub
  ```

- **How to create mobi from epub?**

  You need `kindlegen`:
  ```bash
  kindlegen book/output.epub -c1 -locale zh -dont_append_source
  ```

- **EPUB cover is too small/large?**

  `txt2epub` doesn't resize the cover. Use ImageMagick:
  ```bash
  magick cover.jpg -resize 600x800! cover_new.jpg
  ```
