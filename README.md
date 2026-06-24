# txt2epub &nbsp; [![EN](https://img.shields.io/badge/EN-blue?style=flat-square)](./README_EN.md)

TXT 小说转 EPUB 工具，轻量快速。

## 依赖

- libconfig — 解析配置文件
- zip 或 7z — 打包 EPUB

仅支持 `h1`、`h2`、`hr`、`p` 四种 HTML 标签，不依赖任何 EPUB 库，暂不支持图片内容。

## 使用

```bash
txt2epub init sample  # 初始化小说目录，生成默认配置
cd sample
vim config.cfg        # 修改配置
cp /path/cover.jpg ./ # 复制封面
cp /path/sample.txt ./ # 复制 TXT 文件
txt2epub generate sample.txt  # 生成 EPUB，输出到 book/sample.epub
```

## 配置

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

### 配置项说明

| 字段 | 说明 |
|---|---|
| `title` | 书名 |
| `author` | 作者 |
| `description` | 书籍描述，支持 `\n` 换行 |
| `descriptionFile` | 描述文本文件路径（支持真实换行） |
| `cover` | 封面图片路径 |
| `prefaceReg` | 匹配序章/前言的正则 |
| `level1Reg` | 匹配卷/部/集标题的正则（h1） |
| `level2Reg` | 匹配章节标题的正则（h2） |
| `epilogueReg` | 匹配后序/终章的正则 |
| `filterList` | 过滤行正则列表，用于过滤广告等垃圾内容 |
| `chapterNameLengthLimit` | 标题最大长度（默认60，约20个汉字）。匹配到的标题超过此长度说明正则可能有问题 |
| `compress` | 压缩工具，`"zip"` 或 `"7z"` |

### 描述字段优先级

1. **`description`** — 配置文件中直接填写，支持 `\n` 换行。示例：`description = "一部精彩的玄幻小说\n讲述了一个传奇故事"`
2. **`descriptionFile`** — 外部文本文件路径，支持真实换行。示例：`descriptionFile = "./description.txt"`
3. **自动提取** — 自动提取第一个章节标题前的内容作为描述（前500字符）。会自动过滤 `书名：`、`作者：`、`简介：` 等元数据行
4. **回落** — 以上均不可用时使用 `"txt2epub by cheon"`

### 常见 TXT 开头格式示例

以下展示几种常见的网络小说 TXT 格式，默认正则均可正确识别：

**格式1 — 直接开篇：**
```text
第一章 初入江湖
江湖险恶，人心叵测。
张三丰踏入这个陌生的世界。

第二章 拜师学艺
长老看着眼前的少年，微微点头。
```

**格式2 — 分卷分章：**
```text
第一卷 斗气大陆
第一章 异界重生
萧炎从梦中惊醒，发现自己回到了多年前。

第二章 测试
测试广场上人山人海。
```

**格式3 — 含序章和元数据：**
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

**格式4 — 多卷编号：**
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

## 输出目录

- `out/` — 生成的各章节 markdown 文件和目录
- `book/` — 生成的 EPUB 文件及中间文件
- `book/epub/` — EPUB 原始结构，可手动修改后重新打包

## 常见问题

- **如何手动打包 EPUB？**

  使用 zip：
  ```bash
  cd book/epub
  zip -X -r9 output.epub mimetype META-INF OEBPS
  mv output.epub ../output.epub
  ```

  使用 7z：
  ```bash
  cd book/epub
  cp mimetype 1mimetype
  7z a -tzip output.epub -mx=0 1mimetype > /dev/null
  7z a -tzip output.epub META-INF OEBPS > /dev/null
  7z rn output.epub 1mimetype mimetype > /dev/null
  rm 1mimetype
  mv output.epub ../output.epub
  ```

- **如何转换为 mobi 格式？**

  需要 `kindlegen`：
  ```bash
  kindlegen book/output.epub -c1 -locale zh -dont_append_source
  ```

- **封面图片尺寸不对？**

  `txt2epub` 不会修改封面图，可用 ImageMagick 自行调整：
  ```bash
  magick cover.jpg -resize 600x800! cover_new.jpg
  ```
