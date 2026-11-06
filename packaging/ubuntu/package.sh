#!/bin/bash -ex
# ============================================================
# Qt-App Ubuntu 打包入口
#
#   每个架构产出 3 个文件：
#     Qt-App_<ver>_<date>_<suffix>.{AppImage,deb}
#     Qt-App_<ver>_<date>_linux_<suffix>.7z（7z 带 OS 名，避免与 macOS 同名）
#
#   x86_64 与 arm64 都支持，但 Ubuntu 侧不做交叉编译：
#     - x86_64 在 ubuntu-latest 上原生构建
#     - arm64   在 ubuntu-26.04-arm 上原生构建
#   因此每个 runner 只会构建与自身架构一致的那一条，BUILD_LIST 里两条都
#   enabled=true，实际由宿主机架构过滤（见 main 的 host_arch）。在一台 x64
#   机器上跑，arm64 会被跳过而不是失败。
#
#   依赖（本机 / CI）：
#     cmake ninja appimagetool 7z curl fakeroot dpkg-deb lintian
#   linuxdeploy 部署依赖并捆绑 Qt，appimagetool 负责封包（原因见 utils.sh 顶部）。
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

# 字段：name|release_suffix|deb_arch|enabled
#   enabled=false 表示不发这个架构；能不能建由宿主机架构决定（不交叉编译）
BUILD_LIST=(
	"x86_64|x86_64|amd64|true"
	"arm64|aarch64|arm64|true"
)

[ ${#BUILD_LIST[@]} -gt 0 ] || die "BUILD_LIST 为空"
mkdir -p "$releases_dir"

# Ubuntu 侧不交叉编译：Qt 的 arm64 包、vcpkg 的 arm64-linux triplet 都只能在原生
# arm64 上用，所以只构建与宿主机一致的架构。
host_arch_raw="$(uname -m)"
case "$host_arch_raw" in
x86_64 | amd64) host_arch="x86_64" ;;
aarch64 | arm64) host_arch="arm64" ;;
*)
	log_warn "无法识别的宿主机架构: ${host_arch_raw}，将跳过所有架构"
	host_arch="unknown"
	;;
esac
log_info "宿主机架构: ${host_arch_raw} -> ${host_arch}"

font_stage="${packet_root}/_fonts"
rm -rf "$font_stage"
download_fonts "$font_stage"

