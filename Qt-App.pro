TEMPLATE = subdirs
CONFIG += ordered

SUBDIRS += \
    src \
    examples

TRANSLATIONS += \
    translations/qt-app_en.ts \
    translations/qt-app_zh_CN.ts

DISTFILES += \
    doc/** \
    .clang* \
    LICENSE \
    README*
