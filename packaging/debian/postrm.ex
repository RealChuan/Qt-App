#!/bin/sh
# postrm script for qt-app.
#
# See: dh_installdeb(1).

set -e

APP_NAME="Qt-App"
DESK="/usr/share/applications/${APP_NAME}.desktop"

# 遍历真实用户家目录，删除用户级残留。
# dpkg 以 root 执行，~ 和 ${HOME} 都指向 /root，不能直接用。
remove_user_files() {
    home="$1"
    [ -d "$home" ] || return 0

    # 用户级 desktop 快捷方式（用户可能手动复制过）
    rm -f "$home/.local/share/applications/${APP_NAME}.desktop""

    # 用户配置
    rm -rf "$home/.config/Youth/${APP_NAME}"
    rm -rf "$home/.config/Youth/CrashReport"

    return 0
}

case "$1" in
    remove)
        # 卸载：只删系统文件，保留用户配置。
        rm -f "${DESK}"

        if command -v update-mime-database >/dev/null 2>&1; then
            update-mime-database /usr/share/mime || true
        fi
        if command -v update-desktop-database >/dev/null 2>&1; then
            update-desktop-database -q /usr/share/applications || true
        fi
        ;;

    purge)
        # 彻底清除：系统文件 + 用户文件。
        rm -f "${DESK}"

        for home in /home/* /root; do
            [ "$home" = "/home/*" ] && continue   # glob 未展开
            remove_user_files "$home"
        done

        if command -v update-mime-database >/dev/null 2>&1; then
            update-mime-database /usr/share/mime || true
        fi
        if command -v update-desktop-database >/dev/null 2>&1; then
            update-desktop-database -q /usr/share/applications || true
        fi
        ;;

    upgrade|failed-upgrade|abort-install|abort-upgrade|disappear)
        ;;

    *)
        echo "postrm called with unknown argument '$1'" >&2
        exit 1
        ;;
esac

#DEBHELPER#

exit 0
