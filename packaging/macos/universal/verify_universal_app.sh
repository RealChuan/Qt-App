#!/bin/bash

set -e

# 颜色输出
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# 日志函数
log_info() { echo -e "${GREEN}[INFO]${NC} $1"; }
log_warn() { echo -e "${YELLOW}[WARN]${NC} $1"; }
log_error() { echo -e "${RED}[ERROR]${NC} $1"; }
log_debug() { echo -e "${BLUE}[DEBUG]${NC} $1"; }

# 显示用法
usage() {
	echo "用法: $0 <app_path> [选项]"
	echo "选项:"
	echo "  -v, --verbose    显示详细信息"
	echo "  -s, --summary    只显示摘要"
	echo "  -a, --all-archs  显示所有架构（不仅仅是 x86_64 和 arm64）"
	echo "  -h, --help       显示帮助信息"
	echo ""
	echo "示例:"
	echo "  $0 MyApp.app"
	echo "  $0 MyApp.app -v"
	echo "  $0 MyApp.app --summary"
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
		file -b "$file" 2>/dev/null | grep -q "Mach-O"
		return $?
	fi
	return 1
}

# 检查文件是否已经是 Universal 二进制文件
is_universal_file() {
	local file="$1"
	local check_all_archs="$2"

	if [[ -f "$file" ]]; then
		local arches=$(lipo -archs "$file" 2>/dev/null || echo "")
		if [[ -n "$arches" ]]; then
			# 如果指定了检查所有架构，直接返回架构列表
			if [[ "$check_all_archs" == "true" ]]; then
				echo "$arches"
				return 0
			fi

			# 检查是否包含 x86_64 和 arm64
			if echo "$arches" | grep -q "x86_64" && echo "$arches" | grep -q "arm64"; then
				echo "$arches"
				return 0
			fi
		fi
	fi
	echo ""
	return 1
}

# 获取文件类型描述
get_file_type() {
	local file="$1"
	local file_type=$(file -b "$file" 2>/dev/null || echo "未知")
	echo "$file_type"
}

# 验证单个文件
verify_file() {
	local file="$1"
	local verbose="$2"
	local check_all_archs="$3"

	if ! is_macho_file "$file"; then
		if [[ "$verbose" == "true" ]]; then
			log_debug "非 Mach-O 文件: $file"
		fi
		return 1
	fi

	local arches=$(is_universal_file "$file" "$check_all_archs")
	local relative_path="${file#$app_path/}"

	if [[ -n "$arches" ]]; then
		if [[ "$check_all_archs" == "true" ]] || (echo "$arches" | grep -q "x86_64" && echo "$arches" | grep -q "arm64"); then
			if [[ "$verbose" == "true" ]]; then
				local file_type=$(get_file_type "$file")
				log_info "✓ UNIVERSAL: $relative_path"
				echo "      架构: $arches"
				echo "      类型: $file_type"
			fi
			return 0
		else
			if [[ "$verbose" == "true" ]]; then
				local file_type=$(get_file_type "$file")
				log_warn "⚠ 非 Universal: $relative_path"
				echo "      架构: $arches"
				echo "      类型: $file_type"
			else
				log_warn "非 Universal: $relative_path ($arches)"
			fi
			return 1
		fi
	else
		if [[ "$verbose" == "true" ]]; then
			local file_type=$(get_file_type "$file")
			log_error "✗ 无法读取架构: $relative_path"
			echo "      类型: $file_type"
		else
			log_error "无法读取架构: $relative_path"
		fi
		return 2
	fi
}

