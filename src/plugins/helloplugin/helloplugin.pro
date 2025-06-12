include(../plugins.pri)
include(../../3rdparty/3rdparty.pri)

QT += widgets network core5compat

DEFINES += HELLOPLUGIN_LIBRARY
TARGET = $$replaceLibName(helloplugin)

LIBS += \
    -l$$replaceLibName(core) \
    -l$$replaceLibName(extensionsystem) \
    -l$$replaceLibName(utils)

SOURCES += \
    helloplugin.cc \
    hellowidget.cc

HEADERS += \
    hellowidget.hpp

DISTFILES += \
    helloplugin.json
