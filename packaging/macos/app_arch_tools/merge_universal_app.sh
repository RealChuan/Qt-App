#!/bin/bash

set -e

# 引入 macOS App 工具函数
source "$(dirname "$0")/macos_app_utils.sh"

# 显示用法
usage() {
	echo "用法: $0 -x <x86_64_app_path> -a <arm64_app_path> -o <output_app_path>"
	echo "示例: $0 -x MyApp_x86_64.app -a MyApp_arm64.app -o MyApp_Universal.app"
	exit 1
}

# 检查文件是否已经是 Universal 二进制文件
is_universal_file() {
	local file="$1"
	if [[ -f "$file" ]] && is_macho_file "$file"; then
		local arches=$(get_file_architectures "$file")
		if echo "$arches" | grep -q "x86_64" && echo "$arches" | grep -q "arm64"; then
			return 0
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

	# 移除签名以便 lipo 可以正常工作
	remove_signature "$x86_file"
	remove_signature "$arm_file"

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

# 处理目录进行合并
process_directory_for_merge() {
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
			process_directory_for_merge "$x86_item" "$arm_item" "$output_item"
		fi
	done < <(find "$x86_dir" -mindepth 1 -maxdepth 1 -print0 2>/dev/null)
}

# 验证 Universal App 架构
verify_universal_app() {
	local app_path="$1"

	log_info "验证 Universal App 架构..."
	local verify_result=0
	while IFS= read -r -d '' file; do
		if is_macho_file "$file"; then
			local arches=$(get_file_architectures "$file" 2>/dev/null || echo "未知")
			if echo "$arches" | grep -q "x86_64" && echo "$arches" | grep -q "arm64"; then
				log_info "✓ $(basename "$file"): $arches"
			else
				log_warn "⚠ $(basename "$file"): $arches"
				verify_result=1
			fi
		fi
	done < <(find "$app_path" -type f -print0)

	return $verify_result
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

	process_directory_for_merge "$x86_app" "$arm_app" "$output_app"

	# 设置基本文件权限
	log_info "设置文件权限..."
	find "$output_app" -type f -exec chmod 644 {} \;
	find "$output_app" -type d -exec chmod 755 {} \;

	# 设置执行权限
	set_executable_permissions "$output_app"

	log_info "✓ Universal App 创建完成: $output_app"

	# 验证架构
	if verify_universal_app "$output_app"; then
		log_info "✓ 所有二进制文件都已成功合并为 Universal"
	else
		log_warn "⚠ 部分二进制文件可能不是 Universal 架构"
	fi

	# 验证权限（使用公共函数）
	verify_permissions "$output_app" "true"

	log_info "🎉 合并完成！Universal App 已创建: $output_app"
}

# 运行主函数
main "$@"
