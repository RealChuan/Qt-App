TEMPLATE = subdirs
CONFIG += ordered

SUBDIRS += src

TRANSLATIONS += \
    translations/qt-app_en.ts \
    translations/qt-app_zh_CN.ts

DISTFILES += \
    docs/** \
    .clang* \
    LICENSE \
    README*
