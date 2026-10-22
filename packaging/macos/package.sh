#!/bin/bash -ex
# ============================================================
# Qt-App macOS 打包入口
#
#   单机完成 x86_64 + arm64 + universal，共 9 个产物：
#     Qt-App_<ver>_<date>_{x86_64|aarch64|universal}.{dmg,pkg}
#     Qt-App_<ver>_<date>_macos_{x86_64|aarch64|universal}.7z（带 OS 名，避免与 Linux 同名）
#
#   签名 / 公证：默认关闭，启用方法见 utils.sh 顶部注释
#
#   依赖：
#     cmake ninja macdeployqt 7z curl create-dmg pkgbuild productbuild
#
#   本机可直接运行：./packaging/macos/package.sh
#   GitHub Actions 通过 cmake.yml 调用同一个脚本
# ============================================================

app_name="Qt-App"

cd "$(dirname "$0")"
source utils.sh

cd ../..
project_dir="$(pwd)"
packet_dir="${project_dir}/packaging/packet"
releases_dir="${project_dir}/packaging/releases"

# 版本号：唯一来源 packaging/version.txt
version_file="${project_dir}/packaging/version.txt"
[ -f "$version_file" ] || die "缺少版本文件: $version_file"
version="$(tr -d '[:space:]' <"$version_file")"
[ -n "$version" ] || die "版本文件为空: $version_file"
build_date=$(date +%Y%m%d)
build_type="RelWithDebInfo"

# ============================================================
# BuildList：架构配置中心（改这里即可增删架构 / 启停）
#
# 字段：name|cmake_arch|release_suffix|enabled
#
# 为什么不直接编 universal：vcpkg 没有 universal triplet，依赖只能按单架构编译。
# 因此每个架构单独 configure/build，再用 app_arch_tools 把 universal 的 Qt 库
# 瘦身成单架构，最后由 merge_universal_app.sh 合成 universal。
# ============================================================
BUILD_LIST=(
	"x86_64|x86_64|x86_64|true"
	"arm64|arm64|aarch64|true"
)
BUILD_UNIVERSAL=true
BUILD_PKG=true

