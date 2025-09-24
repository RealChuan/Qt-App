#!/bin/bash -ex

app_name="Qt-App"

cd "$(dirname "$0")"
cd ../..
project_dir="$(pwd)"
echo "Current directory: ${project_dir}"

packet_dir="${project_dir}/packaging/packet"
releases_dir="${project_dir}/packaging/releases"

chmod -R +x ${packet_dir}

# 安装linuxdeployqt
sudo add-apt-repository universe
sudo apt update
sudo apt install -y libfuse2 libxcb-cursor0
wget -nv "https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage" \
    -O /usr/local/bin/linuxdeployqt
sudo chmod +x /usr/local/bin/linuxdeployqt

cp -vf "${project_dir}/packaging/ubuntu/${app_name}.desktop" ${packet_dir}/
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

linuxdeployqt ${packet_dir}/${app_name} \
    -executable=${packet_dir}/CrashReport \
    -qmake=qmake \
    -always-overwrite \
    -unsupported-allow-new-glibc \
    -appimage || true
chmod +x *.AppImage
cd "${project_dir}"

mv -v "${packet_dir}"/*.AppImage "$releases_dir"/${app_name}.AppImage

# package with 7z
zip_path="${releases_dir}/${app_name}.7z"
7z a -t7z -r -mx=9 -mmt "${zip_path}" "${packet_dir}"/*

# package with deb
mkdir -p "${project_dir}"/packaging/${app_name}/
mv -v "${packet_dir}"/* "${project_dir}"/packaging/${app_name}
mkdir -p "${packet_dir}"/opt
mv -v "${project_dir}"/packaging/${app_name} "${packet_dir}"/opt/
cp -rv "${project_dir}"/packaging/ubuntu/DEBIAN "${packet_dir}"/
cp -rv "${project_dir}"/packaging/ubuntu/usr "${packet_dir}"/
cp -v "${project_dir}"/packaging/ubuntu/${app_name}.sh "${packet_dir}"/opt/${app_name}/

chmod -R +x "${packet_dir}"/DEBIAN
chmod 777 "${packet_dir}"/opt/${app_name}/app.png
chmod 744 "${packet_dir}"/usr/share/applications/${app_name}.desktop

deb_path="${releases_dir}/${app_name}.deb"
sudo dpkg -b ${packet_dir}/. ${deb_path}

sudo chmod -R +x ${releases_dir}

echo "Deployment ubuntu completed."
