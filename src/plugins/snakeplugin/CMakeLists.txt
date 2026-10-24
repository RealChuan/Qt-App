set(PROJECT_SOURCES
    snakeplugin.hpp
    snakeplugin.cc
    snakepage.hpp
    snakepage.cc
    snakeboard.hpp
    snakeboard.cc
    snakegame.hpp
    snakegame.cc)

add_app_plugin(snakeplugin ${PROJECT_SOURCES})
target_link_libraries(snakeplugin PRIVATE shell extensionsystem utils Qt::Widgets)
