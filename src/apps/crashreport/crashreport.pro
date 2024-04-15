include(../../../common.pri)

QT       += core gui network widgets core5compat

macx {
CONFIG -= app_bundle
}

TEMPLATE = app

TARGET = CrashReport

win32 {
LIBS += -L$$APP_OUTPUT_PATH/../libs
}

unix {
LIBS += -L$$APP_OUTPUT_PATH
}


LIBS += \
    -l$$replaceLibName(thirdparty) \
    -l$$replaceLibName(gui) \
    -l$$replaceLibName(resource) \
    -l$$replaceLibName(utils)

include(../../3rdparty/3rdparty.pri)

DESTDIR = $$APP_OUTPUT_PATH

RC_ICONS = app.ico
ICON     = app.icns

SOURCES += \
    crashwidgets.cc \
    main.cc

HEADERS += \
    crashwidgets.hpp
