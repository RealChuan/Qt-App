#!/bin/sh
# postinst script for qt-app.
#
# See: dh_installdeb(1).

set -e

APP_NAME="Qt-App"
APP_DIR="/opt/${APP_NAME}"
DESK="/usr/share/applications/${APP_NAME}.desktop"

case "$1" in
    configure)
        # 文件由 deb 包提供，这里只修权限，因此每步先判断存在。
        [ -f "${APP_DIR}/${APP_NAME}" ]    && chmod 0755 "${APP_DIR}/${APP_NAME}"
        [ -f "${APP_DIR}/${APP_NAME}.sh" ] && chmod 0755 "${APP_DIR}/${APP_NAME}.sh"
        [ -f "${APP_DIR}/app.png" ]        && chmod 0644 "${APP_DIR}/app.png"
        [ -f "${DESK}" ]                   && chmod 0644 "${DESK}"

        if command -v update-mime-database >/dev/null 2>&1; then
            update-mime-database /usr/share/mime || true
        fi

        if command -v update-desktop-database >/dev/null 2>&1; then
            update-desktop-database -q /usr/share/applications || true
        fi
        ;;

    abort-upgrade|abort-remove|abort-deconfigure)
        ;;

    *)
        echo "postinst called with unknown argument '$1'" >&2
        exit 1
        ;;
esac

#DEBHELPER#

exit 0
