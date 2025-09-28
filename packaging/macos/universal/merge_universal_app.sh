#!/bin/bash

set -e

# 颜色输出
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# 日志函数
log_info() { echo -e "${GREEN}[INFO]${NC} $1"; }
log_warn() { echo -e "${YELLOW}[WARN]${NC} $1"; }
log_error() { echo -e "${RED}[ERROR]${NC} $1"; }

# 显示用法
usage() {
	echo "用法: $0 -x <x86_64_app_path> -a <arm64_app_path> -o <output_app_path>"
	echo "示例: $0 -x MyApp_x86_64.app -a MyApp_arm64.app -o MyApp_Universal.app"
	exit 1
}

# 检查命令是否存在
check_command() {
	if ! command -v "$1" &>/dev/null; then
		log_error "命令 $1 不存在，请安装后重试"
		exit 1
	fi
}

# 检查文件是否为 Mach-O 文件
is_macho_file() {
	local file="$1"
	if [[ -f "$file" ]]; then
		file -b "$file" | grep -q "Mach-O"
		return $?
	fi
	return 1
}

# 检查文件是否已经是 Universal 二进制文件
is_universal_file() {
	local file="$1"
	if [[ -f "$file" ]]; then
		local arches=$(lipo -archs "$file" 2>/dev/null)
		if [[ $? -eq 0 ]]; then
			# 检查是否包含两种架构
			if echo "$arches" | grep -q "x86_64" && echo "$arches" | grep -q "arm64"; then
				return 0
			fi
		fi
	fi
	return 1
}

# 合并 Mach-O 文件
merge_macho_file() {
	local x86_file="$1"
	local arm_file="$2"
	local output_file="$3"

	if [[ ! -f "$x86_file" ]] || [[ ! -f "$arm_file" ]]; then
		log_warn "文件不存在，跳过: $x86_file 或 $arm_file"
		return 1
	fi

	if is_universal_file "$x86_file"; then
		log_info "文件已经是 Universal: $(basename "$x86_file")"
		cp "$x86_file" "$output_file"
		return 0
	fi

	if is_universal_file "$arm_file"; then
		log_info "文件已经是 Universal: $(basename "$arm_file")"
		cp "$arm_file" "$output_file"
		return 0
	fi

	log_info "合并文件: $(basename "$output_file")"
	lipo -create "$x86_file" "$arm_file" -output "$output_file"

	# 检查合并是否成功
	if lipo -archs "$output_file" | grep -q "x86_64" && lipo -archs "$output_file" | grep -q "arm64"; then
		log_info "✓ 成功合并: $(basename "$output_file")"
		return 0
	else
		log_error "合并失败: $(basename "$output_file")"
		return 1
	fi
}

