#!/bin/bash
set -euo pipefail

# ----------- 硬编码区 ---------------
APPLE_ID="***@***"
TEAM_ID="*********"
NOTARY_PWD="*********"
# ------------------------------------

# ----------- 签名配置（当前未启用，留空即代表跳过） ---------------
# 启用步骤：
#   0. 证书在带口令的钥匙串里时，填 KEYCHAIN_PATH / KEYCHAIN_PASSWORD（sign_* 会自动解锁）
#   1. 把 DEVELOPER_ID_APPLICATION / DEVELOPER_ID_INSTALLER 填上真实证书名
#   2. 取消 package.sh 里对应 sign_* 调用处的注释
#   3. （可选）开启公证：ENABLE_NOTARIZATION=true，并确认 APPLE_ID / TEAM_ID / NOTARY_PWD 有效
#
# 证书类型：
#   - .app / .dmg / 内部二进制 → Developer ID Application
#   - .pkg                     → Developer ID Installer（是另一张证书！）
DEVELOPER_ID_APPLICATION=""   # 例: "Developer ID Application: The Youth. (XXXXXXXXXX)"
DEVELOPER_ID_INSTALLER=""     # 例: "Developer ID Installer: The Youth. (XXXXXXXXXX)"
ENTITLEMENTS=""               # 可选，hardened runtime 所需 entitlements.plist 的路径
ENABLE_NOTARIZATION=false     # 是否对 dmg / pkg 做公证 + staple
KEYCHAIN_PATH="${HOME}/Library/Keychains/login.keychain-db"  # CI 上一般是临时钥匙串
KEYCHAIN_PASSWORD=""          # 为空则跳过解锁（假定钥匙串已手动解锁）
# ------------------------------------------------------------------

# 本文件所在目录 / packaging 根目录：在 source 时解析一次。
# 调用方稍后会 cd 到别处，且 BASH_SOURCE 可能是相对路径（source utils.sh），
# 所以不能等到函数执行时再算。
UTILS_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PACKAGING_DIR="$(cd "${UTILS_DIR}/.." && pwd)"

log() { echo "[$(date +%F\ %T)] $*"; }
die() {
	log "FATAL: $*"
	exit 1
}

# 检查命令是否存在
check_tool() {
	command -v "$1" >/dev/null 2>&1 || die "缺少命令: $1"
}

# 字体下载（替代原 fonts.cmake，7z 走 PATH）
#   macOS 下 FONT_DIR 与 CMake 的 CMAKE_HOST_APPLE 分支一致：
#   Qt-App.app/Contents/Resources/fonts
download_fonts() {
	local dest="$1" url="${2:-}"
	local url_file="${PACKAGING_DIR}/fonts.url"
	if [ -z "$url" ]; then
		[ -f "$url_file" ] || die "缺少字体地址文件: $url_file"
		url="$(tr -d '[:space:]' <"$url_file")"
	fi
	local archive
	archive=$(mktemp -t fonts.XXXXXX.7z)

	log "下载字体: $url"
	curl -fsSL -o "$archive" "$url" || die "字体下载失败"

	mkdir -p "$dest"
	log "解压字体 -> $dest"
	7z x "$archive" "-o$dest" -y >/dev/null || die "字体解压失败"
	rm -f "$archive"

	local count
	count=$(find "$dest" -type f | wc -l | tr -d ' ')
	[ "$count" -gt 0 ] || die "字体解压后为空"
	log "字体下载完成，共 $count 个文件"
}

# 处理 plist
process_plist() {
	local plist=$1
	[[ -n $plist && -f $plist ]] || {
		log "Skip plist: file missing"
		return 0
	}
	log "Convert plist → XML"
	plutil -convert xml1 "$plist"
	log "Lint plist"
	plutil -lint "$plist" >/dev/null || die "Plist lint failed"
	log "Plist processed OK"
}

# ============================================================
# 签名
# ============================================================

