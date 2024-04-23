#!/bin/bash

# 获取当前目录的父目录
parent_dir=$(dirname "$(pwd)")

# 检查父目录是否是 'bin-64'
if [[ "$parent_dir" != */bin-64 ]]; then
    echo "警告：当前目录的父目录不是 'bin-64'"
    exit 1
fi

# 如果目录存在plugins子目录，则复制其内容到当前目录
if [ -d "./plugins" ]; then
    cp -rf ./plugins/* .
fi

# 找到当前目录下所有的.exe和.dll文件，排除crashpad_handler.exe，并使用windeployqt进行部署
files=$(find . -maxdepth 1 -type f -printf '%P\n')
for file in $files; do
    if [[ $file == *.exe || $file == *.dll ]] && [[ $file != crashpad_handler.exe ]]; then
        echo "Deploying $file..."
        windeployqt --force --compiler-runtime $file
    fi
done

# 要删除的文件模式列表
DELETE_PATTERNS=("*Test*" "*plugin*")

# 执行删除操作，但不包括子目录中的文件
find . -maxdepth 1 -type f -name "${DELETE_PATTERNS[@]}" -exec rm -f {} +

# 删除examples文件夹及其内容
rm -rf examples

echo "Deployment windows completed."
