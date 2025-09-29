include(../../../qmake/PlatformLibraries.pri)

QT += network concurrent

DEFINES += TASKING_LIBRARY
TARGET = $$add_platform_library(tasking)

HEADERS += \
    barrier.h \
    concurrentcall.h \
    conditional.h \
    networkquery.h \
    qprocesstask.h \
    tasking_global.h \
    tasktree.h \
    tasktreerunner.h \
    tcpsocket.h

SOURCES += \
    barrier.cpp \
    conditional.cpp \
    networkquery.cpp \
    qprocesstask.cpp \
    tasktree.cpp \
    tasktreerunner.cpp \
    tcpsocket.cpp