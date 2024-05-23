#!/bin/bash -ex

cd "$(dirname "$0")"
cd ..
project_dir="$(pwd)"
echo "Current directory: ${project_dir}"

releases_dir="${project_dir}/packaging/releases"
TAG_NAME="${GITHUB_REF#refs/tags/}"

cd ${releases_dir}
ls -al

cd windows*
mv *.7z Qt-App_${TAG_NAME}_windows_x86_64.7z
mv *.exe Qt-App_${TAG_NAME}_x86_64.exe
mv * ${releases_dir}/
cd ..

cd ubuntu*
mv *.7z Qt-App_${TAG_NAME}_ubuntu_x86_64.7z
mv *.AppImage Qt-App_${TAG_NAME}_x86_64.AppImage
mv *.deb Qt-App_${TAG_NAME}_x86_64.deb
mv * ${releases_dir}/
cd ..

cd macos*x86*
mv *.7z Qt-App_${TAG_NAME}_macos_x86_64.7z
mv *.dmg Qt-App_${TAG_NAME}_x86_64.dmg
mv *.pkg Qt-App_${TAG_NAME}_x86_64.pkg
mv * ${releases_dir}/
cd ..

cd macos*arm*
mv *.7z Qt-App_${TAG_NAME}_macos_aarch64.7z
mv *.dmg Qt-App_${TAG_NAME}_aarch64.dmg
mv *.pkg Qt-App_${TAG_NAME}_aarch64.pkg
mv * ${releases_dir}/
cd ..

cd ${releases_dir}
wget -nv "https://github.com/RealChuan/Qt-App/releases/download/0.0.1/fonts.7z"

rm -rf windows*
rm -rf ubuntu*
rm -rf macos*

ls -al

exit 0
