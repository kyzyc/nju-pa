#!/bin/bash

# convert slides.pdf \
#   -sharpen "0x1.0" \
#   -type truecolor -resize 400x300\! slides.bmp
# 遍历所有 slides-*.pdf 文件
for pdf in slides-*.pdf; do
  # 提取文件名前缀（不包含扩展名）
  base_name=$(basename "$pdf" .pdf)
  
  # 使用 convert 将 PDF 转为 BMP
  convert "$pdf" \
    -sharpen "0x1.0" \
    -type truecolor -resize 400x300\! "$base_name.bmp"
done

mkdir -p $NAVY_HOME/fsimg/share/slides/
rm $NAVY_HOME/fsimg/share/slides/*
mv *.bmp $NAVY_HOME/fsimg/share/slides/
