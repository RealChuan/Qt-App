include(../lib.pri)

QT += widgets core5compat

DEFINES += EXTENSIONSYSTEM_LIBRARY
TARGET = $$replaceLibName(extensionsystem)

LIBS += \
    -l$$replaceLibName(aggregation) \
    -l$$replaceLibName(gui) \
    -l$$replaceLibName(resource) \
    -l$$replaceLibName(utils)

HEADERS += \
    extensionsystem_global.h \
    extensionsystemtr.h \
    invoker.h \
    iplugin.h \
    optionsparser.h \
    plugindetailsview.h \
    pluginerroroverview.h \
    pluginerrorview.h \
    pluginmanager.h \
    pluginmanager_p.h \
    pluginspec.h \
    pluginspec_p.h \
    pluginview.h

SOURCES += \
    invoker.cpp \
    iplugin.cpp \
    optionsparser.cpp \
    plugindetailsview.cpp \
    pluginerroroverview.cpp \
    pluginerrorview.cpp \
    pluginmanager.cpp \
    pluginspec.cpp \
    pluginview.cpp

INCLUDEPATH += $$PWD/../utils
DEPENDPATH += $$PWD/../utils
