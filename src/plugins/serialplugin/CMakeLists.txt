set(PROJECT_SOURCES
    serialplugin.cc
    serialplugin.hpp
    serialport.cpp
    serialport.h
    serialsettings.cc
    serialsettings.hpp
    serialwidget.cpp
    serialwidget.h)

add_custom_plugin(serialplugin ${PROJECT_SOURCES})
target_link_libraries(
  serialplugin
  PRIVATE core
          extensionsystem
          gui
          resource
          utils
          Qt6::SerialPort
          Qt6::Core5Compat
          Qt6::Widgets)
