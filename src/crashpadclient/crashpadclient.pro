include(../../qmake/PlatformLibraries.pri)

TARGET = $$add_platform_library(crashpadclient)

LIBS += \
    -l$$replaceLibName(utils)

include(../../qmake/VcpkgToolchain.pri)

SOURCES += \
    crashpad.cc

HEADERS += \
    crashpad.hpp \
    crashpadclient_global.hpp
