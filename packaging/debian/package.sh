#!/bin/bash -ex

app_name="Qt-App"

cd "$(dirname "$0")"
cd ..
project_dir="$(pwd)"
echo "Current directory: ${project_dir}"

packet_dir="${project_dir}/packaging/packet"
releases_dir="${project_dir}/packaging/releases"

mkdir -p ${packet_dir}
mkdir -p ${releases_dir}
cp -avf ${project_dir}/binaries/*/bin/*/* ${packet_dir}/

cp -vf "${project_dir}/packaging/ubuntu/${app_name}.desktop" ${packet_dir}/
cp -vf "${project_dir}/src/resource/icon/app.png" ${packet_dir}/

chmod +x ${packet_dir}/${app_name} \
	${packet_dir}/CrashReport \
	${packet_dir}/crashpad_handler
chmod 644 ${packet_dir}/*.desktop
chmod 644 ${packet_dir}/app.png

# 删除测试文件
rm -rf "${packet_dir}/examples"
rm -f ${packet_dir}/*Test*
ls -al ${packet_dir}

# 使用linuxdeployqt部署
cd ${packet_dir}
unset SOURCE_DATE_EPOCH
linuxdeployqt ${packet_dir}/${app_name} \
	-executable=${packet_dir}/CrashReport \
	-executable-dir=${packet_dir}/plugins/qt-app/ \
	-qmake=qmake \
	-always-overwrite \
	-unsupported-allow-new-glibc \
	-appimage || true
chmod +x *.AppImage
mv -v *.AppImage ${releases_dir}/${app_name}.AppImage
rm -f .DirIcon AppRun 
cd ${project_dir}

# package with 7z
zip_path="${releases_dir}/${app_name}.7z"
7z a -t7z -mx=9 -mmt "${zip_path}" ${packet_dir}/*

rm -f ${packet_dir}/${app_name}.desktop
cp -vf "${project_dir}/packaging/ubuntu/usr/share/applications/${app_name}.desktop" ${packet_dir}/
cp -v ${project_dir}/packaging/ubuntu/${app_name}.sh ${packet_dir}/

chmod 644 ${packet_dir}/*.desktop
chmod +x ${packet_dir}/*.sh
