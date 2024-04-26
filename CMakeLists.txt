cmake_minimum_required(VERSION 3.25.1)

include(cmake/vcpkg.cmake)
include(cmake/qt.cmake)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(utils)
include(qt_utils)

project(
  Qt-App
  VERSION 0.1.1
  DESCRIPTION "This is a qt Application template"
  HOMEPAGE_URL "https://github.com/RealChuan/Qt-App"
  LANGUAGES CXX)

include(cmake/common.cmake)

find_package(
  Qt6 REQUIRED
  COMPONENTS Widgets
             Network
             Core5Compat
             LinguistTools)

qt_standard_project_setup(I18N_SOURCE_LANGUAGE en I18N_TRANSLATED_LANGUAGES
                          zh_CN)
# qt_standard_project_setup will set CMAKE_RUNTIME_OUTPUT_DIRECTORY, we need to
# set it back, and use EXECUTABLE_OUTPUT_PATH
unset(CMAKE_RUNTIME_OUTPUT_DIRECTORY)

find_package(unofficial-breakpad CONFIG REQUIRED)
if(unofficial-breakpad_FOUND)
  message(STATUS "found unofficial-breakpad")
endif()
find_package(crashpad CONFIG REQUIRED)
if(crashpad_FOUND)
  message(STATUS "found crashpad")
endif()

include_directories(src)
add_subdirectory(src)
add_subdirectory(tests)
add_subdirectory(examples)

add_translations(${PROJECT_NAME})

include(cmake/build_info.cmake)
