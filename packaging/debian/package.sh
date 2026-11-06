#!/bin/bash -ex
# ============================================================
# deb payload 组装脚本（由 debian/rules 的 override_dh_auto_install 调用）
#
#   输入：debhelper 已完成的 CMake 构建目录（cmake --install 到 packet）
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
log_info "Host: $(uname -a)"

# 版本号：唯一来源 packaging/version.txt（deploy 封包时会用到，这里不产 AppImage，
# 但仍按同一份版本走，避免两处口径不一致）
version="$(tr -d '[:space:]' <"${project_dir}/packaging/version.txt")"
[ -n "$version" ] || die "packaging/version.txt 为空"
log_info "Version: ${version}"

for t in curl 7z file linuxdeploy linuxdeploy-plugin-qt appimagetool; do
	check_tool "$t"
done

# 与 ubuntu 版共用同一套安装规则；debhelper 已构建完，这里只需找到它的构建目录。
mapfile -t _cmake_caches < <(find "$project_dir" -maxdepth 3 -name CMakeCache.txt \
	-not -path "${project_dir}/binaries/*" -not -path "*/.git/*" 2>/dev/null)
if [ ${#_cmake_caches[@]} -ne 1 ]; then
	log_error "期望恰好一个 CMakeCache.txt，实际 ${#_cmake_caches[@]} 个"
	printf '  %s\n' "${_cmake_caches[@]}" >&2 || true
	die "无法确定 debhelper 的 CMake 构建目录"
fi
build_dir="$(dirname "${_cmake_caches[0]}")"
log_info "CMake build dir: ${build_dir}"

rm -rf "$packet_dir"
mkdir -p "$packet_dir"
cmake --install "$build_dir" --prefix "$packet_dir"

# 可执行文件的位置由安装规则决定
exe_path="$(find "$packet_dir" -type f -name "${app_name}" -print -quit)"
[ -n "$exe_path" ] || die "cmake --install 后找不到可执行文件 ${app_name}"
usr_bin="$(dirname "$exe_path")"
log_info "可执行文件目录: ${usr_bin}"

# packaging/ubuntu/appdir/ 与 AppDir 根同构：叠加 desktop
cp -R "${project_dir}/packaging/ubuntu/appdir/." "$packet_dir/"

# 启动脚本留在 /opt/Qt-App 本身
cp -vf "${project_dir}/packaging/ubuntu/${app_name}.sh" "$packet_dir/"
install -Dm644 "${project_dir}/src/resource/icon/app.png" \
	"${packet_dir}/usr/share/icons/hicolor/256x256/apps/app.png"

chmod +x "${usr_bin}/${app_name}" "${usr_bin}/crashpad_handler" 2>/dev/null || true
chmod 644 "$packet_dir/usr/share/applications/${app_name}.desktop"
chmod +x "$packet_dir"/*.sh

# 字体：与 ubuntu 产物共用 packaging/fonts.url；已有内容就跳过
app_lower=$(printf '%s' "$app_name" | tr '[:upper:]' '[:lower:]')
resource_dir="${packet_dir}/usr/share/${app_lower}/resources"
[ -d "$resource_dir" ] ||
	die "安装树里没有 ${resource_dir}，RESOURCE_INSTALL_DIR 与打包脚本不一致"
if [ -z "$(ls -A "${resource_dir}/fonts" 2>/dev/null)" ]; then
	download_fonts "${resource_dir}/fonts"
fi

# Qt 依赖部署（deb 只需要部署后的目录，不生成 AppImage）
deploy_appimage "$packet_dir" "$app_name" "$version" false
# AppRun / .DirIcon / *.zsync / 根上的 desktop 副本只服务于 AppImage
clean_appdir_for_packaging "$packet_dir" "$app_name"

# 共享库依赖校验：出现 not found 直接失败
verify_no_missing_libs "$packet_dir"

# 打包内容只展示一次：debhelper 后续基于这份目录生成 .deb
print_packet_tree "$packet_dir" 2

log_info "✓ packet 组装完成: ${packet_dir}"
