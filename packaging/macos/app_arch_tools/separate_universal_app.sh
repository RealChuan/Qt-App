#!/bin/bash

set -e

# 引入 macOS App 工具函数
source "$(dirname "$0")/macos_app_utils.sh"

# 显示用法
usage() {
	echo "用法: $0 -i <input_universal_app> -o <output_app_path> [-a <architecture>]"
	echo "选项:"
	echo "  -i, --input     输入的 Universal App 路径"
	echo "  -o, --output    输出的单架构 App 路径"
	echo "  -a, --arch      目标架构 (x86_64 或 arm64)，仅在检测到 Universal 架构时需要"
	echo "  -f, --force     强制分离（即使检测到已经是单架构）"
	echo "  -h, --help      显示此帮助信息"
	echo ""
	echo "示例:"
	echo "  $0 -i MyApp.app -o MyApp_x86_64.app -a x86_64"
	echo "  $0 -i MyApp.app -o MyApp_arm64.app -a arm64"
	exit 1
}

# 检测 App 的整体架构类型
detect_app_architecture() {
	local app_path="$1"
	local macos_dir="$app_path/Contents/MacOS"

	if [[ ! -d "$macos_dir" ]]; then
		log_error "找不到 MacOS 目录: $macos_dir"
		exit 1
	fi

	local has_universal=false
	local has_x86_64=false
	local has_arm64=false
	local has_other=false
	local macho_files_count=0

	# 检查所有 Mach-O 文件
	while IFS= read -r -d '' file; do
		if is_macho_file "$file"; then
			((macho_files_count++))
			local arches=$(get_file_architectures "$file")
			log_debug "文件 $(basename "$file") 架构: $arches"

			case "$arches" in
			*x86_64*arm64* | *arm64*x86_64*)
				has_universal=true
				;;
			*x86_64*)
				has_x86_64=true
				;;
			*arm64*)
				has_arm64=true
				;;
			*)
				has_other=true
				log_warn "发现不支持的架构: $file - $arches"
				;;
			esac
		fi
	done < <(find "$macos_dir" -type f -print0 2>/dev/null)

	if [[ $macho_files_count -eq 0 ]]; then
		log_error "在 MacOS 目录中没有找到任何 Mach-O 文件"
		echo "unknown"
		return
	fi

	# 分析检测结果
	if [[ "$has_universal" == "true" ]]; then
		echo "universal"
	elif [[ "$has_x86_64" == "true" && "$has_arm64" == "false" && "$has_other" == "false" ]]; then
		echo "x86_64"
	elif [[ "$has_arm64" == "true" && "$has_x86_64" == "false" && "$has_other" == "false" ]]; then
		echo "arm64"
	elif [[ "$has_x86_64" == "true" && "$has_arm64" == "true" ]]; then
		echo "mixed"
	elif [[ "$has_other" == "true" ]]; then
		echo "unknown"
	else
		echo "unknown"
	fi
}

# 分离单个 Mach-O 文件
extract_macho_architecture() {
	local input_file="$1"
	local output_file="$2"
	local target_arch="$3"
	local force="$4"

	if [[ ! -f "$input_file" ]]; then
		log_warn "文件不存在，跳过: $input_file"
		return 1
	fi

	if ! is_macho_file "$input_file"; then
		# 非 Mach-O 文件，直接复制
		cp "$input_file" "$output_file"
		return 0
	fi

	# 移除签名以便 lipo 可以正常工作
	remove_signature "$input_file"

	local arches=$(get_file_architectures "$input_file")

	# 检查是否已经是目标架构
	if [[ "$arches" == "$target_arch" ]]; then
		if [[ "$force" == "true" ]]; then
			log_info "文件已经是 $target_arch 架构，强制复制: $(basename "$input_file")"
			cp "$input_file" "$output_file"
		else
			log_info "文件已经是 $target_arch 架构，跳过: $(basename "$input_file")"
			cp "$input_file" "$output_file"
		fi
		return 0
	fi

	# 检查是否包含目标架构
	if echo "$arches" | grep -q "$target_arch"; then
		log_info "从 $(basename "$input_file") 提取 $target_arch 架构"
		lipo "$input_file" -thin "$target_arch" -output "$output_file"

		# 验证提取结果
		local extracted_arches=$(get_file_architectures "$output_file")
		if [[ "$extracted_arches" == "$target_arch" ]]; then
			log_info "✓ 成功提取: $(basename "$input_file") -> $target_arch"
			return 0
		else
			log_error "提取失败: $(basename "$input_file")，得到架构: $extracted_arches"
			return 1
		fi
	else
		log_error "文件 $(basename "$input_file") 不包含 $target_arch 架构，当前架构: $arches"
		return 1
	fi
}

# 处理目录进行分离
process_directory_for_extract() {
	local input_dir="$1"
	local output_dir="$2"
	local target_arch="$3"
	local force="$4"

	# 创建输出目录
	mkdir -p "$output_dir"

	# 处理文件和子目录
	while IFS= read -r -d '' item; do
		local base_name=$(basename "$item")
		local input_item="$input_dir/$base_name"
		local output_item="$output_dir/$base_name"

		if [[ -L "$input_item" ]]; then
			# 对于符号链接，创建新的符号链接
			local link_target=$(readlink "$input_item")
			ln -sf "$link_target" "$output_item"
			log_info "创建符号链接: $base_name -> $link_target"

		elif [[ -f "$input_item" ]]; then
			# 处理文件
			if extract_macho_architecture "$input_item" "$output_item" "$target_arch" "$force"; then
				# 使用简单的 RPATH 检查（不修改）
				check_rpath "$output_item"
			fi

		elif [[ -d "$input_item" ]]; then
			# 递归处理子目录
			log_info "处理目录: $base_name"
			process_directory_for_extract "$input_item" "$output_item" "$target_arch" "$force"
		fi
	done < <(find "$input_dir" -mindepth 1 -maxdepth 1 -print0 2>/dev/null)
}

