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
    fractalpage.cc

HEADERS += \
    fractalplugin.hpp \
    fractalpage.hpp

DISTFILES += \
    fractalplugin.json
