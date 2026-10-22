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
