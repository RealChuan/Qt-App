#!/bin/bash -ex
# ============================================================
# Qt-App Ubuntu 打包入口
#
#   每个启用的架构产出 3 个文件：
#     Qt-App_<ver>_<date>_<suffix>.{AppImage,deb}
#     Qt-App_<ver>_<date>_linux_<suffix>.7z（7z 带 OS 名，避免与 macOS 同名）
#
#   默认只构建 x86_64。arm64 默认关闭，原因：
#     - ubuntu-latest runner 是 x86_64，arm64 需交叉工具链
#     - Qt for arm64 Linux 需单独安装
#     - linuxdeployqt 的 AppImage 打包不支持交叉
#   推荐启用方式：使用 GitHub Actions 的 ubuntu-24.04-arm runner，
#   原生 arm64 环境下把 enabled 改为 true 即可。
#
#   依赖（本机 / CI）：
#     cmake ninja linuxdeployqt 7z curl fakeroot dpkg-deb lintian
# ============================================================

app_name="Qt-App"
build_type="RelWithDebInfo"
build_date=$(date +%Y%m%d)

cd "$(dirname "$0")"
source utils.sh

cd ../..
project_dir="$(pwd)"
packet_root="${project_dir}/packaging/packet"
releases_dir="${project_dir}/packaging/releases"

# 版本号：唯一来源 packaging/version.txt
version_file="${project_dir}/packaging/version.txt"
[ -f "$version_file" ] || die "缺少版本文件: $version_file"
version="$(tr -d '[:space:]' <"$version_file")"
[ -n "$version" ] || die "版本文件为空: $version_file"

# ============================================================
# BuildList：架构配置中心（改这里即可增删架构 / 启停）
#
# 字段：name|release_suffix|deb_arch|enabled
#   name           : 架构名，用于日志与目录命名
#   release_suffix : 产物文件名后缀，与 upload.py 的 dst_tpl 对齐
#   deb_arch       : DEBIAN/control 的 Architecture 字段
#   enabled        : true / false
#
# arm64 默认关闭；在 arm64 原生 runner 上把 enabled 改 true 即可。
# ============================================================
BUILD_LIST=(
	"x86_64|x86_64|amd64|true"
	"arm64|aarch64|arm64|false"
)

