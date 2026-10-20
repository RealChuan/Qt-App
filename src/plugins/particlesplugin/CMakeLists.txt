set(PROJECT_SOURCES
    particlesplugin.hpp
    particlesplugin.cc
    particleswidget.hpp
    particleswidget.cc)

add_app_plugin(particlesplugin ${PROJECT_SOURCES})
target_link_libraries(particlesplugin PRIVATE shell extensionsystem utils Qt::Widgets)
