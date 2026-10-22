cmake_minimum_required(VERSION 3.25.1)

# cmake-format: off
# 构建系统入口
#
# 对外契约（packaging/ 与 CI 依赖，勿随意改动）：
#   -DTARGET_ARCH=<x86_64|arm64|x86|universal>
#   -DVCPKG_TARGET_TRIPLET=<triplet>
#   产物：binaries/<arch-dir>/bin/<config>/ 与 binaries/<arch-dir>/lib/<config>/
# cmake-format: on

# ---- 构建选项 ---------------------------------------------------------------
option(USE_VCPKG "Use vcpkg for dependency management" ON)
option(QT_APP_USE_CCACHE "探测到 ccache/sccache 时用作编译器启动器" ON)
option(QT_APP_UNITY_BUILD "使用 unity(jumbo) 构建以减少重复解析" OFF)
option(QT_APP_PCH "预编译 Qt 头文件（src/precompiled.hpp）" OFF)
option(QT_APP_RELEASE_WITH_DEBUGINFO "Release 也生成调试信息（与 qmake 侧一致）" ON)
option(QT_APP_BUILD_TESTS "构建 QTest 单元测试（不安装、不打包）" OFF)
option(PROJECT_DEBUG_INFO "打印详细的 CMake 配置信息" OFF)

# ---- 版本号唯一来源：packaging/version.txt ----------------------------------
# Windows / macOS / Ubuntu 打包脚本与 qmake 侧读同一个文件
set(APP_VERSION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/packaging/version.txt")
if(NOT EXISTS "${APP_VERSION_FILE}")
  message(FATAL_ERROR "Version file not found: ${APP_VERSION_FILE}")
endif()
file(READ "${APP_VERSION_FILE}" APP_VERSION)
string(STRIP "${APP_VERSION}" APP_VERSION)
# 注意：CMake 的正则不支持 {n,m} 量词，这里显式展开
if(NOT APP_VERSION MATCHES "^[0-9]+\\.[0-9]+\\.[0-9]+(\\.[0-9]+)?$")
  message(FATAL_ERROR "packaging/version.txt 内容不是合法的版本号: '${APP_VERSION}'\n"
                      "期望形如 0.2.1 或 0.2.1.0（仅数字与点）")
endif()
message(STATUS "Project version (packaging/version.txt): ${APP_VERSION}")

# vcpkg.json 的 version 只能手写，这里做一次一致性校验，避免两处漂移
set(APP_VCPKG_MANIFEST "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg.json")
if(EXISTS "${APP_VCPKG_MANIFEST}")
  file(READ "${APP_VCPKG_MANIFEST}" APP_VCPKG_MANIFEST_CONTENT)
  string(REGEX MATCH "\"version\"[ \t\r\n]*:[ \t\r\n]*\"([^\"]+)\""
               APP_VCPKG_MANIFEST_MATCH "${APP_VCPKG_MANIFEST_CONTENT}")
  set(APP_VCPKG_MANIFEST_VERSION "${CMAKE_MATCH_1}")
  if(APP_VCPKG_MANIFEST_VERSION AND NOT APP_VCPKG_MANIFEST_VERSION STREQUAL
                                    APP_VERSION)
    message(WARNING "vcpkg.json 的 version(${APP_VCPKG_MANIFEST_VERSION}) 与 "
                    "packaging/version.txt(${APP_VERSION}) 不一致")
  endif()
endif()

# ---- C++ 侧版本号一致性 ------------------------------------------------------
# appinfo.hpp 是 C++ 侧唯一的版本来源，而打包脚本 / CI / vcpkg 读的是 version.txt。
# C++ 读不了 version.txt，所以在这里强校验：两边漂移时配置期就失败，
# 不会出现"安装包是 0.2.2、关于页还写着 0.2.1"。
#
# 注意：file(READ) 只在 configure 期生效，必须把该头文件登记为 configure 依赖，
# 否则改完头文件不会重新配置，校验形同虚设。
set_property(
  DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
                           "${CMAKE_CURRENT_SOURCE_DIR}/src/utils/appinfo.hpp")
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/src/utils/appinfo.hpp" APP_INFO_HEADER)
if(NOT APP_INFO_HEADER MATCHES "version = \"${APP_VERSION}\"")
  message(
    FATAL_ERROR "src/utils/appinfo.hpp 的 version 与 packaging/version.txt (${APP_VERSION}) 不一致")
endif()

# ---- 必须在 project() 之前 ------------------------------------------------
# VcpkgToolchain 需要在 project() 触发编译器探测前设置 CMAKE_TOOLCHAIN_FILE；
# macOSDeploymentTarget 需要在编译器初始化前设置 CMAKE_OSX_DEPLOYMENT_TARGET。 这两个文件因此只能用
# CMAKE_HOST_* 判断（那时 WIN32/APPLE/UNIX 尚未定义）。
include(cmake/macOSDeploymentTarget.cmake)
include(cmake/VcpkgToolchain.cmake)

project(
  Qt-App
  VERSION ${APP_VERSION}
  DESCRIPTION "This is a Qt Application template"
  HOMEPAGE_URL "https://github.com/RealChuan/Qt-App"
  LANGUAGES CXX)

# ---- project() 之后：此时 WIN32 / APPLE / UNIX 才可靠 ----------------------
include(cmake/QtSetup.cmake)
include(cmake/ProjectSettings.cmake)
include(cmake/InstallConfig.cmake)
include(cmake/PlatformLibraries.cmake)
include(cmake/InstallCrashpad.cmake)

find_package(Qt6 6.7 REQUIRED COMPONENTS Core Widgets Network Concurrent
                                        LinguistTools)

qt_standard_project_setup(I18N_SOURCE_LANGUAGE en I18N_TRANSLATED_LANGUAGES
                          zh_CN)

include(cmake/QtDeployment.cmake)

find_package(crashpad CONFIG REQUIRED)

add_subdirectory(src)

add_translations(${PROJECT_NAME})
deploy_qt_application(${PROJECT_NAME})

# ---- 单元测试 ---------------------------------------------------------------
# 默认关闭：三条打包链（windows / macos / ubuntu / debian）共用同一份 configure，
# 打开会额外编译一批不参与发布的测试目标。产物见 tests/CMakeLists.txt。
if(QT_APP_BUILD_TESTS)
  find_package(Qt6 REQUIRED COMPONENTS Test)
  enable_testing()
  add_subdirectory(tests)
endif()

include(cmake/CMakeDebugInfo.cmake)
