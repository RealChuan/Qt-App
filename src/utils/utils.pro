include(../slib.pri)
include(../3rdparty/3rdparty.pri)
include(mimetypes2/mimetypes.pri)
include(fsengine/fsengine.pri)
include(theme/theme.pri)
include(tooltip/tooltip.pri)

QT += widgets core5compat concurrent network core-private

DEFINES += UTILS_LIBRARY
TARGET = $$replaceLibName(utils)

LIBS += \
    -l$$replaceLibName(spinner) \
    -l$$replaceLibName(tasking)

win32 {
    LIBS += \
        -ldbghelp \
        -luser32 \
        -lshell32
}

SOURCES += \
    appinfo.cpp \
    async.cpp \
    benchmarker.cpp \
    camelcasecursor.cpp \
    categorysortfiltermodel.cpp \
    commandline.cpp \
    completinglineedit.cpp \
    devicefileaccess.cpp \
    elidinglabel.cpp \
    environment.cpp \
    execmenu.cpp \
    faketooltip.cpp \
    fancylineedit.cpp \
    filepath.cpp \
    filestreamer.cpp \
    filestreamermanager.cpp \
    fileutils.cpp \
    futuresynchronizer.cpp \
    guard.cpp \
    guiutils.cpp \
    historycompleter.cpp \
    hostosinfo.cpp \
    icon.cpp \
    icondisplay.cpp \
    id.cpp \
    infolabel.cpp \
    itemviews.cpp \
    languagemanager.cc \
    layoutbuilder.cpp \
    logasync.cpp \
    logfile.cc \
    macroexpander.cpp \
    multitextcursor.cpp \
    namevaluedictionary.cpp \
    namevalueitem.cpp \
    processhelper.cpp \
    processinterface.cpp \
    processreaper.cpp \
    qtcassert.cpp \
    qtcprocess.cpp \
    qtcsettings.cpp \
    result.cpp \
    savefile.cpp \
    shutdownguard.cpp \
    singletonmanager.cc \
    store.cpp \
    stringtable.cpp \
    stringutils.cpp \
    stylehelper.cpp \
    textcodec.cpp \
    threadutils.cpp \
    treemodel.cpp \
    utils.cpp \
    utilsicons.cpp \
    validator.cc \
    widgetmanager.cc

HEADERS += \
    algorithm.h \
    appdata.hpp \
    appinfo.h \
    async.h \
    benchmarker.h \
    builderutils.h \
    camelcasecursor.h \
    categorysortfiltermodel.h \
    commandline.h \
    completinglineedit.h \
    devicefileaccess.h \
    elidinglabel.h \
    environment.h \
    environmentfwd.h \
    execmenu.h \
    expected.h \
    faketooltip.h \
    fancylineedit.h \
    filepath.h \
    filepathinfo.h \
    filestreamer.h \
    filestreamermanager.h \
    fileutils.h \
    futuresynchronizer.h \
    guard.h \
    guiutils.h \
    historycompleter.h \
    hostosinfo.h \
    icon.h \
    icondisplay.h \
    id.h \
    indexedcontainerproxyconstiterator.h \
    infolabel.h \
    itemviews.h \
    languagemanager.hpp \
    layoutbuilder.h \
    logasync.h \
    logfile.hpp \
    macroexpander.h \
    mimeutils.h \
    multitextcursor.h \
    namevaluedictionary.h \
    namevalueitem.h \
    osspecificaspects.h \
    predicates.h \
    processenums.h \
    processhelper.h \
    processinterface.h \
    processreaper.h \
    qtcassert.h \
    qtcprocess.h \
    qtcsettings.h \
    result.h \
    savefile.h \
    shutdownguard.h \
    singleton.hpp \
    singletonmanager.hpp \
    store.h \
    storekey.h \
    stringtable.h \
    stringutils.h \
    stylehelper.h \
    textcodec.h \
    threadutils.h \
    treemodel.h \
    utils_global.h \
    utils.h \
    utilsicons.h \
    utilstr.h \
    utiltypes.h \
    validator.hpp \
    widgetmanager.hpp

RESOURCES += \
    utils.qrc

macx {
    SOURCES += fileutils_mac.mm
    HEADERS += fileutils_mac.h
}