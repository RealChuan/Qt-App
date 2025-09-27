include(../../../common.pri)

QT       += core gui network widgets core5compat concurrent core-private

TEMPLATE = app

TARGET = Qt-App

LIBS += \ 
    -l$$replaceLibName(dump) \
    -l$$replaceLibName(extensionsystem) \
    -l$$replaceLibName(thirdparty) \
    -l$$replaceLibName(widgets) \
    -l$$replaceLibName(resource) \
    -l$$replaceLibName(utils) \
    -l$$replaceLibName(tasking) \
    -l$$replaceLibName(spinner)

include(../../3rdparty/3rdparty.pri)

DESTDIR = $$APP_OUTPUT_PATH

RC_ICONS = app.ico
ICON     = app.icns

SOURCES += \
    main.cc

win32{
    src_path = $$vcpkg_path/tools/crashpad
    dist_path = $$APP_OUTPUT_PATH
    QMAKE_POST_LINK += $$QMAKE_COPY_DIR $$replace(src_path, /, \\) $$replace(dist_path, /, \\)

    src_path = $$PWD/../../resource/themes
    dist_path = $$APP_OUTPUT_PATH/resources/themes
    QMAKE_POST_LINK += & $$QMAKE_COPY_DIR $$replace(src_path, /, \\) $$replace(dist_path, /, \\)
}

macx{
    QMAKE_POST_LINK += $$QMAKE_COPY_DIR $$APP_OUTPUT_PATH/plugins $$APP_OUTPUT_PATH/Qt-App.app/Contents/MacOS
    QMAKE_POST_LINK += && $$QMAKE_COPY_FILE $$APP_OUTPUT_PATH/CrashReport $$APP_OUTPUT_PATH/Qt-App.app/Contents/MacOS/CrashReport
    QMAKE_POST_LINK += && $$QMAKE_COPY_DIR "$$vcpkg_path/tools/crashpad/*" "$$APP_OUTPUT_PATH/Qt-App.app/Contents/MacOS"
    QMAKE_POST_LINK += && chmod +x "$$APP_OUTPUT_PATH/Qt-App.app/Contents/MacOS/crashpad_handler"

    QMAKE_POST_LINK += && $$QMAKE_COPY_DIR "$$PWD/../../resource/themes" "$$APP_OUTPUT_PATH/Qt-App.app/Contents/Resources"
}

unix:!macx{
    QMAKE_POST_LINK += $$QMAKE_COPY_DIR "$$vcpkg_path/tools/crashpad/*" "$$APP_OUTPUT_PATH"
    QMAKE_POST_LINK += && chmod +x "$$APP_OUTPUT_PATH/crashpad_handler"

    QMAKE_POST_LINK += && mkdir -p "$$APP_OUTPUT_PATH/resources"
    QMAKE_POST_LINK += && $$QMAKE_COPY_DIR "$$PWD/../../resource/themes" "$$APP_OUTPUT_PATH/resources"
}
