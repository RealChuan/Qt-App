#!/bin/bash

# 获取当前目录的父目录
parent_dir=$(dirname "$(pwd)")

# 检查父目录是否是 'bin-64'
if [[ "$parent_dir" != */bin-64 ]]; then
    echo "警告：当前目录的父目录不是 'bin-64'"
    exit 1
fi

sudo chmod -R +x .

# 添加universe仓库
sudo add-apt-repository universe
sudo apt update

# 安装依赖
sudo apt install -y libfuse2 libxcb-cursor0

# 下载linuxdeployqt
wget -c -nv "https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage" \
    -O /usr/local/bin/linuxdeployqt

# 给予执行权限
sudo chmod +x /usr/local/bin/linuxdeployqt

# 复制桌面文件和图标
cp -f ./../../packaging/ubuntu/Qt-App.desktop .
cp -f ./../../src/resource/icon/app.png .
cp -f ./plugins/* .

# 删除测试文件
rm -rf examples
rm -f *Test*

export LD_LIBRARY_PATH=.:./libs:$LD_LIBRARY_PATH

# 使用linuxdeployqt部署
so_files=$(find . -maxdepth 1 -name "*.so" -printf '%P\n')
for so_file in $so_files; do
    linuxdeployqt \
        $so_file \
        -qmake=qmake \
        -always-overwrite \
        -unsupported-allow-new-glibc
done

# 删除插件
rm -f *plugin*
rm -f AppRun
mv -f *.so ./lib

linuxdeployqt \
    Qt-App \
    -executable=CrashReport \
    -qmake=qmake \
    -always-overwrite \
    -unsupported-allow-new-glibc \
    -appimage

chmod +x *.AppImage

echo "Deployment ubuntu completed."
