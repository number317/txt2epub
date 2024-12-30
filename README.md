# txt2epub

A small and fast tool to convert TXT to EPUB.

## Dependencies

- libconfig to parse config
- zip or 7z  to create epub

This program focus on web novel and just conver "h1", "h2", "hr" and "p" tags. So it doesn't depend on any epub library and does't support images content yet.

## Usage

```bash
txt2epub init sample  # This will create a novel directory `sample` with default config file.
cd sample
vim config.cfg # modify the config file.
cp /path/cover.jpg ./ # copy the cover image
cp /path/sample.txt ./ # copy the txt file
txt2epub generate sample.txt # epub will be generated in `book/sample.epub`
```

<details>
<summary>Usage</summary>

```bash
txt2epub -?
Usage: txt2epub [OPTION...] <command> [args...]
txt2epub - A tool to generate epub from txt

Commands:

  init <novel_dir>     Initialize the novel directory
  generate <txt_file>  Generate epub from txt file
  showconf             Print current config file

Options:

  -c, --config=config_path   Override the default config (only for generate and
                             showconf)
  -?, --help                 Give this help list
      --usage                Give a short usage message
  -V, --version              Print program version
```


</details>

## Config

```conf
title = "sample"
author = "Noname"
cover = "./cover.jpg"
prefaceReg = "^ *(序|序章|楔子|前奏|前言|作品相关)( .*)?$"
level1Reg = "^ *(第[零一二三四五六七八九十百千万壹贰叁肆伍陆柒捌玖拾1234567890]*(卷|部|集))|序卷( .*)?$"
level2Reg = "^ *第[零一二三四五六七八九十百千万壹贰叁肆伍陆柒捌玖拾1234567890]*章( .*)?$"
epilogueReg = "^ *(后序|终章)( .*)?$"
filterList = []
chapterNameLengthLimit = 60
compress = "7z"
```

`chapterNameLengthLimit` is the max length of chapter name. The default is 60 (20 characters for Chinese). When chapter name is too long, it means your regex may be wrong. `txt2epub` will stop until you increase this value or fix your regex.
`filterList` is a list of regex that will be ignored when convert. You can use it to filter junk content.

## Output

- `out` folder contains the generated chapter and toc markdown files.
- `book` folder contains the generated epub file and epub directory.
- `book/epub` contains the basic epub structure. You can add content to epub then create epub manually.

# FAQ

- How to create epub manually?

  use command zip:

  ```bash
  cd book/epub
  zip -X -r9 output.epub mimetype META-INF OEBPS
  mv output.epub ../output.epub
  ```

  or use command 7z:

  ```bash
  cd book/epub
  cp mimetype 1mimetype
  7z a -tzip output.epub -mx=0 1mimetype > /dev/null
  7z a -tzip output.epub META-INF OEBPS > /dev/null
  7z rn output.epub 1mimetype mimetype > /dev/null
  rm 1mimetype
  mv output.epub ../output.epub;
  ```

  just like what this program does.

- How to create mobi from epub?

  You need `kindlegen` to convert epub to mobi.

  ```bash
  kindlegen book/output.epub -c1 -locale zh -dont_append_source
  ```

- EPUB cover is too small/large?

  `txt2epub` won't change the cover size. And you need to resize it manually. For example use image magick:

  ```bash
  magick cover.jpg -resize 600x800! cover_new.jpg
  ```
