#!/bin/bash -ex

app_name="Qt-App"

cd "$(dirname "$0")"
source utils.sh

cd ../..
project_dir="$(pwd)"
echo "Current directory: ${project_dir}"

packet_dir="${project_dir}/packaging/packet"
releases_dir="${project_dir}/packaging/releases"

sudo chmod -R +x ${packet_dir}
mv -vf "${packet_dir}/fonts" "${packet_dir}/${app_name}.app/Contents/Resources/"

macdeployqt "${packet_dir}/${app_name}.app" -always-overwrite

ls -al "${packet_dir}/${app_name}.app/Contents/Frameworks"

# package with 7z
zip_path="${releases_dir}/${app_name}.7z"
7z a -t7z -r -mx=9 -mmt "${zip_path}" "${packet_dir}/${app_name}.app"

# package with pkg
version="0.1.1"
mkdir -p ${packet_dir}/output
# process_plist "${project_dir}/packaging/macos/distribution.xml"
sudo chmod -R +x ${project_dir}/packaging/macos/scripts
pkgbuild --root ${packet_dir}/${app_name}.app --identifier org.qt-app.client \
	--version ${version} \
	--scripts ${project_dir}/packaging/macos/scripts \
	--ownership recommended ${packet_dir}/output/tmp.pkg \
	--install-location /Applications/${app_name}.app
productbuild --distribution ${project_dir}/packaging/macos/distribution.xml \
	--resources resources --package-path ${packet_dir}/output \
	--version ${version} ${releases_dir}/${app_name}.pkg

# package with dmg
brew install node graphicsmagick imagemagick
npm install -g create-dmg

cd "${packet_dir}"
create-dmg "${app_name}.app" "${releases_dir}" \
	--overwrite --no-version-in-filename --no-code-sign

echo "Deployment macos completed."
