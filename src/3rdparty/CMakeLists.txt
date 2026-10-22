set(PROJECT_SOURCES
    qtsingleapplication/qtsingleapplication.cc
    qtsingleapplication/qtsingleapplication.hpp
    qtsingleapplication/qtlocalpeer.cc
    qtsingleapplication/qtlocalpeer.hpp
    thirdparty_global.hpp)

# 目录名 3rdparty / target thirdparty / 宏 THIRDPARTY_* 三者对齐。
add_platform_library(thirdparty EXPORT_MACRO THIRDPARTY ${PROJECT_SOURCES})
target_link_libraries(thirdparty PUBLIC utils Qt::Network Qt::Widgets)
