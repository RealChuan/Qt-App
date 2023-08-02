include(../libs.pri)
include(mimetypes/mimetypes.pri)

QT += widgets

greaterThan(QT_MAJOR_VERSION, 5): QT += core5compat

DEFINES += UTILS_LIBRARY
TARGET = $$replaceLibName(utils)

win32 {
LIBS += -ldbghelp
}

SOURCES += \
    benchmarker.cpp \
    countdownlatch.cc \
    hostosinfo.cpp \
    languageconfig.cc \
    logasync.cpp \
    qtcassert.cpp \
    qtcsettings.cpp \
    stringutils.cpp \
    utils.cpp \
    validator.cc

HEADERS += \
    algorithm.h \
    benchmarker.h \
    countdownlatch.hpp \
    executeondestruction.h \
    hostosinfo.h \
    indexedcontainerproxyconstiterator.h \
    languageconfig.hpp \
    logasync.h \
    mimeutils.h \
    osspecificaspects.h \
    predicates.h \
    qtcassert.h \
    qtcsettings.h \
    singleton.hpp \
    stringutils.h \
    taskqueue.h \
    utils_global.h \
    utils.h \
    utilstr.h \
    validator.hpp
