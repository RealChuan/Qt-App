set(PROJECT_SOURCES
    tetrisplugin.hpp
    tetrisplugin.cc
    tetrispage.hpp
    tetrispage.cc
    tetrisboard.hpp
    tetrisboard.cc
    tetrispreview.hpp
    tetrispreview.cc
    tetriscolors.hpp
    tetrisgame.hpp
    tetrisgame.cc)

add_app_plugin(tetrisplugin ${PROJECT_SOURCES})
target_link_libraries(tetrisplugin PRIVATE shell extensionsystem utils Qt::Widgets)
