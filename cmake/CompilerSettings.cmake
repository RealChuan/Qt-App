# =============================================================================
# Standard Project Settings for Modern C++ Projects
# =============================================================================

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_DEBUG_POSTFIX "d")

include(CheckCXXCompilerFlag)

# 编译器版本检查
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 8.0)
    message(
      FATAL_ERROR
        "GCC 8.0+ required for C++20 support. Current: ${CMAKE_CXX_COMPILER_VERSION}"
    )
  endif()
  set(COMPILER_GNU ON)

elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 10.0)
    message(
      FATAL_ERROR
        "Clang 10.0+ required for C++20 support. Current: ${CMAKE_CXX_COMPILER_VERSION}"
    )
  endif()
  set(COMPILER_CLANG ON)

elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 19.28)
    message(
      FATAL_ERROR
        "MSVC 2019 16.10+ required for C++20 support. Current: ${CMAKE_CXX_COMPILER_VERSION}"
    )
  endif()
  set(COMPILER_MSVC ON)
endif()

# 编译器标志设置
if(COMPILER_GNU OR COMPILER_CLANG)
  add_compile_options(-Wall -Wextra -Wpedantic)

  # 安全检查
  check_cxx_compiler_flag(-fstack-protector-strong HAS_STACK_PROTECTOR)
  if(HAS_STACK_PROTECTOR)
    add_compile_options(-fstack-protector-strong)
  endif()

elseif(COMPILER_MSVC)
  add_compile_options(/W4 /permissive-)
endif()

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(
    STATUS "Setting build type to 'RelWithDebInfo' as none was specified.")
  set(CMAKE_BUILD_TYPE
      RelWithDebInfo
      CACHE STRING "Build type" FORCE)

  # 为cmake-gui提供可选值
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release"
                                               "MinSizeRel" "RelWithDebInfo")
endif()

