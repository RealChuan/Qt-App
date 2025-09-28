#!/bin/bash -ex

app_name="Qt-App"

cd "$(dirname "$0")"
cd ..
project_dir="$(pwd)"
echo "Current directory: ${project_dir}"

releases_dir="${project_dir}/packaging/releases"
TAG_NAME="${GITHUB_REF#refs/tags/}"

cd ${releases_dir}
ls -al

cd windows*
mv *.7z ${app_name}_${TAG_NAME}_windows_x86_64.7z
mv *.exe ${app_name}_${TAG_NAME}_x86_64.exe
mv * ${releases_dir}/
cd ..

cd ubuntu*
mv *.7z ${app_name}_${TAG_NAME}_ubuntu_x86_64.7z
mv *.AppImage ${app_name}_${TAG_NAME}_x86_64.AppImage
mv *.deb ${app_name}_${TAG_NAME}_x86_64.deb
mv * ${releases_dir}/
cd ..

cd macos*x86*
mv *.7z ${app_name}_${TAG_NAME}_macos_x86_64.7z
mv *.dmg ${app_name}_${TAG_NAME}_x86_64.dmg
mv *.pkg ${app_name}_${TAG_NAME}_x86_64.pkg
mv * ${releases_dir}/
cd ..

cd macos*arm*
mv *.7z ${app_name}_${TAG_NAME}_macos_aarch64.7z
mv *.dmg ${app_name}_${TAG_NAME}_aarch64.dmg
mv *.pkg ${app_name}_${TAG_NAME}_aarch64.pkg
mv * ${releases_dir}/
cd ..

cd macos*universal*
mv *.7z ${app_name}_${TAG_NAME}_macos_universal.7z
mv *.dmg ${app_name}_${TAG_NAME}_universal.dmg
mv *.pkg ${app_name}_${TAG_NAME}_universal.pkg
mv * ${releases_dir}/
cd ..

cd ${releases_dir}
wget -nv "https://github.com/RealChuan/${app_name}/releases/download/0.0.1/fonts.7z"

rm -rf windows*
rm -rf ubuntu*
rm -rf macos*

ls -al

exit 0
