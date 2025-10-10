#!/bin/bash

set -e

# 引入 macOS App 工具函数
source "$(dirname "$0")/macos_app_utils.sh"

# 显示用法
usage() {
	echo "用法: $0 <app_path> -a <architecture> [选项]"
	echo "架构选项:"
	echo "  x86_64         验证所有文件都是 x86_64 架构"
	echo "  arm64          验证所有文件都是 arm64 架构"
	echo "  universal      验证所有文件都是 universal 架构（同时包含 x86_64 和 arm64）"
	echo "  any            验证所有文件都是单一架构（x86_64 或 arm64，不允许混合）"
	echo ""
	echo "其他选项:"
	echo "  -v, --verbose    显示详细信息"
	echo "  -s, --summary    只显示摘要"
	echo "  -h, --help       显示帮助信息"
	echo ""
	echo "示例:"
	echo "  $0 MyApp.app -a x86_64"
	echo "  $0 MyApp.app -a arm64 --verbose"
	echo "  $0 MyApp.app -a universal --summary"
	echo "  $0 MyApp.app -a any"
	exit 1
}

# 验证文件是否符合目标架构
verify_file_architecture() {
	local file="$1"
	local target_arch="$2"
	local verbose="$3"

	if ! is_macho_file "$file"; then
		if [[ "$verbose" == "true" ]]; then
			log_debug "非 Mach-O 文件: $file"
		fi
		return 0 # 非 Mach-O 文件总是通过
	fi

	local arches=$(get_file_architectures "$file")
	local relative_path="${file#$app_path/}"
	local file_type=$(file -b "$file" 2>/dev/null || echo "未知")

	case "$target_arch" in
	"x86_64")
		if [[ "$arches" == "x86_64" ]]; then
			if [[ "$verbose" == "true" ]]; then
				log_info "✓ x86_64: $relative_path"
				echo "      类型: $file_type"
			fi
			return 0
		else
			if [[ "$verbose" == "true" ]]; then
				log_error "✗ 非 x86_64: $relative_path"
				echo "      实际架构: $arches"
				echo "      类型: $file_type"
			else
				log_error "非 x86_64: $relative_path ($arches)"
			fi
			return 1
		fi
		;;

	"arm64")
		if [[ "$arches" == "arm64" ]]; then
			if [[ "$verbose" == "true" ]]; then
				log_info "✓ arm64: $relative_path"
				echo "      类型: $file_type"
			fi
			return 0
		else
			if [[ "$verbose" == "true" ]]; then
				log_error "✗ 非 arm64: $relative_path"
				echo "      实际架构: $arches"
				echo "      类型: $file_type"
			else
				log_error "非 arm64: $relative_path ($arches)"
			fi
			return 1
		fi
		;;

	"universal")
		if echo "$arches" | grep -q "x86_64" && echo "$arches" | grep -q "arm64"; then
			if [[ "$verbose" == "true" ]]; then
				log_info "✓ UNIVERSAL: $relative_path"
				echo "      架构: $arches"
				echo "      类型: $file_type"
			fi
			return 0
		else
			if [[ "$verbose" == "true" ]]; then
				log_error "✗ 非 Universal: $relative_path"
				echo "      实际架构: $arches"
				echo "      类型: $file_type"
			else
				log_error "非 Universal: $relative_path ($arches)"
			fi
			return 1
		fi
		;;

	"any")
		if [[ "$arches" == "x86_64" || "$arches" == "arm64" ]]; then
			if [[ "$verbose" == "true" ]]; then
				log_info "✓ 单一架构: $relative_path"
				echo "      架构: $arches"
				echo "      类型: $file_type"
			fi
			return 0
		else
			if [[ "$verbose" == "true" ]]; then
				log_error "✗ 非单一架构: $relative_path"
				echo "      实际架构: $arches"
				echo "      类型: $file_type"
			else
				log_error "非单一架构: $relative_path ($arches)"
			fi
			return 1
		fi
		;;

	*)
		log_error "不支持的架构类型: $target_arch"
		return 1
		;;
	esac
}

