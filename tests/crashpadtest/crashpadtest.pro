include(../../common.pri)

QT = core

CONFIG += cmdline

TARGET = CrashpadTest

LIBS += -l$$replaceLibName(dump)

include(../../src/3rdparty/3rdparty.pri)

SOURCES += \
        main.cc

DESTDIR = $$APP_OUTPUT_PATH

macx{
    QMAKE_POST_LINK += $$QMAKE_COPY_DIR "$$vcpkg_path/tools/crashpad/" "$$APP_OUTPUT_PATH"
}