[ ${#BUILD_LIST[@]} -gt 0 ] || die "BUILD_LIST 为空"
mkdir -p "$releases_dir"

# ============================================================
# 字体：只下载一次
# ============================================================
font_stage="${packet_root}/_fonts"
rm -rf "$font_stage"
download_fonts "$font_stage"

# ============================================================
# 单架构：configure → build → install → 复制资源 → 部署
#   $1 arch
# ============================================================
build_one_arch() {
	local arch="$1"
	local build_dir="${project_dir}/build/linux-${arch}"
	local inst_dir="${packet_root}/linux-${arch}"

	log_info "========== 构建 Ubuntu ${arch} =========="
	rm -rf "$build_dir" "$inst_dir"
	mkdir -p "$build_dir" "$inst_dir"

	# 1. configure
	cmake -S "$project_dir" -B "$build_dir" \
		-G "Ninja" \
		-DCMAKE_BUILD_TYPE="$build_type"

	# 2. build + install
	cmake --build "$build_dir" --config "$build_type"
	cmake --install "$build_dir" --prefix "$inst_dir"

	[ -n "$(ls -A "$inst_dir")" ] || die "cmake --install 后 packet 为空: $inst_dir"

	# 3. 复制 desktop / icon
	cp -vf "${project_dir}/packaging/ubuntu/${app_name}.desktop" "$inst_dir/"
	cp -vf "${project_dir}/src/resource/icon/app.png" "$inst_dir/"

	# 4. 字体合并
	local font_dst="${inst_dir}/resources/fonts"
	mkdir -p "$font_dst"
	cp -R "${font_stage}/"* "$font_dst/"

	# 5. 可执行权限
	chmod +x "${inst_dir}/${app_name}" \
		"${inst_dir}/crashpad_handler" 2>/dev/null || true
	chmod 644 "${inst_dir}"/*.desktop "${inst_dir}"/app.png

	# 6. linuxdeployqt（部署 Qt 依赖）
	#    不吞退出码：失败时记录日志，产物是否真的生成在 package_one_arch 里判定
	local ldqt_rc=0
	pushd "$inst_dir" >/dev/null
	linuxdeployqt "${inst_dir}/${app_name}" \
		-executable-dir="${inst_dir}/plugins/qt-app/" \
		-qmake=qmake \
		-always-overwrite \
		-unsupported-allow-new-glibc \
		-appimage || ldqt_rc=$?
	popd >/dev/null
	if [ "$ldqt_rc" -ne 0 ]; then
		log_warn "linuxdeployqt 退出码 ${ldqt_rc}，稍后检查 AppImage 是否生成"
	fi

	# 7. 共享库依赖校验：出现 not found 说明包不完整
	verify_no_missing_libs "$inst_dir"

	# 8. 打包内容只展示一次：AppImage / 7z / deb 都基于这份目录
	print_packet_tree "$inst_dir" 3

	log_info "✓ ${arch} 构建完成"
}

# ============================================================
# 单架构：打包（AppImage + 7z + deb）
#   $1 arch, $2 suffix, $3 deb_arch
# ============================================================
package_one_arch() {
	local arch="$1" suffix="$2" deb_arch="$3"
	local inst_dir="${packet_root}/linux-${arch}"
	[ -d "$inst_dir" ] || die "打包时找不到 packet: $inst_dir"

	log_info "========== 打包 Ubuntu ${arch} =========="

	# --- AppImage ---
	local appimage_src
	appimage_src=$(find "$inst_dir" -maxdepth 1 -name "*.AppImage" | head -1)
	if [ -n "$appimage_src" ]; then
		chmod +x "$appimage_src"
		mv -v "$appimage_src" \
			"${releases_dir}/${app_name}_${version}_${build_date}_${suffix}.AppImage"
		rm -f "$inst_dir/.DirIcon" "$inst_dir/AppRun"
	else
		log_error "未生成 AppImage: ${inst_dir}（linuxdeployqt 失败）"
		appimage_failed=$((appimage_failed + 1))
	fi

	# --- 7z（第一个归档格式，必须在 flat 状态下打包）---
	#   linuxdeployqt 在构建阶段已把 AppImage 写进 inst_dir，必须先把它移出
	#   并清掉 AppRun/.DirIcon，否则归档里会嵌套 AppImage 本体。
	7z a -t7z -mx=9 -mmt -snl \
		"${releases_dir}/${app_name}_${version}_${build_date}_ubuntu_${suffix}.7z" \
		"${inst_dir}"/*

	# --- deb ---
	# 先按 deb 布局整理：opt/Qt-App/{...} + DEBIAN/ + usr/
	mkdir -p "${inst_dir}/opt"
	local payload_dir="${inst_dir}/opt/${app_name}"
	mkdir -p "$payload_dir"

	# 把当前除 opt/DEBIAN/usr 以外的所有文件移入 opt/Qt-App/
	find "$inst_dir" -maxdepth 1 -mindepth 1 \
		! -name opt ! -name DEBIAN ! -name usr \
		-exec mv -v {} "$payload_dir"/ \;

	cp -rv "${project_dir}/packaging/ubuntu/DEBIAN" "$inst_dir/"
	cp -rv "${project_dir}/packaging/ubuntu/usr" "$inst_dir/"
	cp -v "${project_dir}/packaging/ubuntu/${app_name}.sh" "$payload_dir/"

	# 图标按 hicolor 规范安装，desktop 里用 Icon=qt-app 查找
	install -Dm644 "${project_dir}/src/resource/icon/app.png" \
		"$inst_dir/usr/share/icons/hicolor/1024x1024/apps/qt-app.png"

	# DEBIAN/control 的 Architecture / Version 按本次构建替换
	sed -i -e "s/^Architecture:.*/Architecture: ${deb_arch}/" \
		-e "s/^Version:.*/Version: ${version}/" "$inst_dir/DEBIAN/control"

	# control 保持 644，只有 maintainer scripts 需要可执行
	chmod 644 "$inst_dir/DEBIAN/control"
	chmod 755 "$inst_dir/DEBIAN/postinst" "$inst_dir/DEBIAN/postrm" \
		"$inst_dir/DEBIAN/preinst" "$inst_dir/DEBIAN/prerm"
	chmod +x "$payload_dir"/*.sh
	chmod 644 "$inst_dir/usr/share/applications/"*.desktop 2>/dev/null || true

	local deb_path="${releases_dir}/${app_name}_${version}_${build_date}_${suffix}.deb"
	fakeroot dpkg-deb -b "$inst_dir" "$deb_path"

	chmod +x "$deb_path"
	lintian -i "$deb_path" || true

	log_info "✓ ${arch} 打包完成"
}

# ============================================================
# 主流程
# ============================================================
main() {
	# ---------- 依赖检查 ----------
	for t in cmake ninja 7z curl fakeroot dpkg-deb linuxdeployqt file; do
		check_tool "$t"
	done
	command -v lintian >/dev/null 2>&1 || log_warn "lintian 未安装，跳过 deb 检查"

	local appimage_failed=0

	# ---------- 逐架构 ----------
	local entry arch suffix deb_arch enabled
	for entry in "${BUILD_LIST[@]}"; do
		IFS='|' read -r arch suffix deb_arch enabled <<< "$entry"

		if [ "$enabled" != "true" ]; then
			log_info "跳过架构: $arch (enabled=$enabled)"
			continue
		fi

		log_info "架构: arch=$arch suffix=$suffix deb_arch=$deb_arch"

		build_one_arch   "$arch"
		package_one_arch "$arch" "$suffix" "$deb_arch"
	done

	if [ "$appimage_failed" -ne 0 ]; then
		die "有架构未生成 AppImage，打包结果不完整"
	fi

	log_info "========== 全部完成 =========="
}

main "$@"
