include(../../qmake/PlatformLibraries.pri)

QT = core

CONFIG += cmdline

TEMPLATE = app

TARGET = CrashpadTest

LIBS += -l$$replaceLibName(dump)

include(../../qmake/VcpkgDeps.pri)

SOURCES += \
        main.cc

DESTDIR = $$RUNTIME_OUTPUT_DIRECTORY

macx{
    QMAKE_POST_LINK += $$QMAKE_COPY_DIR "$$vcpkg_path/tools/crashpad/*" "$$RUNTIME_OUTPUT_DIRECTORY"
}
