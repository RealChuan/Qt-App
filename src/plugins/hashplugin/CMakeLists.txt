set(PROJECT_SOURCES
    cpubenchthread.cc
    cpubenchthread.hpp
    hashplugin.cc
    hashthread.cc
    hashthread.hpp
    hashwidget.cc
    hashwidget.hpp)

add_custom_plugin(hashplugin ${PROJECT_SOURCES})
target_link_libraries(
  hashplugin
  PRIVATE core
          extensionsystem
          widgets
          resource
          utils
          Qt::Widgets
          tl::expected)
