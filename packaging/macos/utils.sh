#!/bin/bash
set -euo pipefail
# ----------- 硬编码区 ---------------
APPLE_ID="***@***"
TEAM_ID="*********"
NOTARY_PWD="*********"
# ------------------------------------

log() { echo "[$(date +%F\ %T)] $*"; }
die() {
	log "FATAL: $*"
	exit 1
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

# 公证 + staple
notarize_app() {
	local target=$1
	[[ -f $target ]] || die "Notarize target missing: $target"

	log "Submitting for notarization..."
	xcrun notarytool submit "$target" \
		--apple-id "$APPLE_ID" \
		--team-id "$TEAM_ID" \
		--password "$NOTARY_PWD" \
		--wait

	log "Stapling ticket..."
	xcrun stapler staple "$target"

	log "Notarize & staple finished: $target"
}
