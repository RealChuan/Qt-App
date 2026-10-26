set(PROJECT_SOURCES
    fortuneplugin.hpp
    fortuneplugin.cc
    fortunepage.hpp
    fortunepage.cc
    fortuneform.hpp
    fortuneform.cc
    fortunereadingview.hpp
    fortunereadingview.cc
    fortunesharing.hpp
    fortunesharing.cc
    fortunestrings.hpp
    hexagramboard.hpp
    hexagramboard.cc
    fortuneengine.hpp
    fortuneengine.cc
    fortunelunar.hpp
    fortunelunar.cc
    fortunecontent.hpp
    fortunecontent.cc)

add_app_plugin(fortuneplugin ${PROJECT_SOURCES})
target_link_libraries(fortuneplugin PRIVATE shell extensionsystem widgets utils
                                            Qt::Widgets)
