include(../../qmake/PlatformLibraries.pri)

QT += core

TARGET = $$add_shared_library(extensionsystem)

LIBS += \
    -l$$replaceLibName(utils)

include(../../qmake/VcpkgToolchain.pri)

HEADERS += \
    extensionsystem_global.hpp \
    extensionsystemtr.hpp \
    iplugin.hpp \
    plugincontext.hpp \
    pluginmanager.hpp \
    pluginspec.hpp

SOURCES += \
    iplugin.cc \
    plugincontext.cc \
    pluginmanager.cc \
    pluginspec.cc
