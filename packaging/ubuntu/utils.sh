#!/bin/bash
set -euo pipefail

# ============================================================
# Ubuntu 打包工具库
# ============================================================

# 本文件所在目录：在 source 时解析一次。
# 调用方稍后会 cd 到别处，且 BASH_SOURCE 可能是相对路径（source utils.sh），
# 所以不能等到函数执行时再算。
UTILS_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

log_info()  { echo "[INFO] $*"; }
log_warn()  { echo "[WARN] $*"; }
log_error() { echo "[ERROR] $*" >&2; }
die()       { log_error "FATAL: $*"; exit 1; }

# 跨平台通用函数：check_tool / download_fonts / print_packet_tree
source "${UTILS_DIR}/../utils.sh"

# ============================================================
# 共享库依赖校验
#   packet 内每个 ELF 的 ldd 结果中都不允许出现 "not found"，
#   出现即说明安装包缺库（linuxdeploy 漏拷 / 插件依赖未部署）。
# ============================================================
verify_no_missing_libs() {
	local dir="$1"
	local failed=0 f out
	while IFS= read -r -d '' f; do
		file -b "$f" 2>/dev/null | grep -q "ELF" || continue
		out="$(ldd "$f" 2>/dev/null | grep "not found" || true)"
		if [ -n "$out" ]; then
			log_error "缺失依赖: ${f#"$dir"/}"
			printf '%s\n' "$out" | sed 's/^/    /'
			failed=$((failed + 1))
		fi
	done < <(find "$dir" -type f -print0)
	[ "$failed" -eq 0 ] || die "有 ${failed} 个二进制缺少共享库（详见上方）"
	log_info "✓ 共享库依赖检查通过"
}

# ============================================================
# AppImage 打包：linuxdeploy 部署依赖与 Qt，appimagetool 封包。
#
# 拆成两个工具是因为 go-appimage 的 `appimagetool deploy` 在 Qt 6.12 上会
# os.Exit(1)：它要改写 libQt6Core 里的 qt_prfxpath= 锚点，而 Qt 6.12 是
# relocatable 构建，二进制里没有这个字符串（上游 issue #216/#300）。
# 封包路径不碰这段逻辑，可以继续用。
#
# 两个工具自身都是 AppImage，统一用 --appimage-extract-and-run 免 FUSE
# 运行；拿到非 AppImage 版本时设 APPIMAGETOOL_PLAIN=1。
# ============================================================

# 统一入口：默认走 extract-and-run，APPIMAGETOOL_PLAIN=1 时直接调用
appimagetool_run() {
	if [ "${APPIMAGETOOL_PLAIN:-0}" = "1" ]; then
		appimagetool "$@"
	else
		appimagetool --appimage-extract-and-run "$@"
	fi
}

linuxdeploy_run() {
	if [ "${APPIMAGETOOL_PLAIN:-0}" = "1" ]; then
		linuxdeploy "$@"
	else
		linuxdeploy --appimage-extract-and-run "$@"
	fi
}

# 当前架构在 AppImage 里的写法（$ARCH 决定产物文件名）
detect_appimage_arch() {
	case "$(uname -m)" in
	x86_64 | amd64) echo "x86_64" ;;
	aarch64 | arm64) echo "aarch64" ;;
	*)
		log_error "无法识别的宿主机架构: $(uname -m)"
		return 1
		;;
	esac
}

# 部署结果的三个计数：能区分「一个库都没拷」和「只漏了插件」，
# 更细的内容交给 verify_no_missing_libs 的 ldd 检查。
report_deployed_tree() {
	local appdir="$1"
	local libs qtplugins ownplugins
	libs=$(find "$appdir" -name "*.so*" -type f 2>/dev/null | wc -l | tr -d ' ')
	qtplugins=$(find "$appdir" -path "*/plugins/platforms/*.so" -type f 2>/dev/null | wc -l | tr -d ' ')
	ownplugins=$(find "$appdir/usr/plugins" -name "*.so" -type f 2>/dev/null | wc -l | tr -d ' ')
	log_info "部署结果: .so ${libs} 个（Qt 平台插件 ${qtplugins}，应用插件 ${ownplugins}），AppDir $(du -sh "$appdir" | cut -f1)"
}

