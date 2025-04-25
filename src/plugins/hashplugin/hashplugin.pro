include(../plugins.pri)

QT += widgets core5compat

DEFINES += HASHPLUGIN_LIBRARY
TARGET = $$replaceLibName(hashplugin)

LIBS += \
    -l$$replaceLibName(core) \
    -l$$replaceLibName(extensionsystem) \
    -l$$replaceLibName(gui) \
    -l$$replaceLibName(resource) \
    -l$$replaceLibName(utils)

HEADERS += \
    cpubenchthread.hpp \
    hashplugin.hpp \
    hashthread.hpp \
    hashwidget.hpp

SOURCES += \
    cpubenchthread.cc \
    hashplugin.cc \
    hashthread.cc \
    hashwidget.cc

DISTFILES += \
    hashplugin.json
