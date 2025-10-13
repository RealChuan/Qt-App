include(../../qmake/PlatformLibraries.pri)

QT = core

CONFIG += cmdline

TEMPLATE = app

TARGET = CrashpadTest

LIBS += -l$$replaceLibName(dump)

include(../../qmake/InstallCrashpad.pri)

SOURCES += \
        main.cc

DESTDIR = $$RUNTIME_OUTPUT_DIRECTORY

QMAKE_POST_LINK += $$setup_crashpad_handler($$RUNTIME_OUTPUT_DIRECTORY)
