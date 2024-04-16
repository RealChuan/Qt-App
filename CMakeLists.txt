cmake_minimum_required(VERSION 3.25.1)

if(CMAKE_HOST_WIN32)
  set(CMAKE_TOOLCHAIN_FILE
      "C:/vcpkg/scripts/buildsystems/vcpkg.cmake"
      CACHE STRING "Vcpkg toolchain file")
elseif(CMAKE_HOST_APPLE)
  set(CMAKE_TOOLCHAIN_FILE
      "/usr/local/share/vcpkg/scripts/buildsystems/vcpkg.cmake"
      CACHE STRING "Vcpkg toolchain file")
elseif(CMAKE_HOST_LINUX)
  set(CMAKE_TOOLCHAIN_FILE
      "/usr/local/share/vcpkg/scripts/buildsystems/vcpkg.cmake"
      CACHE STRING "Vcpkg toolchain file")
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(utils)

if(CMAKE_HOST_WIN32)
  list(APPEND CMAKE_PREFIX_PATH "C:\\Qt\\6.7.0\\msvc2019_64")
elseif(CMAKE_HOST_APPLE)

elseif(CMAKE_HOST_LINUX)
  list(APPEND CMAKE_PREFIX_PATH "/opt/Qt/6.7.0/gcc_64")
endif()

project(
  Qt-App
  VERSION 0.0.1
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_DEBUG_POSTFIX d)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message("Setting build type to 'RelWithDebInfo' as none was specified.")
  set(CMAKE_BUILD_TYPE
      RelWithDebInfo
      CACHE STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release"
                                               "MinSizeRel" "RelWithDebInfo")
endif()

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(CURRENT_PLATFORM "-64")
else()
  set(CURRENT_PLATFORM "-32")
endif()

message(STATUS "Current Platform is ${CURRENT_PLATFORM}")

# 设定可执行二进制文件的目录
set(EXECUTABLE_OUTPUT_PATH
    ${PROJECT_SOURCE_DIR}/bin${CURRENT_PLATFORM}/${CMAKE_BUILD_TYPE}) # 源文件目录
# 设定存放编译出来的库文件的目录
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin${CURRENT_PLATFORM}/libs)
# 并且把该目录设为连接目录
link_directories(${LIBRARY_OUTPUT_PATH})

include_directories(${PROJECT_SOURCE_DIR})

find_package(
  Qt6 REQUIRED
  COMPONENTS Widgets
             Network
             Core5Compat
             Concurrent
             Multimedia
             OpenGLWidgets
             SerialPort
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

include_directories(src)
add_subdirectory(src)
add_subdirectory(examples)

string(TOLOWER ${PROJECT_NAME} Lower_PROJECT_NAME)

qt_add_translations(
  ${PROJECT_NAME}
  TS_FILE_BASE
  ${Lower_PROJECT_NAME}
  TS_FILE_DIR
  translations
  INCLUDE_DIRECTORIES
  directory
  ${PROJECT_SOURCE_DIR}/src)

file(GLOB_RECURSE TS_FILES "${PROJECT_SOURCE_DIR}/translations/*.ts")
set_qt_translations_output_location(${TS_FILES})
