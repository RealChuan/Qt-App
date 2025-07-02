include(../../../common.pri)

QT       += core gui network widgets core5compat concurrent core-private

macx {
CONFIG -= app_bundle
}

TEMPLATE = app

TARGET = CrashReport

LIBS += \
    -l$$replaceLibName(dump) \
    -l$$replaceLibName(thirdparty) \
    -l$$replaceLibName(widgets) \
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
