include(../../qmake/PlatformLibraries.pri)
include(qtsingleapplication/qtsingleapplication.pri)

DEFINES += THRIDPARTY_LIBRARY
TARGET = $$add_platform_library(thirdparty)

LIBS += \
    -l$$replaceLibName(utils)

HEADERS += \
    thirdparty_global.hpp
