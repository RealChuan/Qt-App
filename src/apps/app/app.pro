include(../../../qmake/PlatformLibraries.pri)

QT       += core gui network widgets

TEMPLATE = app

TARGET = $$PROJECT_NAME

# 注意：续行符 \ 必须是该行最后一个字符，行尾有空格会让续行失效
LIBS += \
    -l$$replaceLibName(dump) \
    -l$$replaceLibName(extensionsystem) \
    -l$$replaceLibName(shell) \
    -l$$replaceLibName(thirdparty) \
    -l$$replaceLibName(widgets) \
    -l$$replaceLibName(resource) \
    -l$$replaceLibName(utils)

include(../../../qmake/InstallCrashpad.pri)

DESTDIR = $$RUNTIME_OUTPUT_DIRECTORY

RC_ICONS = app.ico
ICON     = app.icns

SOURCES += \
    main.cc

win32 | unix:!macx {
    CRASHPAD_TARGET_DIR = $$RUNTIME_OUTPUT_DIRECTORY
}

macx {
    CRASHPAD_TARGET_DIR = $$RUNTIME_OUTPUT_DIRECTORY/$$member(PROJECT_NAME, 0).app/Contents/MacOS
}

QMAKE_POST_LINK += $$setup_crashpad_handler($$CRASHPAD_TARGET_DIR)

macx {
    dest_path = $$RUNTIME_OUTPUT_DIRECTORY/$$member(PROJECT_NAME, 0).app/Contents/MacOS
    QMAKE_POST_LINK += && $$QMAKE_COPY_FILE $$RUNTIME_OUTPUT_DIRECTORY/CrashReport $$dest_path/CrashReport
}