[ ${#BUILD_LIST[@]} -gt 0 ] || die "BUILD_LIST 为空"
mkdir -p "$releases_dir"

# ============================================================
# 生成 distribution.xml 临时副本
#   $1 host_arch : x86_64 | arm64 | x86_64,arm64
#   $2 output    : 输出文件路径
#   安装架构与版本号都由这里注入，模板里不保留可用的硬编码值
# ============================================================
render_distribution() {
	local host_arch="$1" output="$2"
	sed -e "s/hostArchitectures=\"[^\"]*\"/hostArchitectures=\"${host_arch}\"/" \
		-e "/<pkg-ref id=\"org.qt-app.client\"/,/<\/pkg-ref>/ s/version=\"[^\"]*\"/version=\"${version}\"/" \
		"${project_dir}/packaging/macos/distribution.xml" >"$output"
}

# ============================================================
# 7z 便携包：在源目录里打包，归档内只保留相对路径
#   $1 源目录  $2 归档内的顶层条目  $3 输出文件
# ============================================================
make_7z() {
	local src_dir="$1" entry="$2" output="$3"
	log "生成 7z: $output"
	(
		cd "$src_dir"
		# -snl：符号链接按链接存储。.app 里的 framework 依赖 Versions/Current 这类链接，
		# 不加会把链接变成实体文件，包体积暴涨且签名会失效。
		7z a -t7z -mx=9 -mmt -snl "$output" "$entry"
	)
}

# ============================================================
# 单架构：编译 + macdeployqt + 分离纯单架构 + 验证 + 签名
#   $1 arch          : 架构名（x86_64 / arm64）
#   $2 cmake_arch    : CMAKE_OSX_ARCHITECTURES
# ============================================================
build_one_arch() {
	local arch="$1" cmake_arch="$2"
	local build_dir="${project_dir}/build/macos-${arch}"
	local inst_dir="${packet_dir}/macos-${arch}"

	log "========== 构建 macOS ${arch} =========="
	rm -rf "$build_dir" "$inst_dir"
	mkdir -p "$build_dir" "$inst_dir"

	# 1. configure
	cmake -S "$project_dir" -B "$build_dir" \
		-G "Ninja" \
		-DCMAKE_BUILD_TYPE="$build_type" \
		-DCMAKE_OSX_ARCHITECTURES="$cmake_arch"

	# 2. build + install
	cmake --build "$build_dir" --config "$build_type"
	cmake --install "$build_dir" --prefix "$inst_dir"

	local app_path="${inst_dir}/${app_name}.app"
	[ -d "$app_path" ] || die "cmake --install 后找不到 .app: $app_path"

	# 3. macdeployqt
	macdeployqt "$app_path" -always-overwrite

	# macdeployqt 会把 PlugIns 下的插件当成普通 dylib 依赖，再拷一份到 Frameworks。
	# 插件只在运行期被 QPluginLoader dlopen，没有任何二进制 link 它们，这份拷贝
	# 纯属冗余（同名同内容），留着会让后面的 universal 合并与签名多做一遍工。
	# 判定依据是「PlugIns 里有同名文件」，与后缀无关（.dylib / .so 都覆盖），
	# 也不会误删真正的第三方 dylib。
	local app_plugins_dir
	app_plugins_dir="${app_path}/Contents/PlugIns/$(printf '%s' "$app_name" | tr '[:upper:]' '[:lower:]')"
	if [ -d "$app_plugins_dir" ]; then
		local dup name
		for dup in "$app_path/Contents/Frameworks"/*; do
			[ -f "$dup" ] || continue
			name=$(basename "$dup")
			[ -f "$app_plugins_dir/$name" ] || continue
			log "删除 Frameworks 下重复的插件拷贝: $name"
			rm -f "$dup"
		done
	fi

	# 4. 分离纯单架构（macdeployqt 拷进的 Qt 库通常是 universal）
	local tmp_app="${inst_dir}/${app_name}_tmp.app"
	rm -rf "$tmp_app"
	chmod -R +x "${project_dir}/packaging/macos/app_arch_tools"
	"${project_dir}/packaging/macos/app_arch_tools/separate_universal_app.sh" \
		-i "$app_path" -o "$tmp_app"
	rm -rf "$app_path"
	mv "$tmp_app" "$app_path"

	# 5. 严格验证单架构
	"${project_dir}/packaging/macos/app_arch_tools/verify_app_architecture.sh" \
		"$app_path" -a "$arch" -v -s

	# 6. 签名 .app —— 当前未启用，启用时取消下面这行注释
	# sign_app "$app_path"

	log "✓ ${arch} 构建完成"
}

# ============================================================
# 单架构：打包（dmg + 可选 pkg）
#   $1 arch   : 架构名
#   $2 suffix : 产物文件名后缀
# ============================================================
package_one_arch() {
	local arch="$1" suffix="$2"
	local inst_dir="${packet_dir}/macos-${arch}"
	local app_path="${inst_dir}/${app_name}.app"
	[ -d "$app_path" ] || die "打包时找不到 .app: $app_path"

	log "========== 打包 macOS ${arch} =========="

	# --- 7z（排在最前：归档的是刚构建好、未被 dmg/pkg 工具动过的 .app）---
	make_7z "$inst_dir" "${app_name}.app" \
		"${releases_dir}/${app_name}_${version}_${build_date}_macos_${suffix}.7z"

	# --- dmg ---
	local tmp_dmg_dir
	tmp_dmg_dir=$(mktemp -d -t qtaapp_dmg_XXXXXX)
	(
		cd "$inst_dir"
		create-dmg "${app_name}.app" "$tmp_dmg_dir" \
			--overwrite --no-version-in-filename --no-code-sign
	)
	local dmg_path="${releases_dir}/${app_name}_${version}_${build_date}_${suffix}.dmg"
	mv "${tmp_dmg_dir}/${app_name}.dmg" "$dmg_path"
	rm -rf "$tmp_dmg_dir"

	# 签名 + 公证 dmg —— 当前未启用，启用时取消下面两行注释
	# sign_dmg     "$dmg_path"
	# notarize_app "$dmg_path"

	# --- pkg ---
	if [ "$BUILD_PKG" = "true" ]; then
		local tmp_pkg_dir
		tmp_pkg_dir=$(mktemp -d -t qtaapp_pkg_XXXXXX)
		render_distribution "$arch" "${tmp_pkg_dir}/distribution.xml"

		chmod -R +x "${project_dir}/packaging/macos/scripts"
		pkgbuild --root "$app_path" --identifier "org.qt-app.client" \
			--version "$version" \
			--scripts "${project_dir}/packaging/macos/scripts" \
			--ownership recommended "${tmp_pkg_dir}/tmp.pkg" \
			--install-location "/Applications/${app_name}.app"
		productbuild --distribution "${tmp_pkg_dir}/distribution.xml" \
			--resources "${project_dir}/packaging/macos/resources" \
			--package-path "$tmp_pkg_dir" \
			--version "$version" "${tmp_pkg_dir}/${app_name}.pkg"

		local pkg_path="${releases_dir}/${app_name}_${version}_${build_date}_${suffix}.pkg"
		mv "${tmp_pkg_dir}/${app_name}.pkg" "$pkg_path"
		rm -rf "$tmp_pkg_dir"

		# 签名 + 公证 pkg —— 当前未启用，启用时取消下面两行注释
		# sign_pkg     "$pkg_path"
		# notarize_app "$pkg_path"
	fi

	log "✓ ${arch} 打包完成"
}

# ============================================================
# Universal：合并 + 打包
# ============================================================
build_and_package_universal() {
	log "========== 合并 Universal =========="

	local x86_app="${packet_dir}/macos-x86_64/${app_name}.app"
	local arm_app="${packet_dir}/macos-arm64/${app_name}.app"
	local uni_dir="${packet_dir}/macos-universal"
	local uni_app="${uni_dir}/${app_name}.app"

	# 若某单架构被 enabled=false 跳过，则 universal 自动忽略，不报错
	if [ ! -d "$x86_app" ] || [ ! -d "$arm_app" ]; then
		log "⚠ 缺少单架构产物（x86_64 或 arm64），跳过 universal"
		return 0
	fi

	rm -rf "$uni_dir"
	mkdir -p "$uni_dir"

	# 1. 合并
	"${project_dir}/packaging/macos/app_arch_tools/merge_universal_app.sh" \
		-x "$x86_app" -a "$arm_app" -o "$uni_app"

	# 2. 验证
	"${project_dir}/packaging/macos/app_arch_tools/verify_app_architecture.sh" \
		"$uni_app" -a universal -v -s

	# 3. 打包内容只展示一次：7z / dmg / pkg 都基于这份 .app
	print_packet_tree "$uni_dir" 5

	# 4. 签名 universal .app —— 当前未启用
	# sign_app "$uni_app"

	# 5. 7z（排在最前：归档的是刚合并好、未被 dmg/pkg 工具动过的 .app）
	make_7z "$uni_dir" "${app_name}.app" \
		"${releases_dir}/${app_name}_${version}_${build_date}_macos_universal.7z"

	# 6. dmg
	local tmp_dmg_dir
	tmp_dmg_dir=$(mktemp -d -t qtaapp_uni_dmg_XXXXXX)
	(
		cd "$uni_dir"
		create-dmg "${app_name}.app" "$tmp_dmg_dir" \
			--overwrite --no-version-in-filename --no-code-sign
	)
	local dmg_path="${releases_dir}/${app_name}_${version}_${build_date}_universal.dmg"
	mv "${tmp_dmg_dir}/${app_name}.dmg" "$dmg_path"
	rm -rf "$tmp_dmg_dir"

	# 签名 + 公证 dmg —— 当前未启用
	# sign_dmg     "$dmg_path"
	# notarize_app "$dmg_path"

	# 7. pkg
	if [ "$BUILD_PKG" = "true" ]; then
		local tmp_pkg_dir
		tmp_pkg_dir=$(mktemp -d -t qtaapp_uni_pkg_XXXXXX)
		render_distribution "x86_64,arm64" "${tmp_pkg_dir}/distribution.xml"

		chmod -R +x "${project_dir}/packaging/macos/scripts"
		pkgbuild --root "$uni_app" --identifier "org.qt-app.client" \
			--version "$version" \
			--scripts "${project_dir}/packaging/macos/scripts" \
			--ownership recommended "${tmp_pkg_dir}/tmp.pkg" \
			--install-location "/Applications/${app_name}.app"
		productbuild --distribution "${tmp_pkg_dir}/distribution.xml" \
			--resources "${project_dir}/packaging/macos/resources" \
			--package-path "$tmp_pkg_dir" \
			--version "$version" "${tmp_pkg_dir}/${app_name}.pkg"

		local pkg_path="${releases_dir}/${app_name}_${version}_${build_date}_universal.pkg"
		mv "${tmp_pkg_dir}/${app_name}.pkg" "$pkg_path"
		rm -rf "$tmp_pkg_dir"

		# 签名 + 公证 pkg —— 当前未启用
		# sign_pkg     "$pkg_path"
		# notarize_app "$pkg_path"
	fi

	log "✓ universal 打包完成"
}

# ============================================================
# 主流程
# ============================================================
main() {
	# ---------- 依赖检查 / 安装 ----------
	if ! command -v create-dmg >/dev/null 2>&1; then
		log "安装 create-dmg 及其依赖..."
		brew install node graphicsmagick imagemagick
		npm install -g create-dmg
	fi
	for t in cmake ninja macdeployqt 7z curl create-dmg; do
		check_tool "$t"
	done
	if [ "$BUILD_PKG" = "true" ]; then
		check_tool pkgbuild
		check_tool productbuild
	fi
	# 签名启用时额外检查
	if [ -n "$DEVELOPER_ID_APPLICATION" ] || [ -n "$DEVELOPER_ID_INSTALLER" ]; then
		check_tool codesign
		check_tool productsign
	fi

	# ---------- 字体（一次，各架构共享） ----------
	local font_stage="${packet_dir}/_fonts"
	rm -rf "$font_stage"
	download_fonts "$font_stage"

	# ---------- 逐架构 ----------
	local entry arch cmake_arch suffix enabled
	for entry in "${BUILD_LIST[@]}"; do
		IFS='|' read -r arch cmake_arch suffix enabled <<< "$entry"

		if [ "$enabled" != "true" ]; then
			log "跳过架构: $arch (enabled=$enabled)"
			continue
		fi

		log "架构: arch=$arch cmake_arch=$cmake_arch suffix=$suffix"

		build_one_arch "$arch" "$cmake_arch"

		# 字体合并进 .app
		local font_dst="${packet_dir}/macos-${arch}/${app_name}.app/Contents/Resources/fonts"
		mkdir -p "$font_dst"
		cp -R "${font_stage}/"* "$font_dst/"

		# 打包内容只展示一次：7z / dmg / pkg 都基于这份 .app
		print_packet_tree "${packet_dir}/macos-${arch}" 5

		package_one_arch "$arch" "$suffix"
	done

	# ---------- universal ----------
	if [ "$BUILD_UNIVERSAL" = "true" ]; then
		build_and_package_universal
	fi

	log "========== 全部完成 =========="
}

main "$@"
