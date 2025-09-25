if(CMAKE_HOST_APPLE)
  set(CMAKE_OSX_DEPLOYMENT_TARGET
      "12.0"
      CACHE STRING "Minimum OS X deployment version")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 编译器特性检查
include(CheckCXXCompilerFlag)
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 8)
    message(
      FATAL_ERROR
        "GCC 8.0+ required for C++20 support. Current: ${CMAKE_CXX_COMPILER_VERSION}"
    )
  endif()
  # 添加GNU特定标志
  add_compile_options(-Wall -Wextra -Wpedantic)

elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 10)
    message(
      FATAL_ERROR
        "Clang 10.0+ required for C++20 support. Current: ${CMAKE_CXX_COMPILER_VERSION}"
    )
  endif()
  # 添加Clang特定标志
  add_compile_options(-Wall -Wextra -Wpedantic)

elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 19.28)
    message(
      FATAL_ERROR
        "MSVC 2019 16.10+ required for C++20 support. Current: ${CMAKE_CXX_COMPILER_VERSION}"
    )
  endif()
  # 添加MSVC特定标志
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

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(ARCH_BITS 64)
  set(ARCH_NAME "x64")
else()
  set(ARCH_BITS 32)
  set(ARCH_NAME "x86")
endif()

message(STATUS "Building for ${ARCH_NAME} (${ARCH_BITS}-bit)")

# 输出目录配置
set(CMAKE_DEBUG_POSTFIX "d")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin-${ARCH_BITS}/libs)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY
    ${PROJECT_SOURCE_DIR}/bin-${ARCH_BITS}/${CMAKE_BUILD_TYPE})

# 为不同配置类型设置不同输出目录（多配置生成器如Visual Studio）
foreach(OUTPUTCONFIG IN ITEMS DEBUG RELEASE RELWITHDEBINFO MINSIZEREL)
  string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG_UPPER)
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG_UPPER}
      ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG_UPPER}
      ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG_UPPER}
      ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
endforeach()

# 包含当前目录和生成目录
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 工具链特性检测
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  check_cxx_compiler_flag(-fstack-protector-strong HAS_STACK_PROTECTOR)
  if(HAS_STACK_PROTECTOR)
    add_compile_options(-fstack-protector-strong)
  endif()
endif()