# ============================================================
# 单架构：configure → build → install 到 AppDir → 部署 → 封包
#   产物目录就是标准 AppDir，AppImage / deb / 7z 三者同源。
#   $1 arch
# ============================================================
build_one_arch() {
	local arch="$1"
	local build_dir="${project_dir}/build/linux-${arch}"
	local appdir="${packet_root}/linux-${arch}"
	local bin_dir

	log_info "========== 构建 Ubuntu ${arch} =========="
	rm -rf "$build_dir" "$appdir"
	mkdir -p "$build_dir"

	# 1. configure
	log_info "cmake configure: build_dir=${build_dir}"
	cmake -S "$project_dir" -B "$build_dir" \
		-G "Ninja" \
		-DCMAKE_BUILD_TYPE="$build_type"

	log_info "$(grep -E '^TARGET_ARCH:' "${build_dir}/CMakeCache.txt" 2>/dev/null || echo 'TARGET_ARCH 未知')"

	# 2. build
	cmake --build "$build_dir" --config "$build_type"

	# 3. install：Linux 的安装规则本身就是 FHS 布局，装出来就是 AppDir
	cmake --install "$build_dir" --prefix "$appdir"

	local exe_path
	exe_path=$(find "$appdir" -type f -name "${app_name}" -print -quit)
	[ -n "$exe_path" ] || die "cmake --install 后找不到可执行文件 ${app_name}"
	bin_dir=$(dirname "$exe_path")
	log_info "可执行文件目录: ${bin_dir}"

	# 4. AppDir 叠加层：desktop 与图标
	#    packaging/ubuntu/appdir/ 与 AppDir 根同构，直接叠上去；
	#    图标供 linuxdeploy 按 desktop 里的 Icon=app 挑用。
	cp -R "${project_dir}/packaging/ubuntu/appdir/." "$appdir/"
	install -Dm644 "${project_dir}/src/resource/icon/app.png" \
		"${appdir}/usr/share/icons/hicolor/256x256/apps/app.png"

	# 5. 字体：填进 cmake --install 建好的资源目录
	local app_lower resource_dir
	app_lower=$(printf '%s' "$app_name" | tr '[:upper:]' '[:lower:]')
	resource_dir="${appdir}/usr/share/${app_lower}/resources"
	[ -d "$resource_dir" ] ||
		die "安装树里没有 ${resource_dir}，RESOURCE_INSTALL_DIR 与打包脚本不一致"
	local font_dst="${resource_dir}/fonts"
	mkdir -p "$font_dst"
	cp -R "${font_stage}/"* "$font_dst/"

	# 6. 可执行权限
	chmod +x "${bin_dir}/${app_name}" "${bin_dir}/crashpad_handler" 2>/dev/null || true
	chmod 644 "${appdir}/usr/share/applications/${app_name}.desktop"

	# 7. Qt 依赖部署 + AppImage 封包
	deploy_appimage "$appdir" "$app_name" "$version" true

	# 8. 共享库依赖校验：出现 not found 说明包不完整
	verify_no_missing_libs "$appdir"

	# 9. 打包内容只展示一次：AppImage / 7z / deb 都基于这份目录
	print_packet_tree "$appdir" 2

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
		# AppRun / .DirIcon / *.zsync 只属于 AppImage，不能进 7z 和 deb
		clean_appdir_for_packaging "$inst_dir" "$app_name"
	else
		log_error "未生成 AppImage: ${inst_dir}（appimagetool 封包失败）"
		echo "---- ${inst_dir} 根目录内容 ----"
		ls -la "$inst_dir" || true
		appimage_failed=$((appimage_failed + 1))
	fi

	# --- 7z（第一个归档格式，必须在 flat 状态下打包）---
	#   appimagetool 在构建阶段已把 AppImage 写进 inst_dir，必须先把它移出
	#   并清掉 AppRun/.DirIcon，否则归档里会嵌套 AppImage 本体。
	7z a -t7z -mx=9 -mmt -snl \
		"${releases_dir}/${app_name}_${version}_${build_date}_ubuntu_${suffix}.7z" \
		"${inst_dir}"/*

	# --- deb ---
	# 先按 deb 布局整理：opt/Qt-App/{usr/...,app.png} + DEBIAN/ + usr/
	#
	# AppDir 自带的 usr/ 属于应用本体，必须整体挪进 opt/Qt-App/；
	# 留在顶层会被当成 deb 的系统 /usr，把 Qt-App 装到系统 /usr/bin 下。
	mkdir -p "${inst_dir}/opt"
	local payload_dir="${inst_dir}/opt/${app_name}"
	mkdir -p "$payload_dir"

	find "$inst_dir" -maxdepth 1 -mindepth 1 \
		! -name opt ! -name DEBIAN \
		-exec mv -v {} "$payload_dir"/ \;

	# packaging/ubuntu/deb/ 与 deb 根同构：DEBIAN/ + 系统级 usr/
	cp -rv "${project_dir}/packaging/ubuntu/deb/DEBIAN" "$inst_dir/"
	cp -rv "${project_dir}/packaging/ubuntu/deb/usr" "$inst_dir/"
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
	for t in cmake ninja 7z curl fakeroot dpkg-deb linuxdeploy linuxdeploy-plugin-qt appimagetool file; do
		check_tool "$t"
	done
	command -v lintian >/dev/null 2>&1 || log_warn "lintian 未安装，跳过 deb 检查"

	log_info "version=${version} build_type=${build_type} host_arch=${host_arch} QTDIR=${QTDIR:-<unset>}"

	local appimage_failed=0
	local built=0

	# ---------- 逐架构 ----------
	local entry arch suffix deb_arch enabled
	for entry in "${BUILD_LIST[@]}"; do
		IFS='|' read -r arch suffix deb_arch enabled <<< "$entry"

		if [ "$enabled" != "true" ]; then
			log_info "跳过架构: $arch (enabled=$enabled)"
			continue
		fi

		# 不做交叉编译：只构建与宿主机一致的架构
		if [ "$arch" != "$host_arch" ]; then
			log_info "跳过架构: $arch（宿主机是 ${host_arch}，Ubuntu 侧不做交叉编译）"
			continue
		fi

		log_info "架构: arch=$arch suffix=$suffix deb_arch=$deb_arch"

		build_one_arch   "$arch"
		package_one_arch "$arch" "$suffix" "$deb_arch"
		built=$((built + 1))
	done

	# 一台机器都没匹配上说明 BUILD_LIST 与 runner 架构对不上，直接失败，
	# 否则会出现「CI 绿了但一个产物都没有」
	[ "$built" -gt 0 ] || die "没有任何架构被构建（host_arch=${host_arch}），请检查 BUILD_LIST"

	if [ "$appimage_failed" -ne 0 ]; then
		die "有架构未生成 AppImage，打包结果不完整"
	fi

	log_info "========== 全部完成（${built} 个架构）=========="
}

main "$@"
