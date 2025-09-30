include(../../../qmake/PlatformLibraries.pri)

QT       += core gui network widgets core5compat concurrent core-private

TEMPLATE = app

TARGET = $$PROJECT_NAME

LIBS += \ 
    -l$$replaceLibName(dump) \
    -l$$replaceLibName(extensionsystem) \
    -l$$replaceLibName(thirdparty) \
    -l$$replaceLibName(widgets) \
    -l$$replaceLibName(resource) \
    -l$$replaceLibName(utils) \
    -l$$replaceLibName(tasking) \
    -l$$replaceLibName(spinner)

include(../../../qmake/VcpkgToolchain.pri)

DESTDIR = $$RUNTIME_OUTPUT_DIRECTORY

RC_ICONS = app.ico
ICON     = app.icns

SOURCES += \
    main.cc

win32{
    src_path = $$vcpkg_path/tools/crashpad
    dest_path = $$RUNTIME_OUTPUT_DIRECTORY
    QMAKE_POST_LINK += $$QMAKE_COPY_DIR $$replace(src_path, /, \\) $$replace(dest_path, /, \\)
}

macx{
    dest_path = $$RUNTIME_OUTPUT_DIRECTORY/$$member(PROJECT_NAME, 0).app/Contents/MacOS
    QMAKE_POST_LINK += $$QMAKE_COPY_FILE $$RUNTIME_OUTPUT_DIRECTORY/CrashReport $$dest_path/CrashReport
    QMAKE_POST_LINK += && $$QMAKE_COPY_DIR "$$vcpkg_path/tools/crashpad/*" "$$dest_path"
    QMAKE_POST_LINK += && chmod +x "$$dest_path/crashpad_handler"
}

unix:!macx{
    QMAKE_POST_LINK += $$QMAKE_COPY_DIR "$$vcpkg_path/tools/crashpad/*" "$$RUNTIME_OUTPUT_DIRECTORY"
    QMAKE_POST_LINK += && chmod +x "$$RUNTIME_OUTPUT_DIRECTORY/crashpad_handler"
}
