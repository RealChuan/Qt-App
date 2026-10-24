include(../../qmake/PlatformLibraries.pri)

QT += widgets

# 导出宏由 add_platform_library() 统一写入（UTILS_LIBRARY / UTILS_STATIC_LIBRARY）
TARGET = $$add_platform_library(utils)

include(../../qmake/VcpkgToolchain.pri)

win32 {
    LIBS += \
        -ldbghelp \
        -luser32 \
        -lshell32
}

SOURCES += \
    appinfo.cc \
    asynclog.cc \
    filepath.cc \
    hostosinfo.cc \
    languagemanager.cc \
    logbuffer.cc \
    logsink.cc \
    qtcassert.cc \
    result.cc \
    settings.cc \
    singletonmanager.cc \
    theme.cc \
    uihelpers.cc \
    utils.cc \
    widgetmanager.cc

HEADERS += \
    algorithm.hpp \
    appinfo.hpp \
    asynclog.hpp \
    filepath.hpp \
    hostosinfo.hpp \
    languagemanager.hpp \
    logbuffer.hpp \
    logsink.hpp \
    osspecificaspects.hpp \
    predicates.hpp \
    qtcassert.hpp \
    result.hpp \
    settings.hpp \
    singleton.hpp \
    singletonmanager.hpp \
    theme.hpp \
    uihelpers.hpp \
    utils_global.hpp \
    utils.hpp \
    utilstr.hpp \
    widgetmanager.hpp
