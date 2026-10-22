set(PROJECT_SOURCES
    colorpickerplugin.hpp
    colorpickerplugin.cc
    colorpickerpage.hpp
    colorpickerpage.cc
    colorpickerwidget.hpp
    colorpickerwidget.cc)

add_app_plugin(colorpickerplugin ${PROJECT_SOURCES})
target_link_libraries(colorpickerplugin PRIVATE shell extensionsystem utils Qt::Widgets)
