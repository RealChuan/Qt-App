#!/bin/bash -ex

cd "$(dirname "$0")"
source utils.sh

cd ../..
project_dir="$(pwd)"
echo "Current directory: ${project_dir}"

packet_dir="${project_dir}/packaging/packet"
releases_dir="${project_dir}/packaging/releases"

sudo chmod -R +x ${packet_dir}
mv -vf "${packet_dir}/fonts" "${packet_dir}/Qt-App.app/Contents/MacOS/"

macdeployqt "${packet_dir}/Qt-App.app" -always-overwrite

ls -al "${packet_dir}/Qt-App.app/Contents/Frameworks"

# package with 7z
zip_path="${releases_dir}/Qt-App.7z"
7z a -t7z -r -mx=9 -mmt "${zip_path}" "${packet_dir}/Qt-App.app"

# package with pkg
version="0.1.1"
mkdir -p ${packet_dir}/output
# process_plist "${project_dir}/packaging/macos/distribution.xml"
sudo chmod -R +x ${project_dir}/packaging/macos/scripts
pkgbuild --root ${packet_dir}/Qt-App.app --identifier org.qt-app.client \
    --version ${version} \
    --scripts ${project_dir}/packaging/macos/scripts \
    --ownership recommended ${packet_dir}/output/tmp.pkg \
    --install-location /Applications/Qt-App.app
productbuild --distribution ${project_dir}/packaging/macos/distribution.xml \
    --resources resources --package-path ${packet_dir}/output \
    --version ${version} ${releases_dir}/Qt-App.pkg

# package with dmg
pip3 install dmgbuild

cd "${packet_dir}"
dmgbuild -s "${project_dir}/packaging/macos/dmgbuild.py" "Qt-App.app" "Qt-App.dmg"
mv -v "${packet_dir}/Qt-App.dmg" "${releases_dir}"

echo "Deployment macos completed."
