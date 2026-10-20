set(PROJECT_SOURCES
    aboutpage.hpp
    aboutpage.cc
    commandpalette.hpp
    commandpalette.cc
    configpage.hpp
    configpage.cc
    coreplugin.hpp
    coreplugin.cc
    mainwindow.hpp
    mainwindow.cc
    pluginpage.hpp
    pluginpage.cc)

add_app_plugin(coreplugin ${PROJECT_SOURCES})
target_link_libraries(
  coreplugin
  PRIVATE shell
          extensionsystem
          widgets
          resource
          utils
          Qt::Widgets)
