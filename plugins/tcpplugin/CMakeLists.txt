set(PROJECT_SOURCES
    tcpclient.cpp
    tcpclient.h
    tcpplugin.cc
    tcpplugin.hpp
    tcpserver.cpp
    tcpserver.h
    tcpwidget.cpp
    tcpwidget.h)

add_custom_plugin(tcpplugin ${PROJECT_SOURCES})
target_link_libraries(
  tcpplugin
  PRIVATE core
          extensionsystem
          gui
          resource
          utils
          Qt6::Network
          Qt6::Core5Compat
          Qt6::Widgets)
