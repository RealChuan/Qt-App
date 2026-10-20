include(../../../qmake/PlatformLibraries.pri)

QT += widgets concurrent

TARGET = $$add_app_plugin(fractalplugin)

LIBS += \
    -l$$replaceLibName(shell) \
    -l$$replaceLibName(extensionsystem) \
    -l$$replaceLibName(utils)

include(../../../qmake/VcpkgToolchain.pri)

SOURCES += \
    fractalplugin.cc \
    fractalwidget.cc

HEADERS += \
    fractalplugin.hpp \
    fractalwidget.hpp

DISTFILES += \
    fractalplugin.json