# 清掉只服务于 AppImage 的文件，给 deb / 7z 用
clean_appdir_for_packaging() {
	local appdir="$1" app_name="$2"
	log_info "清理 AppImage 专用产物（AppRun / .DirIcon / *.zsync / 根 desktop 副本）"
	rm -fv "${appdir}/AppRun" "${appdir}/.DirIcon"
	rm -fv "${appdir}"/*.zsync 2>/dev/null || true
	rm -fv "${appdir}/${app_name}.desktop" 2>/dev/null || true
	# 注意：AppDir 根的 plugins/ 与 translations/ 不要动。
	# linuxdeploy 把 Qt 插件的真身放在根上，再在 usr/ 下建指回来的相对软链；
	# 删掉根上的真身，usr/ 里的软链就全悬空了。
}

# 部署依赖并封包
#   $1 AppDir 根  $2 应用名  $3 版本号  $4 是否封包（默认 true）
#   前置：<appdir>/usr/bin/<app_name> 与 <appdir>/usr/share/applications/<app_name>.desktop
deploy_appimage() {
	local appdir="$1" app_name="$2" app_version="$3" want_appimage="${4:-true}"
	local desktop_file="${appdir}/usr/share/applications/${app_name}.desktop"

	check_tool linuxdeploy
	check_tool appimagetool
	[ -d "$appdir" ] || die "AppDir 不存在: $appdir"
	[ -d "${appdir}/usr/bin" ] ||
		die "AppDir 不是 FHS 布局：缺少 ${appdir}/usr/bin"
	[ -x "${appdir}/usr/bin/${app_name}" ] ||
		die "缺少可执行文件: ${appdir}/usr/bin/${app_name}"
	[ -f "$desktop_file" ] ||
		die "缺少 ${desktop_file}：linuxdeploy 靠它生成 AppRun 与根上的链接"
	command -v linuxdeploy-plugin-qt >/dev/null 2>&1 ||
		die "找不到 linuxdeploy-plugin-qt：--plugin qt 需要它在 PATH 上"

	# QTDIR：插件靠 QMAKE 定位 Qt；CI 由 install-qt-action 提供 QT_ROOT_DIR
	local qtdir="${QTDIR:-${QT_ROOT_DIR:-}}"
	local qmake_bin
	if [ -z "$qtdir" ]; then
		qmake_bin=$(command -v qmake 2>/dev/null || command -v qmake6 2>/dev/null || true)
		[ -n "$qmake_bin" ] || die "QTDIR / QT_ROOT_DIR 都未设置，且 PATH 上没有 qmake"
		qtdir=$("$qmake_bin" -query QT_INSTALL_PREFIX)
		log_warn "QTDIR 未设置，从 qmake 推断: $qtdir"
	fi
	[ -d "$qtdir" ] || die "QTDIR 指向的目录不存在: $qtdir"
	[ -d "${qtdir}/plugins/platforms" ] ||
		die "QTDIR=${qtdir} 下没有 plugins/platforms，Qt 插件无处可取"
	export QTDIR="$qtdir"

	# Qt 装在非标准路径下，不塞进 LD_LIBRARY_PATH 就找不到 libQt6Core
	export LD_LIBRARY_PATH="${qtdir}/lib:${LD_LIBRARY_PATH:-}"

	# 不显式给 QMAKE 的话，插件会依赖 PATH 上碰巧存在的 qmake
	[ -n "${qmake_bin:-}" ] || qmake_bin="${qtdir}/bin/qmake"
	[ -x "$qmake_bin" ] || qmake_bin=$(command -v qmake || true)
	[ -n "$qmake_bin" ] || die "找不到可执行的 qmake（QTDIR=${qtdir}）"
	export QMAKE="$qmake_bin"

	# $ARCH 决定产物文件名，也避免它去 AppRun（是个脚本）上猜架构
	local appimage_arch
	appimage_arch=$(detect_appimage_arch) || die "无法确定 AppImage 架构"
	export ARCH="$appimage_arch"

	log_info "部署 AppDir=${appdir}"
	log_info "QTDIR=${QTDIR} QMAKE=${QMAKE} ARCH=${ARCH}"

	# 插件默认只打 libqxcb.so，Wayland 会话下会报 "Could not find ... wayland"
	local platforms_dir="${QTDIR}/plugins/platforms"
	if [ -f "${platforms_dir}/libqwayland-egl.so" ] &&
		[ -f "${platforms_dir}/libqwayland-generic.so" ]; then
		export EXTRA_PLATFORM_PLUGINS="libqwayland-egl.so;libqwayland-generic.so"
		log_info "额外部署 Wayland 平台插件: ${EXTRA_PLATFORM_PLUGINS}"
	fi

	# --deploy-deps-only 只处理目录第一层，所以逐层传进去
	local -a ld_args=(--appdir "$appdir" --plugin qt)
	local d
	while IFS= read -r d; do
		ld_args+=(--deploy-deps-only "$d")
	done < <(find "${appdir}/usr/bin" "${appdir}/usr/plugins" -type d 2>/dev/null | sort)

	# 用自定义 AppRun 脚本替换默认的符号链接，原因见该脚本
	if [ -f "${UTILS_DIR}/AppRun" ]; then
		ld_args+=(--custom-apprun "${UTILS_DIR}/AppRun")
		log_info "使用自定义 AppRun: ${UTILS_DIR}/AppRun"
	else
		log_warn "缺少 ${UTILS_DIR}/AppRun，将使用 linuxdeploy 生成的符号链接（AppImage 可能找不到插件与翻译）"
	fi

	if ! linuxdeploy_run "${ld_args[@]}"; then
		die "linuxdeploy 部署失败（AppDir=${appdir}）"
	fi

	report_deployed_tree "$appdir"

	if [ -z "$(find "$appdir" -name 'libQt6Core.so*' -type f -print -quit 2>/dev/null)" ]; then
		die "部署后整棵树里找不到 libQt6Core.so*，QTDIR=${QTDIR} 可能指错了 Qt"
	fi
	[ -f "${appdir}/usr/bin/qt.conf" ] ||
		die "linuxdeploy-plugin-qt 没有生成 ${appdir}/usr/bin/qt.conf"

	# ---- 封包 ----
	if [ "$want_appimage" = "true" ]; then
		log_info "封包 AppImage（VERSION=${app_version} ARCH=${ARCH}）"
		# 产物落在当前工作目录，所以必须 cd 进 AppDir
		if ! (
			cd "$appdir" && export VERSION="$app_version" && appimagetool_run "$appdir"
		); then
			die "appimagetool 封包失败（AppDir=${appdir}）"
		fi
		echo "---- ${appdir} 下的 AppImage / zsync ----"
		find "$appdir" -maxdepth 1 \( -name "*.AppImage" -o -name "*.zsync" \) \
			-printf "%f (%s bytes)\n" 2>/dev/null | sort || true
	fi
}
