#!/bin/bash

# macOS App 架构管理工具函数库

set -e

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# 日志函数
log_info() { echo -e "${GREEN}[INFO]${NC} $1" >&2; }
log_warn() { echo -e "${YELLOW}[WARN]${NC} $1" >&2; }
log_error() { echo -e "${RED}[ERROR]${NC} $1" >&2; }
log_debug() { echo -e "${BLUE}[DEBUG]${NC} $1" >&2; }

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
		file -b "$file" 2>/dev/null | grep -q "Mach-O"
		return $?
	fi
	return 1
}

# 获取文件的架构信息
get_file_architectures() {
	local file="$1"
	if [[ -f "$file" ]] && is_macho_file "$file"; then
		lipo -archs "$file" 2>/dev/null || echo "unknown"
	else
		echo "not_macho"
	fi
}

# 移除文件的代码签名
remove_signature() {
	local file="$1"
	if [[ -f "$file" ]] && is_macho_file "$file"; then
		if codesign --remove-signature "$file" 2>/dev/null; then
			log_debug "移除签名: $(basename "$file")"
			return 0
		else
			log_warn "无法移除签名: $(basename "$file")"
			return 1
		fi
	fi
	return 0
}

# 验证 App 基本结构
verify_app_structure() {
	local app_path="$1"

	log_info "验证 App 结构..."

	# 检查必要的目录结构
	local necessary_dirs=("Contents" "Contents/MacOS")
	local missing_dirs=()

	for dir in "${necessary_dirs[@]}"; do
		if [[ ! -d "$app_path/$dir" ]]; then
			missing_dirs+=("$dir")
		fi
	done

	if [[ ${#missing_dirs[@]} -gt 0 ]]; then
		log_error "缺少必要目录: ${missing_dirs[*]}"
		return 1
	else
		log_info "✓ App 目录结构完整"
	fi

	# 检查 Info.plist
	if [[ -f "$app_path/Contents/Info.plist" ]]; then
		local bundle_name=$(defaults read "$app_path/Contents/Info.plist" CFBundleName 2>/dev/null || echo "未知")
		local bundle_version=$(defaults read "$app_path/Contents/Info.plist" CFBundleShortVersionString 2>/dev/null || echo "未知")
		local executable=$(defaults read "$app_path/Contents/Info.plist" CFBundleExecutable 2>/dev/null || echo "未知")

		log_info "App 信息:"
		echo "      名称: $bundle_name"
		echo "      版本: $bundle_version"
		echo "      可执行文件: $executable"

		# 检查可执行文件是否存在
		if [[ ! -f "$app_path/Contents/MacOS/$executable" ]]; then
			log_error "✗ 主可执行文件不存在: $executable"
			return 1
		fi
	else
		log_error "✗ 缺少 Info.plist 文件"
		return 1
	fi

	return 0
}

# 设置 App 执行权限
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

# 验证权限
verify_permissions() {
	local app_path="$1"
	local verbose="$2"

	local macos_dir="$app_path/Contents/MacOS"

	if [[ ! -d "$macos_dir" ]]; then
		log_error "找不到 MacOS 目录: $macos_dir"
		return 1
	fi

	log_info "验证文件权限..."

	local total_files=0
	local executable_files=0
	local non_executable_files=()

	while IFS= read -r -d '' file; do
		((total_files++))
		if [[ -x "$file" ]]; then
			((executable_files++))
		else
			non_executable_files+=("${file#$app_path/}")
		fi
	done < <(find "$macos_dir" -type f -print0 2>/dev/null)

	if [[ $total_files -eq 0 ]]; then
		log_error "MacOS 目录下没有文件"
		return 1
	fi

	if [[ $executable_files -eq $total_files ]]; then
		log_info "✓ 所有文件都有执行权限 ($executable_files/$total_files)"
		return 0
	else
		log_error "✗ 部分文件缺少执行权限: $executable_files/$total_files"
		if [[ "$verbose" == "true" ]]; then
			for file in "${non_executable_files[@]}"; do
				log_error "  缺少执行权限: $file"
			done
		fi
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
