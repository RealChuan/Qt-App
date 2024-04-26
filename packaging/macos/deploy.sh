#!/bin/bash

# 获取当前目录的父目录
parent_dir=$(dirname "$(pwd)")

# 检查父目录是否是 'bin-64'
if [[ "$parent_dir" != */bin-64 ]]; then
    echo "警告：当前目录的父目录不是 'bin-64'"
    exit 1
fi

sudo chmod -R +x .

# 部署Qt应用
macdeployqt "Qt-App.app" -always-overwrite

# 列出Frameworks目录
ls -al Qt-App.app/Contents/Frameworks

# 安装dmgbuild
pip3 install dmgbuild

# 创建dmg包
dmgbuild -s ./../../packaging/macos/Qt-App.dmgbuild.py "Qt-App" "Qt-App.dmg"

# 排除的文件名
EXCLUDE_FILE="Qt-App.dmg"

# 找到当前目录下除了排除文件以外的所有文件和文件夹
# 然后删除它们，但不删除当前目录和父目录
find . -maxdepth 1 ! -name "$EXCLUDE_FILE" ! -name "." ! -name ".." -exec rm -rf -- {} +

echo "Deployment macos completed."
