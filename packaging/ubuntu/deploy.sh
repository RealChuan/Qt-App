#!/bin/bash

# 获取当前目录的父目录
parent_dir=$(dirname "$(pwd)")

# 检查父目录是否是 'bin-64'
if [[ "$parent_dir" != */bin-64 ]]; then
    echo "警告：当前目录的父目录不是 'bin-64'"
    exit 1
fi

# 添加universe仓库
sudo add-apt-repository universe
sudo apt update

# 安装依赖
sudo apt install -y libfuse2 libxcb-cursor0

# 下载linuxdeployqt
wget -c -nv "https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage"

# 给予执行权限
sudo chmod -R a+x linuxdeployqt

# 复制桌面文件和图标
cp -f ./../../packaging/ubuntu/Qt-App.desktop .
cp -f ./../../src/resource/icon/app.png .
cp -f ./plugins/* .

# 使用linuxdeployqt部署
so_files=$(find . -maxdepth 1 -name "*.so" -printf '%P\n')
for so_file in $so_files; do
    ./linuxdeployqt-continuous-x86_64.AppImage \
        $so_file \
        -qmake=qmake \
        -always-overwrite -unsupported-allow-new-glibc
done
rm -f AppRun
./linuxdeployqt-continuous-x86_64.AppImage \
    Qt-App \
    -executable=CrashReport \
    -qmake=qmake \
    -always-overwrite -unsupported-allow-new-glibc \
    -appimage

# 清理
rm linuxdeployqt-continuous-x86_64.AppImage
rm -f *.so

echo "Deployment ubuntu completed."
