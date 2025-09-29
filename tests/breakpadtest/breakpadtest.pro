include(../../qmake/PlatformLibraries.pri)

QT = core widgets core5compat

CONFIG += cmdline

TEMPLATE = app

TARGET = BreakpadTest

LIBS += \
    -l$$replaceLibName(dump) \
    -l$$replaceLibName(utils)

include(../../qmake/VcpkgDeps.pri)

SOURCES += \
        main.cc

DESTDIR = $$RUNTIME_OUTPUT_DIRECTORY
