#!/bin/bash -ex

# 配置变量
BUILD_TYPE="Release"
BUILD_DIR="build"
VERSION="v4.4.0"
TARGET_DIR="libtiff"
INSTALL_PREFIX="/usr/local"
LIB_NAME="libtiff.so.5"

cd "$(dirname "$0")"
cd ../..
project_dir="$(pwd)"
echo "Current directory: ${project_dir}"

# 日志函数 (无颜色，适合 CI 环境)
log_info() {
	echo "[INFO] $1"
}

log_warn() {
	echo "[WARN] $1"
}

log_error() {
	echo "[ERROR] $1"
}

# 检查 libtiff.so.5 是否已存在
check_libtiff_exists() {
	log_info "检查系统中是否已存在 $LIB_NAME..."

	# 简单直接的查找方法
	if find /usr -name "$LIB_NAME" 2>/dev/null | grep -q .; then
		local lib_path=$(find /usr -name "$LIB_NAME" 2>/dev/null | head -1)
		log_info "找到 $LIB_NAME: $lib_path"
		return 0
	fi

	log_info "未找到 $LIB_NAME，需要安装"
	return 1
}

# 检查命令是否存在
check_command() {
	if ! command -v "$1" &>/dev/null; then
		log_error "命令 $1 未找到，请先安装"
		return 1
	fi
}

# 安装依赖
install_dependencies() {
	log_info "更新软件包列表..."
	sudo apt update

	log_info "安装编译依赖..."
	sudo apt install -y \
		libjpeg8-dev \
		libjbig-dev \
		liblzma-dev \
		zlib1g-dev \
		libdeflate-dev \
		libzstd-dev \
		libwebp-dev \
		liblerc-dev \
		freeglut3-dev \
		libglu1-mesa-dev \
		build-essential
}

# 编译安装 libtiff
build_libtiff() {
	# 清理之前的构建
	if [ -d "$TARGET_DIR" ]; then
		log_warn "检测到之前的构建目录 $TARGET_DIR，正在清理..."
		rm -rf "$TARGET_DIR"
	fi

	if [ -d "$BUILD_DIR" ]; then
		log_warn "检测到之前的构建目录 $BUILD_DIR，正在清理..."
		rm -rf "$BUILD_DIR"
	fi

	log_info "克隆 libtiff 源码 (版本: $VERSION) 到目录: $TARGET_DIR..."
	git clone --branch "$VERSION" --depth 1 https://gitlab.com/libtiff/libtiff.git "$TARGET_DIR"

	cd "$TARGET_DIR"

	log_info "配置 CMake (构建类型: $BUILD_TYPE, 构建目录: $BUILD_DIR)..."
	cmake \
		-S . \
		-B "$BUILD_DIR" \
		-DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
		-G Ninja \
		-DCMAKE_INSTALL_PREFIX="$INSTALL_PREFIX"

	log_info "编译 libtiff..."
	cmake --build "$BUILD_DIR" --config "$BUILD_TYPE"

	log_info "安装 libtiff..."
	sudo cmake --build "$BUILD_DIR" --config "$BUILD_TYPE" --target install

	cd ..
}

# 验证安装
verify_installation() {
	log_info "验证安装..."

	# 更新库缓存
	sudo ldconfig

	# 使用简单的方法验证
	if find /usr -name "$LIB_NAME" 2>/dev/null | grep -q .; then
		local lib_path=$(find /usr -name "$LIB_NAME" 2>/dev/null | head -1)
		log_info "$LIB_NAME 安装成功: $lib_path"
	else
		log_warn "$LIB_NAME 未找到，但可能已安装在其他位置"
	fi

	# 检查 tiffinfo 命令
	if command -v tiffinfo &>/dev/null; then
		log_info "tiffinfo 命令可用"
		tiffinfo || true
	else
		log_warn "tiffinfo 命令未找到"
	fi
}

main() {
	log_info "开始检查 libtiff 安装状态..."

	# 检查是否已安装
	if check_libtiff_exists; then
		log_info "系统中已存在 $LIB_NAME，跳过安装过程"

		# 验证 tiffinfo 是否可用
		if command -v tiffinfo &>/dev/null; then
			log_info "tiffinfo 版本信息:"
			tiffinfo --version
		fi

		exit 0
	fi

	log_info "开始安装 libtiff..."

	# 检查必要命令
	check_command git || exit 1
	check_command cmake || exit 1
	check_command ninja || exit 1

	# 执行安装流程
	install_dependencies
	build_libtiff
	verify_installation

	log_info "libtiff 安装完成!"
}

# 显示帮助信息
show_help() {
	echo "用法: $0 [选项]"
	echo "选项:"
	echo "  -h, --help     显示此帮助信息"
	echo "  -v, --version  显示脚本版本"
	echo "  -f, --force    强制重新安装，即使已存在 $LIB_NAME"
	echo ""
	echo "此脚本用于编译安装 libtiff $VERSION"
	echo "配置:"
	echo "  构建类型: $BUILD_TYPE"
	echo "  构建目录: $BUILD_DIR"
	echo "  安装前缀: $INSTALL_PREFIX"
	echo "  检测库: $LIB_NAME"
}

# 参数处理
case "${1:-}" in
-h | --help)
	show_help
	exit 0
	;;
-v | --version)
	echo "build_libtiff.sh 版本 1.2 (GitHub Actions 优化版)"
	exit 0
	;;
-f | --force)
	log_info "强制安装模式，跳过库检查"
	# 继续执行安装流程
	;;
*)
	# 默认行为：先检查库是否存在
	;;
esac

# 执行主函数
main
