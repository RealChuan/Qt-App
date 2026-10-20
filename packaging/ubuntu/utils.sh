#!/bin/bash
set -euo pipefail

# ============================================================
# Ubuntu 打包工具库
# ============================================================

# 本文件所在目录 / packaging 根目录：在 source 时解析一次。
# 调用方稍后会 cd 到别处，且 BASH_SOURCE 可能是相对路径（source utils.sh），
# 所以不能等到函数执行时再算。
UTILS_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PACKAGING_DIR="$(cd "${UTILS_DIR}/.." && pwd)"

log_info()  { echo "[INFO] $*"; }
log_warn()  { echo "[WARN] $*"; }
log_error() { echo "[ERROR] $*" >&2; }
die()       { log_error "FATAL: $*"; exit 1; }

# 检查命令是否存在
check_tool() {
	command -v "$1" >/dev/null 2>&1 || die "缺少命令: $1"
}

# 字体下载（替代原 fonts.cmake，7z 走 PATH）
#   目标路径：packet/linux-<arch>/resources/fonts
download_fonts() {
	local dest="$1" url="${2:-}"
	local url_file="${PACKAGING_DIR}/fonts.url"
	if [ -z "$url" ]; then
		[ -f "$url_file" ] || die "缺少字体地址文件: $url_file"
		url="$(tr -d '[:space:]' <"$url_file")"
	fi
	local archive
	archive=$(mktemp -t fonts.XXXXXX.7z)

	log_info "下载字体: $url"
	if ! curl -fsSL -o "$archive" "$url"; then
		rm -f "$archive"
		die "字体下载失败"
	fi

	mkdir -p "$dest"
	log_info "解压字体 -> $dest"
	if ! 7z x "$archive" "-o$dest" -y >/dev/null; then
		rm -f "$archive"
		die "字体解压失败"
	fi
	rm -f "$archive"

	local count
	count=$(find "$dest" -type f | wc -l | tr -d ' ')
	[ "$count" -gt 0 ] || die "字体解压后为空"
	log_info "字体下载完成，共 $count 个文件"
}

# ============================================================
# 共享库依赖校验
#   packet 内每个 ELF 的 ldd 结果中都不允许出现 "not found"，
#   出现即说明安装包缺库（linuxdeployqt 漏拷 / 插件依赖未部署）。
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
