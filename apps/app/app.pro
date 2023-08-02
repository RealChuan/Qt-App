include(../../common.pri)

QT       += core gui network widgets

TEMPLATE = app

TARGET = Qt-App

win32 {
LIBS += -L$$APP_OUTPUT_PATH/../libs
}

unix {
LIBS += -L$$APP_OUTPUT_PATH
}

LIBS += \ 
    -l$$replaceLibName(extensionsystem) \
    -l$$replaceLibName(thirdparty) \
    -l$$replaceLibName(gui) \
    -l$$replaceLibName(resource) \
    -l$$replaceLibName(utils)

include(../../3rdparty/3rdparty.pri)

DESTDIR = $$APP_OUTPUT_PATH

RC_ICONS = app.ico
ICON     = app.icns

SOURCES += \
    main.cc

macx{
QMAKE_POST_LINK += $$QMAKE_COPY_DIR $$APP_OUTPUT_PATH/plugins $$APP_OUTPUT_PATH/Qt-App.app/Contents/MacOS/plugins
QMAKE_POST_LINK += & $$QMAKE_COPY_FILE $$APP_OUTPUT_PATH/CrashReport $$APP_OUTPUT_PATH/Qt-App.app/Contents/MacOS/CrashReport
}
