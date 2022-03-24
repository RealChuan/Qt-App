include(../../libs.pri)
include(../../3rdparty/3rdparty.pri)

QT += widgets

DEFINES += CRASHHANDLER_LIBRARY
TARGET = $$replaceLibName(crashhandler)

LIBS += \
	-l$$replaceLibName(utils) \
        -l$$replaceLibName(controls)
	
SOURCES += \
    breakpad.cc \
    crashdialog.cpp \
    crashhandler.cpp

HEADERS += \
    breakpad.hpp \
    crashHandler_global.h \
    crashdialog.h \
    crashhandler.h

QMAKE_TARGET_PRODUCT = Crash Handler
QMAKE_TARGET_DESCRIPTION = Crash Tracking
