set(PROJECT_SOURCES
    configwidget.cpp
    configwidget.h
    coreplugin.cpp
    coreplugin.h
    coreplugintr.h
    mainwindow.cpp
    mainwindow.h
    plugindialog.cpp
    plugindialog.h)

add_custom_plugin(coreplugin ${PROJECT_SOURCES})
target_link_libraries(coreplugin PRIVATE core extensionsystem gui resource
                                         utils Qt6::Widgets)
