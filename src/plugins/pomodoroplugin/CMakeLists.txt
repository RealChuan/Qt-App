set(PROJECT_SOURCES
    pomodoroplugin.hpp
    pomodoroplugin.cc
    pomodoropage.hpp
    pomodoropage.cc)

add_app_plugin(pomodoroplugin ${PROJECT_SOURCES})
target_link_libraries(pomodoroplugin PRIVATE shell extensionsystem utils Qt::Widgets)
