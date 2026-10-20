include(../../qmake/PlatformLibraries.pri)

TARGET = $$add_platform_library(dump)

LIBS += \
    -l$$replaceLibName(utils)

include(../../qmake/VcpkgToolchain.pri)

SOURCES += \
    crashpad.cc

HEADERS += \
    crashpad.hpp \
    dump_global.hpp
