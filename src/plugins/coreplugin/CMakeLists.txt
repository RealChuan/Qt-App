set(PROJECT_SOURCES
    aboutpage.hpp
    aboutpage.cc
    appearancepage.hpp
    appearancepage.cc
    commandpalette.hpp
    commandpalette.cc
    coreplugin.hpp
    coreplugin.cc
    mainwindow.hpp
    mainwindow.cc
    plugindetailsview.hpp
    plugindetailsview.cc
    pluginpage.hpp
    pluginpage.cc
    pluginview.hpp
    pluginview.cc)

add_app_plugin(coreplugin ${PROJECT_SOURCES})
target_link_libraries(
  coreplugin
  PRIVATE shell
          extensionsystem
          widgets
          resource
          utils
          Qt::Widgets)
