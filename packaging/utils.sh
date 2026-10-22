#!/bin/bash
# ============================================================
# 打包通用工具库（Linux / macOS 共用）
#
#   只收跨平台、且能整块复用的函数：
#     - check_tool         命令存在性检查
#     - download_fonts     字体下载 + 解压
#     - print_packet_tree  打包内容树（同一目录只打印一次）
#
#   调用方需先定义 log_info / log_warn / die：
#   日志格式与退出行为由各平台自己的 utils.sh 决定。
# ============================================================

# packaging/ 根目录：本文件就在这一层，download_fonts 用它找 fonts.url。
# 只在文件内部使用，调用方稍后会 cd 到别处，所以在 source 时解析一次。
_PACKAGING_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# 检查命令是否存在
check_tool() {
	command -v "$1" >/dev/null 2>&1 || die "缺少命令: $1"
}

# ============================================================
# 打包内容树
#   AppImage / 7z / deb / dmg / pkg 都基于同一份 packet 目录，
#   只展示一次；_PACKET_TREE_SHOWN 记录已展示过的目录。
#   $1 目录  $2 展示深度（默认 3；macOS 的 .app 需要 5）
# ============================================================
_PACKET_TREE_SHOWN=""
print_packet_tree() {
	local dir="$1" depth="${2:-3}"
	[ -d "$dir" ] || die "展示打包内容失败，目录不存在: $dir"
	case " ${_PACKET_TREE_SHOWN} " in
	*" ${dir} "*) return 0 ;;
	esac
	_PACKET_TREE_SHOWN="${_PACKET_TREE_SHOWN} ${dir}"

	local files size
	files=$(find "$dir" -type f | wc -l | tr -d ' ')
	size=$(du -sh "$dir" | cut -f1)

	log_info "========== 打包内容（${files} 个文件，${size}）=========="
	log_info "目录: ${dir}"
	if command -v tree >/dev/null 2>&1; then
		tree -a -L "$depth" "$dir"
	else
		log_warn "未安装 tree，改用缩进列表展示（安装 tree 可得到带连接线的树）"
		(
			cd "$dir"
			# 用路径层级补缩进，避免额外依赖。
			find . -maxdepth "$depth" -print | sort | awk -F/ '
				{
					indent = ""
					for (i = 1; i < NF - 1; i++)
						indent = indent "    "
					print indent $NF
				}'
		)
	fi
}

# 字体下载（替代原 fonts.cmake，7z 走 PATH）
#   目标路径由调用方给出，例如 packet/<arch>/resources/fonts
download_fonts() {
	local dest="$1" url="${2:-}"
	local url_file="${_PACKAGING_DIR}/fonts.url"
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