# 解锁钥匙串：CI 上证书通常导入临时钥匙串并设了口令，
# 不先解锁的话 codesign / productsign 会卡在密码框或直接失败。
unlock_keychain() {
	[ -n "$KEYCHAIN_PATH" ] || die "unlock_keychain: KEYCHAIN_PATH 未配置"
	[ -f "$KEYCHAIN_PATH" ] || die "unlock_keychain: 钥匙串不存在: $KEYCHAIN_PATH"

	if [ -z "$KEYCHAIN_PASSWORD" ]; then
		log "KEYCHAIN_PASSWORD 未配置，跳过钥匙串解锁（假定已手动解锁）"
		return 0
	fi

	log "解锁钥匙串: $KEYCHAIN_PATH"
	security unlock-keychain -p "$KEYCHAIN_PASSWORD" "$KEYCHAIN_PATH"
	# 允许 codesign / productsign 无人值守取用私钥（首次执行会要求确认一次）
	# security set-key-partition-list -S apple-tool:,apple: -s -k "$KEYCHAIN_PASSWORD" "$KEYCHAIN_PATH"
	log "✓ 钥匙串已解锁"
}

# 签名 .app 及其内部所有二进制（framework / dylib / 主可执行文件）
#   采用 --deep：对当前项目结构（Qt 框架 + 应用插件）足够；将来若需
#   精细控制嵌套顺序，可拆成"从内到外逐个 sign"再签外层。
sign_app() {
	local app="$1"
	[ -n "$DEVELOPER_ID_APPLICATION" ] || die "sign_app: DEVELOPER_ID_APPLICATION 未配置"
	[ -d "$app" ] || die "sign_app: .app 不存在: $app"
	unlock_keychain

	log "签名 .app: $app"
	local args=(
		--force
		--options runtime
		--timestamp
		--sign "$DEVELOPER_ID_APPLICATION"
	)
	if [ -n "$ENTITLEMENTS" ] && [ -f "$ENTITLEMENTS" ]; then
		args+=(--entitlements "$ENTITLEMENTS")
	fi

	codesign "${args[@]}" --deep "$app"

	log "验证 .app 签名..."
	codesign --verify --deep --strict --verbose=2 "$app"
	log "✓ .app 签名完成"
}

# 签名 .dmg（对最终分发文件单独签名）
sign_dmg() {
	local dmg="$1"
	[ -n "$DEVELOPER_ID_APPLICATION" ] || die "sign_dmg: DEVELOPER_ID_APPLICATION 未配置"
	[ -f "$dmg" ] || die "sign_dmg: dmg 不存在: $dmg"
	unlock_keychain

	log "签名 dmg: $dmg"
	codesign --force --sign "$DEVELOPER_ID_APPLICATION" --timestamp "$dmg"
	codesign --verify --verbose=2 "$dmg"
	log "✓ dmg 签名完成"
}

# 签名 .pkg（需要 Developer ID Installer 证书，与 Application 是两张证书）
sign_pkg() {
	local pkg="$1"
	[ -n "$DEVELOPER_ID_INSTALLER" ] || die "sign_pkg: DEVELOPER_ID_INSTALLER 未配置"
	[ -f "$pkg" ] || die "sign_pkg: pkg 不存在: $pkg"
	unlock_keychain

	log "签名 pkg: $pkg"
	local signed="${pkg}.signed"
	productsign --sign "$DEVELOPER_ID_INSTALLER" --timestamp "$pkg" "$signed"
	mv "$signed" "$pkg"
	log "✓ pkg 签名完成"
}

# ============================================================
# 公证（仅 dmg / pkg 有意义；.app 需先打进 dmg/pkg 再公证）
#   - 调用前文件必须已签名，否则 notarytool 会拒绝
#   - ENABLE_NOTARIZATION=false 时静默跳过，不报错
# ============================================================
notarize_app() {
	local target=$1
	if [ "$ENABLE_NOTARIZATION" != "true" ]; then
		return 0
	fi
	[[ -f $target ]] || die "Notarize target missing: $target"

	log "提交公证: $target"
	xcrun notarytool submit "$target" \
		--apple-id "$APPLE_ID" \
		--team-id "$TEAM_ID" \
		--password "$NOTARY_PWD" \
		--wait

	log "Staple: $target"
	xcrun stapler staple "$target"
	log "✓ 公证 + staple 完成: $target"
}
