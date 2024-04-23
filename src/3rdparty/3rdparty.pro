include(../slib.pri)
include(qtsingleapplication/qtsingleapplication.pri)

DEFINES += THRIDPARTY_LIBRARY
TARGET = $$replaceLibName(thirdparty)

HEADERS += \
    thirdparty_global.hpp
