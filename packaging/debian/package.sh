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
mv -v ${project_dir}/bin-64/RelWithDebInfo/* ${packet_dir}/
chmod -R +x ${packet_dir}

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

unset SOURCE_DATE_EPOCH
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

rm -f "${packet_dir}"/${app_name}.desktop
cp -vf "${project_dir}/packaging/ubuntu/usr/share/applications/${app_name}.desktop" ${packet_dir}/

chmod 777 "${packet_dir}"/app.png
chmod 744 "${packet_dir}"/${app_name}.desktop
