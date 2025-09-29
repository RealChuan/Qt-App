include(../../qmake/PlatformLibraries.pri)

QT = core

CONFIG += cmdline

TEMPLATE = app

TARGET = I18N

SOURCES += \
        main.cc

DESTDIR = $$RUNTIME_OUTPUT_DIRECTORY/examples
