set(PROJECT_SOURCES
    helloplugin.hpp
    helloplugin.cc
    hellopage.hpp
    hellopage.cc)

add_app_plugin(helloplugin ${PROJECT_SOURCES})
target_link_libraries(helloplugin PRIVATE shell extensionsystem utils Qt::Widgets)
