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
    appinfo.cpp \
    asynclog.cc \
    filepath.cpp \
    hostosinfo.cpp \
    languagemanager.cc \
    logbuffer.cc \
    logsink.cc \
    qtcassert.cpp \
    result.cpp \
    settings.cpp \
    singletonmanager.cc \
    utils.cc \
    widgetmanager.cc

HEADERS += \
    algorithm.h \
    appdata.hpp \
    appinfo.h \
    asynclog.hpp \
    filepath.h \
    hostosinfo.h \
    languagemanager.hpp \
    logbuffer.hpp \
    logsink.hpp \
    osspecificaspects.h \
    predicates.h \
    qtcassert.h \
    result.h \
    settings.h \
    singleton.hpp \
    singletonmanager.hpp \
    utils_global.h \
    utils.hpp \
    utilstr.h \
    widgetmanager.hpp
