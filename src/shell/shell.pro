include(../../qmake/PlatformLibraries.pri)

QT += widgets

TARGET = $$add_shared_library(shell)

LIBS += \
    -l$$replaceLibName(extensionsystem) \
    -l$$replaceLibName(utils)

include(../../qmake/VcpkgToolchain.pri)

HEADERS += \
    shell_global.hpp \
    page.hpp \
    command.hpp \
    functioncommand.hpp \
    registry.hpp \
    host.hpp

SOURCES += \
    functioncommand.cc \
    registry.cc \
    host.cc
