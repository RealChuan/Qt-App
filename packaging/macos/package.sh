#!/bin/bash -ex

cd "$(dirname "$0")"
cd ../..
project_dir="$(pwd)"
echo "Current directory: ${project_dir}"

packet_dir="${project_dir}/packaging/packet"
releases_dir="${project_dir}/packaging/releases"

sudo chmod -R +x ${packet_dir}
mv -vf "${packet_dir}/fonts" "${packet_dir}/Qt-App.app/Contents/MacOS/"

macdeployqt "${packet_dir}/Qt-App.app" -always-overwrite

ls -al "${packet_dir}/Qt-App.app/Contents/Frameworks"

pip3 install dmgbuild

cd "${packet_dir}"
dmgbuild -s "${project_dir}/packaging/macos/dmgbuild.py" "Qt-App.app" "Qt-App.dmg"

# 排除的文件名
EXCLUDE_FILE="Qt-App.dmg"
find . -maxdepth 1 ! -name "$EXCLUDE_FILE" ! -name "." ! -name ".." -exec rm -rf -- {} +
cd "${project_dir}"

echo "Deployment macos completed."
