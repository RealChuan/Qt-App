include(../plugins.pri)

QT += widgets

DEFINES += HASHPLUGIN_LIBRARY
TARGET = $$replaceLibName(hashplugin)

LIBS += \
    -l$$replaceLibName(core) \
    -l$$replaceLibName(extensionsystem) \
    -l$$replaceLibName(gui) \
    -l$$replaceLibName(resource) \
    -l$$replaceLibName(utils)

HEADERS += \
    hashplugin.hpp \
    hashthread.hpp \
    hashwidget.hpp

SOURCES += \
    hashplugin.cc \
    hashthread.cc \
    hashwidget.cc

DISTFILES += \
    hashplugin.json
