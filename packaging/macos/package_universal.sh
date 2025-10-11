#!/bin/bash -ex

app_name="Qt-App"

cd "$(dirname "$0")"
source utils.sh

cd ../..
project_dir="$(pwd)"
echo "Current directory: ${project_dir}"

packet_dir="${project_dir}/packaging/packet"
releases_dir="${project_dir}/packaging/releases"

cd ${packet_dir}
ls -al

cd macos*x86*
7z x *.7z
ls -al
cd ..

cd macos*arm64*
7z x *.7z
ls -al
cd ..

cd ${project_dir}
chmod -R +x ${project_dir}/packaging/macos/app_arch_tools

${project_dir}/packaging/macos/app_arch_tools/merge_universal_app.sh \
	-x ${packet_dir}/macos*x86*/${app_name}.app \
	-a ${packet_dir}/macos*arm64*/${app_name}.app \
	-o ${packet_dir}/${app_name}.app

${project_dir}/packaging/macos/app_arch_tools/verify_app_architecture.sh \
	${packet_dir}/${app_name}.app \
	-a universal -v -s

tree "${packet_dir}/${app_name}.app"

# package with 7z
zip_path="${releases_dir}/${app_name}.7z"
7z a -t7z -mx=9 -mmt "${zip_path}" "${packet_dir}/${app_name}.app"

# package with pkg
version="0.2.1"
mkdir -p ${packet_dir}/output
# process_plist "${project_dir}/packaging/macos/distribution.xml"
chmod -R +x ${project_dir}/packaging/macos/scripts
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

cd ${packet_dir}
create-dmg "${app_name}.app" "${releases_dir}" \
	--overwrite --no-version-in-filename --no-code-sign

echo "Deployment macos completed."
