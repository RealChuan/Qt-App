include(../../../qmake/PlatformLibraries.pri)

QT += widgets

TARGET = $$add_app_plugin(pomodoroplugin)

LIBS += \
    -l$$replaceLibName(shell) \
    -l$$replaceLibName(extensionsystem) \
    -l$$replaceLibName(utils)

include(../../../qmake/VcpkgToolchain.pri)

SOURCES += \
    pomodoroplugin.cc \
    pomodoropage.cc

HEADERS += \
    pomodoroplugin.hpp \
    pomodoropage.hpp

DISTFILES += \
    pomodoroplugin.json
