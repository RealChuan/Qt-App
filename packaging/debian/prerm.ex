#!/bin/sh
# prerm script for qt-app.
#
# See: dh_installdeb(1).

set -e

APP_NAME="Qt-App"

stop_process() {
    name="$1"

    # 先 SIGTERM 给程序清理机会，最多等 3 秒，再 SIGKILL。
    if command -v pkill >/dev/null 2>&1; then
        pkill -TERM -x "$name" 2>/dev/null || true
    fi

    i=0
    while [ "$i" -lt 3 ]; do
        if ! pgrep -x "$name" >/dev/null 2>&1; then
            return 0
        fi
        sleep 1
        i=$((i + 1))
    done

    if command -v pkill >/dev/null 2>&1; then
        pkill -KILL -x "$name" 2>/dev/null || true
    fi

    return 0
}

case "$1" in
    remove|upgrade|deconfigure)
        stop_process "${APP_NAME}"
        stop_process "CrashReport"
        ;;

    failed-upgrade)
        # 升级失败时 dpkg 会重调 prerm 恢复旧版本，
        # 此时不应杀进程，避免干扰恢复流程。
        ;;

    *)
        echo "prerm called with unknown argument '$1'" >&2
        exit 1
        ;;
esac

#DEBHELPER#

exit 0
