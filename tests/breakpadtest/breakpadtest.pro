include(../../common.pri)

QT = core widgets core5compat

CONFIG += cmdline

TARGET = BreakpadTest

LIBS += \
    -l$$replaceLibName(dump) \
    -l$$replaceLibName(utils)

include(../../src/3rdparty/3rdparty.pri)

SOURCES += \
        main.cc

DESTDIR = $$APP_OUTPUT_PATH
