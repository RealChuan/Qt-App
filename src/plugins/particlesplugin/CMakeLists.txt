set(PROJECT_SOURCES
    particlesplugin.hpp
    particlesplugin.cc
    particlespage.hpp
    particlespage.cc)

add_app_plugin(particlesplugin ${PROJECT_SOURCES})
target_link_libraries(particlesplugin PRIVATE shell extensionsystem utils Qt::Widgets)
