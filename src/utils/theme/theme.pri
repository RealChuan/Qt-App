HEADERS += \
    $$PWD/theme.h \
    $$PWD/theme_p.h

SOURCES += \
    $$PWD/theme.cpp 

macx{
    HEADERS += $$PWD/theme_mac.h 
    SOURCES += $$PWD/theme_mac.mm
}