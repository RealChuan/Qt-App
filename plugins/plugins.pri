include(../common.pri)

TEMPLATE = lib

win32 {
DESTDIR = $$APP_OUTPUT_PATH/../libs
DLLDESTDIR = $$APP_OUTPUT_PATH/plugins
LIBS += -L$$DESTDIR
}

unix {
DESTDIR = $$APP_OUTPUT_PATH/plugins
LIBS += -L$$APP_OUTPUT_PATH
}
