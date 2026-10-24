include(../../../qmake/PlatformLibraries.pri)

QT += widgets

TARGET = $$add_app_plugin(snakeplugin)

LIBS += \
    -l$$replaceLibName(shell) \
    -l$$replaceLibName(extensionsystem) \
    -l$$replaceLibName(utils)

include(../../../qmake/VcpkgToolchain.pri)

SOURCES += \
    snakeplugin.cc \
    snakepage.cc \
    snakeboard.cc \
    snakegame.cc

HEADERS += \
    snakeplugin.hpp \
    snakepage.hpp \
    snakeboard.hpp \
    snakegame.hpp

DISTFILES += \
    snakeplugin.json
