include(../../../qmake/PlatformLibraries.pri)

QT += widgets

TARGET = $$add_app_plugin(particlesplugin)

LIBS += \
    -l$$replaceLibName(shell) \
    -l$$replaceLibName(extensionsystem) \
    -l$$replaceLibName(utils)

include(../../../qmake/VcpkgToolchain.pri)

SOURCES += \
    particlesplugin.cc \
    particlespage.cc

HEADERS += \
    particlesplugin.hpp \
    particlespage.hpp

DISTFILES += \
    particlesplugin.json
