include(../../qmake/PlatformLibraries.pri)

QT += widgets core5compat network

DEFINES += EXTENSIONSYSTEM_LIBRARY
TARGET = $$add_shared_library(extensionsystem)

LIBS += \
    -l$$replaceLibName(aggregation) \
    -l$$replaceLibName(utils) \
    -l$$replaceLibName(tasking) \
    -l$$replaceLibName(spinner)

include(../../qmake/VcpkgDeps.pri)

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
