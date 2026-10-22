#!/bin/bash -ex
# ============================================================
# deb payload 组装脚本（由 debian/rules 的 override_dh_auto_install 调用）
#
#   输入：CMake 产物 binaries/cmake/<arch>/bin/<config>/
#   输出：packaging/packet/deb/ —— 完整可运行目录（主程序 + Qt 库 + 字体）
#
#   与 packaging/ubuntu/package.sh 的分工：
#     - ubuntu 版：自己 configure/build，并产出 AppImage / 7z / 手搓 deb
#     - 本脚本：只准备 debhelper 流程需要的目录，不产出 AppImage / 7z
#   公共逻辑（日志、字体下载、共享库校验）直接复用 packaging/ubuntu/utils.sh
# ============================================================

set -euo pipefail

# 先解析符号链接再定位目录：
# CI 里 debian/ 是 ./debian -> packaging/debian 的符号链接，不能靠 cd .. 猜层级
script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd -P)"
source "${script_dir}/../ubuntu/utils.sh"

app_name="Qt-App"
project_dir="$(cd "${script_dir}/../.." && pwd -P)"
packet_dir="${project_dir}/packaging/packet/deb"

log_info "Project directory: ${project_dir}"

for t in curl 7z file linuxdeployqt; do
	check_tool "$t"
done

# 只允许单一架构产物，避免 x64 / arm64 混进同一个包
# 只扫 binaries/cmake/ 下的产物（binaries/qmake/ 是 qmake 链的输出，不参与打包）
shopt -s nullglob
src_dirs=("${project_dir}"/binaries/cmake/*/bin/*/)
[ ${#src_dirs[@]} -eq 1 ] || die "binaries/cmake 下应恰好有一个 <arch>/bin/<config>/ 目录，实际 ${#src_dirs[@]} 个"
src_dir="${src_dirs[0]}"

rm -rf "$packet_dir"
mkdir -p "$packet_dir"
cp -avf "${src_dir}"* "$packet_dir"/

# 与 ubuntu 版保持一致：提权启动脚本 + 图标
cp -vf "${project_dir}/packaging/ubuntu/${app_name}.sh" "$packet_dir/"
cp -vf "${project_dir}/src/resource/icon/app.png" "$packet_dir"/

chmod +x "${packet_dir}/${app_name}" \
	"${packet_dir}/crashpad_handler" 2>/dev/null || true
chmod 644 "$packet_dir"/app.png
chmod +x "$packet_dir"/*.sh

# 字体：与 ubuntu 产物共用 packaging/fonts.url；已有内容就跳过
if [ -z "$(ls -A "$packet_dir/resources/fonts" 2>/dev/null)" ]; then
	download_fonts "$packet_dir/resources/fonts"
fi

# Qt 依赖部署（deb 只需要部署后的目录，不生成 AppImage）
pushd "$packet_dir" >/dev/null
linuxdeployqt "${packet_dir}/${app_name}" \
	-executable-dir="${packet_dir}/plugins/qt-app/" \
	-qmake=qmake \
	-always-overwrite \
	-unsupported-allow-new-glibc || die "linuxdeployqt 失败"
popd >/dev/null

# 共享库依赖校验：出现 not found 直接失败
verify_no_missing_libs "$packet_dir"

# 打包内容只展示一次：debhelper 后续基于这份目录生成 .deb
print_packet_tree "$packet_dir" 3

log_info "✓ packet 组装完成: ${packet_dir}"