# 简单的 RPATH 检查
check_rpath() {
	local file="$1"

	if ! is_macho_file "$file"; then
		return 0
	fi

	log_info "检查 RPATH: $(basename "$file")"

	# 获取当前的 RPATH
	local current_rpaths=$(otool -l "$file" 2>/dev/null | grep -A2 LC_RPATH | grep path | awk '{print $2}' 2>/dev/null || true)

	if [[ -z "$current_rpaths" ]]; then
		log_info "  没有设置 RPATH"
		return 0
	fi

	# 分析 RPATH 设置
	for rpath in $current_rpaths; do
		if [[ "$rpath" == @loader_path/* ]] || [[ "$rpath" == @executable_path/* ]]; then
			log_info "  ✓ 相对路径: $rpath"
		else
			log_warn "  ⚠ 绝对路径: $rpath"
		fi
	done

	return 0
}

# 递归处理目录
process_directory() {
	local x86_dir="$1"
	local arm_dir="$2"
	local output_dir="$3"

	# 创建输出目录
	mkdir -p "$output_dir"

	# 处理文件
	while IFS= read -r -d '' item; do
		local base_name=$(basename "$item")
		local x86_item="$x86_dir/$base_name"
		local arm_item="$arm_dir/$base_name"
		local output_item="$output_dir/$base_name"

		if [[ -L "$x86_item" ]]; then
			# 处理符号链接
			local link_target=$(readlink "$x86_item")
			ln -sf "$link_target" "$output_item"
			log_info "创建符号链接: $base_name -> $link_target"

		elif [[ -f "$x86_item" ]]; then
			if is_macho_file "$x86_item"; then
				# 合并二进制文件
				if merge_macho_file "$x86_item" "$arm_item" "$output_item"; then
					# 使用简单的 RPATH 检查（不修改）
					check_rpath "$output_item"
				fi
			else
				# 非 Mach-O 文件，直接复制（使用 x86_64 版本）
				cp "$x86_item" "$output_item"
				log_info "复制非二进制文件: $base_name"
			fi

		elif [[ -d "$x86_item" ]]; then
			# 递归处理子目录
			log_info "处理目录: $base_name"
			process_directory "$x86_item" "$arm_item" "$output_item"
		fi
	done < <(find "$x86_dir" -mindepth 1 -maxdepth 1 -print0 2>/dev/null)
}

# 设置执行权限
set_executable_permissions() {
	local app_path="$1"
	local macos_dir="$app_path/Contents/MacOS"

	if [[ ! -d "$macos_dir" ]]; then
		log_warn "找不到 MacOS 目录: $macos_dir"
		return 1
	fi

	log_info "设置执行权限: $macos_dir"

	# 设置目录权限
	find "$macos_dir" -type d -exec chmod 755 {} \;
	log_info "✓ 设置目录权限为 755"

	# 设置文件执行权限
	find "$macos_dir" -type f -exec chmod +x {} \;
	log_info "✓ 设置所有文件为可执行"

	# 特别处理主可执行文件
	local main_executable=$(defaults read "$app_path/Contents/Info.plist" CFBundleExecutable 2>/dev/null || true)
	if [[ -n "$main_executable" && -f "$macos_dir/$main_executable" ]]; then
		chmod 755 "$macos_dir/$main_executable"
		log_info "设置主可执行文件权限: $main_executable"
	fi
}

# 主函数
main() {
	# 检查必要命令
	check_command lipo
	check_command file
	check_command defaults

	# 解析参数
	local x86_app arm_app output_app
	while getopts "x:a:o:h" opt; do
		case $opt in
		x) x86_app="$OPTARG" ;;
		a) arm_app="$OPTARG" ;;
		o) output_app="$OPTARG" ;;
		h) usage ;;
		*) usage ;;
		esac
	done

	# 验证参数
	if [[ -z "$x86_app" || -z "$arm_app" || -z "$output_app" ]]; then
		usage
	fi

	if [[ ! -d "$x86_app" ]]; then
		log_error "x86_64 App 不存在: $x86_app"
		exit 1
	fi

	if [[ ! -d "$arm_app" ]]; then
		log_error "arm64 App 不存在: $arm_app"
		exit 1
	fi

	if [[ -e "$output_app" ]]; then
		log_warn "输出目录已存在，将被删除: $output_app"
		rm -rf "$output_app"
	fi

	log_info "开始合并 App..."
	log_info "x86_64 App: $x86_app"
	log_info "arm64 App: $arm_app"
	log_info "输出 Universal App: $output_app"

	process_directory "$x86_app" "$arm_app" "$output_app"

	# 设置基本文件权限
	log_info "设置文件权限..."
	find "$output_app" -type f -exec chmod 644 {} \;
	find "$output_app" -type d -exec chmod 755 {} \;

	# 设置执行权限
	set_executable_permissions "$output_app"

	log_info "✓ Universal App 创建完成: $output_app"

	# 验证架构
	log_info "验证 Universal App 架构..."
	local verify_result=0
	while IFS= read -r -d '' file; do
		if is_macho_file "$file"; then
			local arches=$(lipo -archs "$file" 2>/dev/null || echo "未知")
			if echo "$arches" | grep -q "x86_64" && echo "$arches" | grep -q "arm64"; then
				log_info "✓ $(basename "$file"): $arches"
			else
				log_warn "⚠ $(basename "$file"): $arches"
				verify_result=1
			fi
		fi
	done < <(find "$output_app" -type f -print0)

	# 验证权限
	log_info "验证文件权限..."
	local macos_dir="$output_app/Contents/MacOS"
	if [[ -d "$macos_dir" ]]; then
		local non_exec_files=$(find "$macos_dir" -type f ! -perm -111 | wc -l)
		if [[ $non_exec_files -eq 0 ]]; then
			log_info "✓ 所有 MacOS 目录下的文件都有执行权限"
		else
			log_warn "⚠ 发现 $non_exec_files 个文件没有执行权限"
			# 显示没有执行权限的文件
			find "$macos_dir" -type f ! -perm -111 -exec ls -la {} \;
		fi
	fi

	if [[ $verify_result -eq 0 ]]; then
		log_info "✓ 所有二进制文件都已成功合并为 Universal"
	else
		log_warn "⚠ 部分二进制文件可能不是 Universal 架构"
	fi

	log_info "🎉 合并完成！Universal App 已创建: $output_app"
}

# 运行主函数
main "$@"