# 生成验证报告
generate_report() {
	local app_path="$1"
	local target_arch="$2"
	local passed_count="$3"
	local failed_count="$4"
	local total_macho_count="$5"
	local failed_files_list="$6"

	echo ""
	echo "========================================"
	echo "           APP 架构验证报告"
	echo "========================================"
	echo "App: $app_path"
	echo "目标架构: $target_arch"
	echo "Mach-O 文件总数: $total_macho_count"
	echo "符合架构的文件: $passed_count"
	echo "不符合架构的文件: $failed_count"
	echo ""

	if [[ $failed_count -eq 0 ]]; then
		case "$target_arch" in
		"x86_64")
			log_info "🎉 这是一个完整的 x86_64 App!"
			echo "所有二进制文件都是 x86_64 架构"
			;;
		"arm64")
			log_info "🎉 这是一个完整的 arm64 App!"
			echo "所有二进制文件都是 arm64 架构"
			;;
		"universal")
			log_info "🎉 这是一个完整的 Universal App!"
			echo "所有二进制文件都包含 x86_64 和 arm64 架构"
			;;
		"any")
			log_info "🎉 这是一个架构一致的 App!"
			echo "所有二进制文件都是单一架构（x86_64 或 arm64）"
			;;
		esac
		return 0
	else
		log_error "✗ 架构验证失败!"
		echo "有 $failed_count 个文件不符合目标架构: $target_arch"
		echo ""
		log_error "不符合架构的文件列表:"
		echo "$failed_files_list"
		return 1
	fi
}

# 主验证函数
verify_app() {
	local app_path="$1"
	local target_arch="$2"
	local verbose="$3"
	local summary_only="$4"

	if [[ ! -d "$app_path" ]]; then
		log_error "App 不存在: $app_path"
		exit 1
	fi

	log_info "开始验证 App: $app_path"
	log_info "目标架构: $target_arch"

	# 验证基本结构
	if ! verify_app_structure "$app_path"; then
		log_error "App 结构验证失败"
		exit 1
	fi

	# 验证权限（使用公共函数）
	if ! verify_permissions "$app_path" "$verbose"; then
		log_error "文件权限验证失败"
		exit 1
	fi

	# 统计信息
	local passed_count=0
	local failed_count=0
	local total_macho_count=0
	local failed_files_list=""

	if [[ "$summary_only" != "true" ]]; then
		log_info "检查二进制文件架构..."
	fi

	# 查找并验证所有 Mach-O 文件
	while IFS= read -r -d '' file; do
		if is_macho_file "$file"; then
			((total_macho_count++))

			if [[ "$summary_only" == "true" ]]; then
				# 在摘要模式下，静默检查
				if verify_file_architecture "$file" "$target_arch" "false" >/dev/null 2>&1; then
					((passed_count++))
				else
					((failed_count++))
					local relative_path="${file#$app_path/}"
					local arches=$(get_file_architectures "$file")
					failed_files_list+="  - $relative_path (架构: $arches)"$'\n'
				fi
			else
				# 详细模式
				if verify_file_architecture "$file" "$target_arch" "$verbose"; then
					((passed_count++))
				else
					((failed_count++))
					local relative_path="${file#$app_path/}"
					local arches=$(get_file_architectures "$file")
					failed_files_list+="  - $relative_path (架构: $arches)"$'\n'
				fi
			fi
		fi
	done < <(find "$app_path" -type f -print0 2>/dev/null)

	if [[ $total_macho_count -eq 0 ]]; then
		log_error "在 App 中没有找到任何 Mach-O 文件"
		exit 1
	fi

	# 生成报告
	if generate_report "$app_path" "$target_arch" "$passed_count" "$failed_count" "$total_macho_count" "$failed_files_list"; then
		log_info "✓ 架构验证通过"
		exit 0
	else
		log_error "✗ 架构验证失败"
		exit 1
	fi
}

# 主函数
main() {
	# 检查必要命令
	check_command lipo
	check_command file
	check_command defaults

	# 解析参数
	local app_path=""
	local target_arch=""
	local verbose="false"
	local summary_only="false"

	while [[ $# -gt 0 ]]; do
		case $1 in
		-a | --arch)
			target_arch="$2"
			shift 2
			;;
		-v | --verbose)
			verbose="true"
			shift
			;;
		-s | --summary)
			summary_only="true"
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

	if [[ -z "$target_arch" ]]; then
		log_error "请使用 -a 参数指定目标架构"
		echo "可用架构: x86_64, arm64, universal, any"
		usage
	fi

	# 验证架构参数
	case "$target_arch" in
	"x86_64" | "arm64" | "universal" | "any")
		# 有效架构
		;;
	*)
		log_error "不支持的架构: $target_arch"
		echo "可用架构: x86_64, arm64, universal, any"
		usage
		;;
	esac

	# 存储 app_path 为全局变量，用于文件路径显示
	export app_path=$(realpath "$app_path" 2>/dev/null || echo "$app_path")

	# 执行验证
	verify_app "$app_path" "$target_arch" "$verbose" "$summary_only"
}

# 运行主函数
main "$@"
