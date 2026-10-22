include(../../../qmake/PlatformLibraries.pri)

QT += widgets

TARGET = $$add_app_plugin(helloplugin)

LIBS += \
    -l$$replaceLibName(shell) \
    -l$$replaceLibName(extensionsystem) \
    -l$$replaceLibName(utils)

include(../../../qmake/VcpkgToolchain.pri)

SOURCES += \
    helloplugin.cc \
    hellopage.cc

HEADERS += \
    helloplugin.hpp \
    hellopage.hpp

DISTFILES += \
    helloplugin.json
