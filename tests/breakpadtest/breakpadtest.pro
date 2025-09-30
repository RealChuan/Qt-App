include(../../qmake/PlatformLibraries.pri)

QT = core widgets core5compat

CONFIG += cmdline

TEMPLATE = app

TARGET = BreakpadTest

LIBS += \
    -l$$replaceLibName(dump) \
    -l$$replaceLibName(utils)

include(../../qmake/VcpkgToolchain.pri)

SOURCES += \
        main.cc

DESTDIR = $$RUNTIME_OUTPUT_DIRECTORY