# 验证 App 结构
verify_app_structure() {
	local app_path="$1"

	log_info "验证 App 结构..."

	# 检查必要的目录结构
	local necessary_dirs=("Contents" "Contents/MacOS" "Contents/Resources" "Contents/Frameworks")
	local missing_dirs=()

	for dir in "${necessary_dirs[@]}"; do
		if [[ ! -d "$app_path/$dir" ]]; then
			missing_dirs+=("$dir")
		fi
	done

	if [[ ${#missing_dirs[@]} -gt 0 ]]; then
		log_warn "缺少标准目录: ${missing_dirs[*]}"
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
		if [[ -f "$app_path/Contents/MacOS/$executable" ]]; then
			log_info "✓ 主可执行文件存在"
		else
			log_error "✗ 主可执行文件不存在: $executable"
		fi
	else
		log_error "✗ 缺少 Info.plist 文件"
	fi
}

# 验证权限
verify_permissions() {
	local app_path="$1"
	local macos_dir="$app_path/Contents/MacOS"

	if [[ ! -d "$macos_dir" ]]; then
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
		log_warn "MacOS 目录下没有文件"
		return 1
	fi

	if [[ $executable_files -eq $total_files ]]; then
		log_info "✓ 所有文件都有执行权限 ($executable_files/$total_files)"
	else
		log_warn "⚠ 部分文件缺少执行权限: $executable_files/$total_files"
		if [[ "$verbose" == "true" ]]; then
			for file in "${non_executable_files[@]}"; do
				log_warn "  缺少执行权限: $file"
			done
		fi
	fi
}

# 生成验证报告
generate_report() {
	local app_path="$1"
	local universal_count="$2"
	local non_universal_count="$3"
	local error_count="$4"
	local total_count="$5"

	echo ""
	echo "========================================"
	echo "          UNIVERSAL APP 验证报告"
	echo "========================================"
	echo "App: $app_path"
	echo "总二进制文件数: $total_count"
	echo "Universal 文件: $universal_count"
	echo "非 Universal 文件: $non_universal_count"
	echo "错误文件: $error_count"
	echo ""

	local universal_percentage=0
	if [[ $total_count -gt 0 ]]; then
		universal_percentage=$((universal_count * 100 / total_count))
	fi

	if [[ $non_universal_count -eq 0 && $error_count -eq 0 ]]; then
		log_info "🎉 这是一个完整的 Universal App!"
		echo "所有二进制文件都包含 x86_64 和 arm64 架构"
		return 0
	elif [[ $universal_percentage -ge 90 ]]; then
		log_info "✓ 这是一个基本合格的 Universal App"
		echo "绝大多数二进制文件都是 Universal 架构"
		return 0
	elif [[ $universal_percentage -ge 50 ]]; then
		log_warn "⚠ 这是一个部分 Universal App"
		echo "部分二进制文件不是 Universal 架构"
		return 1
	else
		log_error "✗ 这不是一个合格的 Universal App"
		echo "大部分二进制文件缺少 Universal 架构支持"
		return 2
	fi
}

# 主验证函数
verify_app() {
	local app_path="$1"
	local verbose="$2"
	local summary_only="$3"
	local check_all_archs="$4"

	if [[ ! -d "$app_path" ]]; then
		log_error "App 不存在: $app_path"
		exit 1
	fi

	log_info "开始验证 App: $app_path"

	# 验证基本结构
	verify_app_structure "$app_path"

	# 验证权限
	verify_permissions "$app_path"

	# 统计信息
	local universal_count=0
	local non_universal_count=0
	local error_count=0
	local total_count=0

	if [[ "$summary_only" != "true" ]]; then
		log_info "检查二进制文件架构..."
	fi

	# 查找并验证所有 Mach-O 文件
	while IFS= read -r -d '' file; do
		((total_count++))

		if [[ "$summary_only" == "true" ]]; then
			# 在摘要模式下，静默检查
			if is_macho_file "$file"; then
				local arches=$(is_universal_file "$file" "$check_all_archs")
				if [[ -n "$arches" ]]; then
					if [[ "$check_all_archs" == "true" ]] || (echo "$arches" | grep -q "x86_64" && echo "$arches" | grep -q "arm64"); then
						((universal_count++))
					else
						((non_universal_count++))
					fi
				else
					((error_count++))
				fi
			fi
		else
			# 详细模式
			verify_file "$file" "$verbose" "$check_all_archs"
			local result=$?
			case $result in
			0) ((universal_count++)) ;;
			1) ((non_universal_count++)) ;;
			2) ((error_count++)) ;;
			esac
		fi
	done < <(find "$app_path" -type f -print0 2>/dev/null)

	# 生成报告
	generate_report "$app_path" "$universal_count" "$non_universal_count" "$error_count" "$total_count"

	return $?
}

# 主函数
main() {
	# 检查必要命令
	check_command lipo
	check_command file
	check_command defaults

	# 解析参数
	local app_path=""
	local verbose="false"
	local summary_only="false"
	local check_all_archs="false"

	while [[ $# -gt 0 ]]; do
		case $1 in
		-v | --verbose)
			verbose="true"
			shift
			;;
		-s | --summary)
			summary_only="true"
			shift
			;;
		-a | --all-archs)
			check_all_archs="true"
			shift
			;;
		-h | --help)
			usage
			;;
		*)
			if [[ -z "$app_path" ]]; then
				app_path="$1"
			else
				log_error "未知参数: $1"
				usage
			fi
			shift
			;;
		esac
	done

	if [[ -z "$app_path" ]]; then
		log_error "请指定要验证的 App 路径"
		usage
	fi

	# 存储 app_path 为全局变量，用于文件路径显示
	export app_path=$(realpath "$app_path" 2>/dev/null || echo "$app_path")

	# 执行验证
	verify_app "$app_path" "$verbose" "$summary_only" "$check_all_archs"
	local exit_code=$?

	exit $exit_code
}

# 运行主函数
main "$@"
