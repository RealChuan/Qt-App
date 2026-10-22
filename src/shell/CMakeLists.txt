set(PROJECT_SOURCES
    shell_global.hpp
    page.hpp
    command.hpp
    functioncommand.hpp
    functioncommand.cc
    registry.hpp
    registry.cc
    host.hpp
    host.cc)

add_shared_library(shell ${PROJECT_SOURCES})
target_link_libraries(shell PUBLIC extensionsystem utils Qt::Widgets)
