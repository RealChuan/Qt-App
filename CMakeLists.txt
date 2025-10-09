cmake_minimum_required(VERSION 3.25.1)

include(cmake/macOSDeploymentTarget.cmake)
include(cmake/VcpkgToolchain.cmake)
include(cmake/QtSetup.cmake)

project(
  Qt-App
  VERSION 0.1.1
  DESCRIPTION "This is a Qt Application template"
  HOMEPAGE_URL "https://github.com/RealChuan/Qt-App"
  LANGUAGES CXX)

include(cmake/ProjectSettings.cmake)
include(cmake/InstallConfig.cmake)
include(cmake/PlatformLibraries.cmake)

find_package(
  Qt6 REQUIRED
  COMPONENTS Core
             CorePrivate
             Widgets
             Network
             Concurrent
             Core5Compat
             LinguistTools)

qt_standard_project_setup(I18N_SOURCE_LANGUAGE en I18N_TRANSLATED_LANGUAGES
                          zh_CN)

include(cmake/QtTranslations.cmake)

find_package(unofficial-breakpad CONFIG REQUIRED)
if(unofficial-breakpad_FOUND)
  message(STATUS "found unofficial-breakpad")
endif()
find_package(crashpad CONFIG REQUIRED)
if(crashpad_FOUND)
  message(STATUS "found crashpad")
endif()
find_package(tl-expected CONFIG REQUIRED)
if(tl-expected_FOUND)
  message(STATUS "found tl-expected")
endif()

include_directories(src)

add_subdirectory(src)
add_subdirectory(tests)
add_subdirectory(examples)

add_translations(${PROJECT_NAME})

set(deploy_tool_options_arg "")
if(WIN32)
  set(deploy_tool_options_arg "--compiler-runtime" "--openssl-root"
                              "${VCPKG_ROOT}/installed/${ARCH_DIR}-windows")
endif()
qt_generate_deploy_app_script(
  TARGET ${PROJECT_NAME} OUTPUT_SCRIPT deploy_script DEPLOY_TOOL_OPTIONS
  ${deploy_tool_options_arg})
install(SCRIPT ${deploy_script})

include(cmake/CMakeDebugInfo.cmake)
