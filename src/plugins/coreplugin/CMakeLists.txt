set(PROJECT_SOURCES
    configwidget.cpp
    configwidget.h
    coreplugin.cpp
    coreplugin.hpp
    coreplugintr.h
    icore.cpp
    icore.h
    mainwindow.cpp
    mainwindow.h
    plugindialog.cpp
    plugindialog.h)

add_platform_plugin(coreplugin ${PROJECT_SOURCES})
target_link_libraries(
  coreplugin
  PRIVATE core
          extensionsystem
          widgets
          resource
          utils
          Qt::Widgets
          tl::expected)