function(detect_target_architecture)
  # 最高优先级: 检查macOS特定架构设置
  if(CMAKE_HOST_APPLE AND CMAKE_OSX_ARCHITECTURES)
    # 检查是否为通用二进制文件（包含多个架构）
    list(FIND CMAKE_OSX_ARCHITECTURES "x86_64" HAS_X86_64)
    list(FIND CMAKE_OSX_ARCHITECTURES "arm64" HAS_ARM64)

    # 如果同时包含x86_64和arm64，则为通用二进制文件
    if(HAS_X86_64 GREATER_EQUAL 0 AND HAS_ARM64 GREATER_EQUAL 0)
      set(TARGET_ARCH
          "universal"
          PARENT_SCOPE)
      message(STATUS "Target architecture: universal (x86_64 + arm64)")
      return()
    endif()

    # 单一架构处理
    list(GET CMAKE_OSX_ARCHITECTURES 0 DETECTED_ARCH)
    if(DETECTED_ARCH MATCHES "arm64")
      set(TARGET_ARCH
          "arm64"
          PARENT_SCOPE)
      message(
        STATUS "Target architecture (from CMAKE_OSX_ARCHITECTURES): arm64")
      return()
    elseif(DETECTED_ARCH MATCHES "x86_64")
      set(TARGET_ARCH
          "x86_64"
          PARENT_SCOPE)
      message(
        STATUS "Target architecture (from CMAKE_OSX_ARCHITECTURES): x86_64")
      return()
    endif()
  endif()

  # 次高优先级: 检查Windows Generator Platform (-A 参数)
  if(CMAKE_GENERATOR_PLATFORM)
    if(CMAKE_GENERATOR_PLATFORM MATCHES "x64|AMD64")
      set(TARGET_ARCH
          "x86_64"
          PARENT_SCOPE)
      message(
        STATUS "Target architecture (from CMAKE_GENERATOR_PLATFORM): x86_64")
      return()
    elseif(CMAKE_GENERATOR_PLATFORM MATCHES "ARM64")
      set(TARGET_ARCH
          "arm64"
          PARENT_SCOPE)
      message(
        STATUS "Target architecture (from CMAKE_GENERATOR_PLATFORM): arm64")
      return()
    elseif(CMAKE_GENERATOR_PLATFORM MATCHES "Win32")
      set(TARGET_ARCH
          "x86"
          PARENT_SCOPE) # 注意这里是x86
      message(STATUS "Target architecture (from CMAKE_GENERATOR_PLATFORM): x86")
      return()
    endif()
  endif()

  # 检查交叉编译环境 (CMAKE_SYSTEM_PROCESSOR)
  if(CMAKE_CROSSCOMPILING)
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|ARM64|arm64")
      set(TARGET_ARCH
          "arm64"
          PARENT_SCOPE)
      message(
        STATUS
          "Target architecture (from CMAKE_SYSTEM_PROCESSOR, cross-compiling): arm64"
      )
      return()
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64")
      set(TARGET_ARCH
          "x86_64"
          PARENT_SCOPE)
      message(
        STATUS
          "Target architecture (from CMAKE_SYSTEM_PROCESSOR, cross-compiling): x86_64"
      )
      return()
    endif()
  endif()

  # 最低优先级: 回退到本地主机架构检测
  if(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "aarch64|ARM64|arm64")
    set(TARGET_ARCH
        "arm64"
        PARENT_SCOPE)
    message(STATUS "Target architecture (detected from host): arm64")
  elseif(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64")
    set(TARGET_ARCH
        "x86_64"
        PARENT_SCOPE)
    message(STATUS "Target architecture (detected from host): x86_64")
  else()
    set(TARGET_ARCH
        "unknown"
        PARENT_SCOPE)
    message(
      WARNING
        "Could not reliably detect the target architecture. Consider specifying it explicitly."
    )
  endif()
endfunction()

# 初始架构检测
detect_target_architecture()

if(TARGET_ARCH STREQUAL "unknown")
  # 备用检测方法
  if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(TARGET_ARCH "x86_64")
  else()
    execute_process(
      COMMAND uname -m
      OUTPUT_VARIABLE UNAME_M
      OUTPUT_STRIP_TRAILING_WHITESPACE)
    if(UNAME_M MATCHES "aarch64|arm64")
      set(TARGET_ARCH "arm64")
    elseif(UNAME_M MATCHES "x86_64|amd64")
      set(TARGET_ARCH "x86_64")
    else()
      message(WARNING "Cannot detect target architecture automatically")
      set(TARGET_ARCH "unknown")
    endif()
  endif()
endif()

message(STATUS "Target architecture: ${TARGET_ARCH}")

# 检测是否为多配置生成器
if(CMAKE_CONFIGURATION_TYPES)
  set(IS_MULTI_CONFIG ON)
else()
  set(IS_MULTI_CONFIG OFF)
endif()

function(setup_output_directories)
  # 确定架构目录名
  if(TARGET_ARCH STREQUAL "x86_64")
    set(ARCH_DIR "x64")
  elseif(TARGET_ARCH STREQUAL "arm64")
    set(ARCH_DIR "arm64")
  elseif(TARGET_ARCH STREQUAL "universal")
    set(ARCH_DIR "universal")
  elseif(TARGET_ARCH STREQUAL "x86")
    set(ARCH_DIR "x86")
  else()
    set(ARCH_DIR "unknown")
  endif()

  # 基础输出路径
  set(BASE_OUTPUT_DIR "${PROJECT_SOURCE_DIR}/binaries/${ARCH_DIR}")

  if(IS_MULTI_CONFIG)
    # 多配置生成器（Visual Studio, Xcode）
    foreach(config IN LISTS CMAKE_CONFIGURATION_TYPES)
      string(TOUPPER "${config}" CONFIG_UPPER)

      set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CONFIG_UPPER}
          "${BASE_OUTPUT_DIR}/lib/${config}"
          PARENT_SCOPE)
      set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CONFIG_UPPER}
          "${BASE_OUTPUT_DIR}/lib/${config}"
          PARENT_SCOPE)
      set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CONFIG_UPPER}
          "${BASE_OUTPUT_DIR}/bin/${config}"
          PARENT_SCOPE)
    endforeach()

    # 设置默认目录
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY
        "${BASE_OUTPUT_DIR}/lib"
        PARENT_SCOPE)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY
        "${BASE_OUTPUT_DIR}/lib"
        PARENT_SCOPE)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY
        "${BASE_OUTPUT_DIR}/bin"
        PARENT_SCOPE)

  else()
    # 单配置生成器（Makefile, Ninja）
    if(CMAKE_BUILD_TYPE STREQUAL "None")
      set(OUTPUT_SUFFIX "none")
    else()
      string(TOLOWER "${CMAKE_BUILD_TYPE}" OUTPUT_SUFFIX)
    endif()

    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY
        "${BASE_OUTPUT_DIR}/lib/${OUTPUT_SUFFIX}"
        PARENT_SCOPE)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY
        "${BASE_OUTPUT_DIR}/lib/${OUTPUT_SUFFIX}"
        PARENT_SCOPE)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY
        "${BASE_OUTPUT_DIR}/bin/${OUTPUT_SUFFIX}"
        PARENT_SCOPE)
  endif()

  # 创建目录
  file(MAKE_DIRECTORY "${BASE_OUTPUT_DIR}/bin")
  file(MAKE_DIRECTORY "${BASE_OUTPUT_DIR}/lib")
endfunction()

setup_output_directories()

function(setup_link_directories)
  # 添加库搜索路径
  if(CMAKE_ARCHIVE_OUTPUT_DIRECTORY)
    link_directories("${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}")
  endif()

  if(CMAKE_LIBRARY_OUTPUT_DIRECTORY)
    link_directories("${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
  endif()

  message(STATUS "Static library directory: ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}")
  message(STATUS "Shared library directory: ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
endfunction()

setup_link_directories()

if(CMAKE_CROSSCOMPILING)
  message(
    STATUS
      "Cross-compiling: ${CMAKE_HOST_SYSTEM_PROCESSOR} -> ${CMAKE_SYSTEM_PROCESSOR}"
  )

  # 设置交叉编译查找规则
  if(TARGET_ARCH STREQUAL "arm64")
    set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
    set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
    set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
  endif()
endif()