# 验证输出 App 的架构
verify_output_architecture() {
	local app_path="$1"
	local target_arch="$2"

	local macos_dir="$app_path/Contents/MacOS"
	if [[ ! -d "$macos_dir" ]]; then
		log_error "输出 App 结构不完整: 缺少 Contents/MacOS"
		return 1
	fi

	log_info "验证输出 App 架构..."
	local all_correct=true

	while IFS= read -r -d '' file; do
		if is_macho_file "$file"; then
			local arches=$(get_file_architectures "$file")
			if [[ "$arches" == "$target_arch" ]]; then
				log_info "✓ $(basename "$file"): $arches"
			else
				log_error "✗ $(basename "$file"): 期望 $target_arch，实际 $arches"
				all_correct=false
			fi
		fi
	done < <(find "$macos_dir" -type f -print0 2>/dev/null)

	if [[ "$all_correct" == "true" ]]; then
		log_info "✓ 所有二进制文件都已成功转换为 $target_arch 架构"
		return 0
	else
		log_error "✗ 部分二进制文件架构不正确"
		return 1
	fi
}

# 主函数
main() {
	# 检查必要命令
	check_command lipo
	check_command file
	check_command defaults
	check_command codesign

	# 解析参数
	local input_app output_app target_arch force_flag
	while [[ $# -gt 0 ]]; do
		case $1 in
		-i | --input)
			input_app="$2"
			shift 2
			;;
		-o | --output)
			output_app="$2"
			shift 2
			;;
		-a | --arch)
			target_arch="$2"
			shift 2
			;;
		-f | --force)
			force_flag="true"
			shift
			;;
		-h | --help)
			usage
			;;
		*)
			log_error "未知参数: $1"
			usage
			;;
		esac
	done

	# 验证必要参数
	if [[ -z "$input_app" || -z "$output_app" ]]; then
		log_error "缺少必要参数"
		usage
	fi

	if [[ ! -d "$input_app" ]]; then
		log_error "输入 App 不存在: $input_app"
		exit 1
	fi

	if [[ -e "$output_app" ]]; then
		log_warn "输出目录已存在，将被删除: $output_app"
		rm -rf "$output_app"
	fi

	log_info "开始检测输入 App 架构..."
	local detected_arch
	detected_arch=$(detect_app_architecture "$input_app")

	log_info "输入 App 架构检测结果: $detected_arch"

	# 根据检测结果处理目标架构
	case "$detected_arch" in
	"universal")
		if [[ -z "$target_arch" ]]; then
			log_error "检测到 Universal App，必须使用 -a 参数指定目标架构 (x86_64 或 arm64)"
			echo ""
			echo "请使用以下方式之一运行脚本:"
			echo "  $0 -i \"$input_app\" -o \"$output_app\" -a x86_64"
			echo "  $0 -i \"$input_app\" -o \"$output_app\" -a arm64"
			exit 1
		fi

		if [[ "$target_arch" != "x86_64" && "$target_arch" != "arm64" ]]; then
			log_error "不支持的架构: $target_arch，支持 x86_64 或 arm64"
			exit 1
		fi

		log_info "检测到 Universal App，开始分离 $target_arch 架构..."
		;;

	"x86_64" | "arm64")
		# 单架构 App，自动确定目标架构
		local auto_target="$detected_arch"

		if [[ -n "$target_arch" && "$target_arch" != "$auto_target" ]]; then
			log_warn "检测到 App 是 $auto_target 架构，但指定了 $target_arch 架构，将使用检测到的架构: $auto_target"
		fi

		target_arch="$auto_target"
		log_info "检测到单架构 App ($target_arch)，直接处理..."
		;;

	"mixed")
		log_error "检测到混合架构（部分文件为 x86_64，部分为 arm64），这不符合预期"
		log_error "请检查输入 App 的完整性"
		exit 1
		;;

	"unknown")
		log_error "无法检测 App 架构或发现不支持的架构"
		log_error "请确认输入是有效的 macOS App"
		exit 1
		;;

	*)
		log_error "未知的架构检测结果: $detected_arch"
		exit 1
		;;
	esac

	log_info "开始处理 App..."
	log_info "输入 App: $input_app"
	log_info "目标架构: $target_arch"
	log_info "输出 App: $output_app"

	# 复制并处理整个 App 结构
	process_directory_for_extract "$input_app" "$output_app" "$target_arch" "$force_flag"

	# 设置基本文件权限
	log_info "设置文件权限..."
	find "$output_app" -type f -exec chmod 644 {} \;
	find "$output_app" -type d -exec chmod 755 {} \;

	# 设置执行权限
	set_executable_permissions "$output_app"

	# 验证权限（使用公共函数）
	verify_permissions "$output_app" "true"

	# 验证输出架构
	if verify_output_architecture "$output_app" "$target_arch"; then
		log_info "✓ 单架构 App 创建完成: $output_app"
		log_info "🎉 处理完成！$target_arch App 已创建: $output_app"
	else
		log_error "✗ 架构验证失败"
		exit 1
	fi
}

# 运行主函数
main "$@"
