#!/bin/sh
# preinst script for qt-app.
#
# See: dh_installdeb(1).

set -e

case "$1" in
    install|upgrade)
        # 安装/升级前无需操作。
        # 停止进程放在 prerm（卸载和升级场景都覆盖）。
        ;;
    abort-upgrade)
        ;;
    *)
        echo "preinst called with unknown argument '$1'" >&2
        exit 1
        ;;
esac

#DEBHELPER#

exit 0#!/bin/sh
# preinst script for qt-app.
#
# See: dh_installdeb(1).

set -e

case "$1" in
    install|upgrade)
        # 安装/升级前无需操作。
        # 停止进程放在 prerm（卸载和升级场景都覆盖）。
        ;;
    abort-upgrade)
        ;;
    *)
        echo "preinst called with unknown argument '$1'" >&2
        exit 1
        ;;
esac

#DEBHELPER#

exit 0
