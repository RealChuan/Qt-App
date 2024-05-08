#!/bin/bash -ex

cd "$(dirname "$0")"
cd ../..
project_dir="$(pwd)"
echo "Current directory: ${project_dir}"

packet_dir="${project_dir}/packaging/packet"
releases_dir="${project_dir}/packaging/releases"

sudo chmod -R +x ${packet_dir}

# 安装linuxdeployqt
sudo add-apt-repository universe
sudo apt update
sudo apt install -y libfuse2 libxcb-cursor0
wget -nv "https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage" \
    -O /usr/local/bin/linuxdeployqt
sudo chmod +x /usr/local/bin/linuxdeployqt

cp -vf "${project_dir}/packaging/ubuntu/Qt-App.desktop" ${packet_dir}/
cp -vf "${project_dir}/src/resource/icon/app.png" ${packet_dir}/

plugins="${packet_dir}/plugins"
if [ -d "${plugins}" ]; then
    cp -vf "${plugins}"/* ${packet_dir}/
fi

# 删除测试文件
rm -rf "${packet_dir}/examples"
rm -f "${packet_dir}"/*Test*

export LD_LIBRARY_PATH=${packet_dir}:${packet_dir}/lib:$LD_LIBRARY_PATH

# 使用linuxdeployqt部署
cd "${packet_dir}"
so_files=$(find . -maxdepth 1 -name "*.so" -printf '%P\n')
for so_file in ${so_files}; do
    linuxdeployqt ${so_file} \
        -qmake=qmake \
        -always-overwrite \
        -unsupported-allow-new-glibc || true
done

# 删除插件
rm -f "${packet_dir}"/*plugin*.so
rm -f "${packet_dir}"/AppRun
mv -vf "${packet_dir}/"*.so "${packet_dir}/lib"

linuxdeployqt ${packet_dir}/Qt-App \
    -executable=${packet_dir}/CrashReport \
    -qmake=qmake \
    -always-overwrite \
    -unsupported-allow-new-glibc \
    -appimage || true
sudo chmod +x *.AppImage
cd "${project_dir}"

echo "Deployment ubuntu completed."
