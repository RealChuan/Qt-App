set(PROJECT_SOURCES
    qtsingleapplication/qtsingleapplication.cpp
    qtsingleapplication/qtsingleapplication.h
    qtsingleapplication/qtlocalpeer.cpp
    qtsingleapplication/qtlocalpeer.h
    thirdparty_global.hpp)

# thirdparty_global.hpp 里历史上拼成 THRIDPARTY_*，这里用 EXPORT_MACRO 保持原样，
# 避免为了构建系统去改第三方源码。
add_platform_library(thirdparty EXPORT_MACRO THRIDPARTY ${PROJECT_SOURCES})
target_link_libraries(thirdparty PUBLIC utils Qt::Network Qt::Widgets)
