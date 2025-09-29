include(../../qmake/PlatformLibraries.pri)

QT += widgets

DEFINES += RESOURCE_LIBRARY
TARGET = $$add_platform_library(resource)

SOURCES += \
    resource.cc

HEADERS += \
    resource_global.hpp \
    resource.hpp

RESOURCES += \
    resource.qrc

win32{
    src_path = $$PWD/themes
    dest_path = $$RUNTIME_OUTPUT_DIRECTORY/resources/themes
    QMAKE_POST_LINK += $$QMAKE_COPY_DIR $$replace(src_path, /, \\) $$replace(dest_path, /, \\)
}

macx{
    dest_path = $$RUNTIME_OUTPUT_DIRECTORY/$$member(PROJECT_NAME, 0).app/Contents/Resources
    QMAKE_POST_LINK += mkdir -p $$dest_path
    QMAKE_POST_LINK += && $$QMAKE_COPY_DIR "$$PWD/themes" "$$dest_path"
}

unix:!macx{
    dest_path = $$RUNTIME_OUTPUT_DIRECTORY/resources
    QMAKE_POST_LINK += mkdir -p "$$dest_path"
    QMAKE_POST_LINK += && $$QMAKE_COPY_DIR "$$PWD/themes" "$$dest_path"
}
