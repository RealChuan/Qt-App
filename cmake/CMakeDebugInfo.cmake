# =============================================================================
# CMake Debug Information Utility
# =============================================================================

# -----------------------------------------------------------------------------
# 配置选项
# -----------------------------------------------------------------------------
option(PROJECT_DEBUG_INFO "Print detailed CMake configuration information" ON)

# -----------------------------------------------------------------------------
# 调试信息显示函数
# -----------------------------------------------------------------------------
function(show_cmake_debug_info)
  if(NOT PROJECT_DEBUG_INFO)
    return()
  endif()

  # 获取当前时间戳
  string(TIMESTAMP CURRENT_TIME "%Y-%m-%d %H:%M:%S")

  message(STATUS "")
  message(STATUS "┌───────────── CMake Configuration Debug Info ─────────────┐")
  message(STATUS "│ Timestamp       : ${CURRENT_TIME}")
  message(STATUS "├─────────────────── System Information ───────────────────┤")
  message(STATUS "│ CMake Version   : ${CMAKE_VERSION}")
  message(STATUS "│ Generator       : ${CMAKE_GENERATOR}")

  # 检测生成器额外信息
  if(CMAKE_GENERATOR MATCHES "Visual Studio")
    message(STATUS "│ Generator Platform: ${CMAKE_GENERATOR_PLATFORM}")
    if(CMAKE_GENERATOR_TOOLSET)
      message(STATUS "│ Generator Toolset : ${CMAKE_GENERATOR_TOOLSET}")
    endif()
  endif()

  message(
    STATUS "│ System          : ${CMAKE_SYSTEM_NAME} ${CMAKE_SYSTEM_VERSION}")
  message(STATUS "│ Architecture    : ${CMAKE_SYSTEM_PROCESSOR}")

  # 32/64位检测
  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(BITNESS "64-bit")
  else()
    set(BITNESS "32-bit")
  endif()
  message(STATUS "│ Bitness         : ${BITNESS}")

  # 检测是否为交叉编译
  if(CMAKE_CROSSCOMPILING)
    message(
      STATUS
        "│ Cross-compiling : YES (${CMAKE_HOST_SYSTEM_PROCESSOR} → ${CMAKE_SYSTEM_PROCESSOR})"
    )
  else()
    message(STATUS "│ Cross-compiling : NO")
  endif()

  message(
    STATUS "├──────────────────── Compiler Information ─────────────────┤")
  message(STATUS "│ C Compiler      : ${CMAKE_C_COMPILER}")
  message(
    STATUS
      "│ C Compiler ID   : ${CMAKE_C_COMPILER_ID} ${CMAKE_C_COMPILER_VERSION}")
  message(STATUS "│ CXX Compiler    : ${CMAKE_CXX_COMPILER}")
  message(
    STATUS
      "│ CXX Compiler ID : ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}"
  )
  message(
    STATUS
      "│ CXX Standard    : ${CMAKE_CXX_STANDARD} (Required: ${CMAKE_CXX_STANDARD_REQUIRED})"
  )
  message(STATUS "│ CXX Extensions  : ${CMAKE_CXX_EXTENSIONS}")

  message(
    STATUS "├───────────────────── Build Configuration ─────────────────┤")
  message(STATUS "│ Build Type      : ${CMAKE_BUILD_TYPE}")

  # 多配置检测
  if(CMAKE_CONFIGURATION_TYPES)
    message(STATUS "│ Config Types    : ${CMAKE_CONFIGURATION_TYPES}")
  endif()

  message(STATUS "│ C Flags         : ${CMAKE_C_FLAGS}")
  message(STATUS "│ CXX Flags       : ${CMAKE_CXX_FLAGS}")
  message(STATUS "│ Exe Linker Flags: ${CMAKE_EXE_LINKER_FLAGS}")
  message(STATUS "│ Shared Linker   : ${CMAKE_SHARED_LINKER_FLAGS}")
  message(STATUS "│ Static Linker   : ${CMAKE_STATIC_LINKER_FLAGS}")

  message(
    STATUS "├────────────────────── Directory Layout ───────────────────┤")
  message(STATUS "│ Source Directory: ${CMAKE_SOURCE_DIR}")
  message(STATUS "│ Binary Directory: ${CMAKE_BINARY_DIR}")
  message(STATUS "│ Install Prefix  : ${CMAKE_INSTALL_PREFIX}")

  # 显示项目特定输出目录
  if(DEFINED CMAKE_RUNTIME_OUTPUT_DIRECTORY)
    message(STATUS "│ Runtime Output  : ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
  endif()
  if(DEFINED CMAKE_LIBRARY_OUTPUT_DIRECTORY)
    message(STATUS "│ Library Output  : ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
  endif()
  if(DEFINED CMAKE_ARCHIVE_OUTPUT_DIRECTORY)
    message(STATUS "│ Archive Output  : ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}")
  endif()

  message(
    STATUS "├───────────────────────── Features ────────────────────────┤")
  message(STATUS "│ Build Shared Libs: ${BUILD_SHARED_LIBS}")
  message(STATUS "│ Export Compile Cmds: ${CMAKE_EXPORT_COMPILE_COMMANDS}")

  # 检测其他常用特性
  if(DEFINED CMAKE_BUILD_RPATH)
    message(STATUS "│ Build RPATH     : ${CMAKE_BUILD_RPATH}")
  endif()
  if(DEFINED CMAKE_INSTALL_RPATH)
    message(STATUS "│ Install RPATH   : ${CMAKE_INSTALL_RPATH}")
  endif()

  # 检测编译特性支持
  if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    include(CheckCXXCompilerFlag)
    check_cxx_compiler_flag(-fstack-protector-strong HAS_STACK_PROTECTOR)
    message(STATUS "│ Stack Protector : ${HAS_STACK_PROTECTOR}")
  endif()

  message(
    STATUS "├────────────────────── Project Specific ───────────────────┤")

  # 显示项目特定变量
  if(DEFINED PROJECT_NAME)
    message(STATUS "│ Project Name    : ${PROJECT_NAME}")
  endif()

  if(DEFINED PROJECT_VERSION)
    message(STATUS "│ Project Version : ${PROJECT_VERSION}")
  endif()

  if(DEFINED TARGET_ARCH)
    message(STATUS "│ Target Arch     : ${TARGET_ARCH}")
  endif()

  # 显示自定义缓存变量
  get_cmake_property(CACHE_VARS CACHE_VARIABLES)
  set(CUSTOM_CACHE_VARS "")
  foreach(CACHE_VAR ${CACHE_VARS})
    if(CACHE_VAR MATCHES "^PROJECT_"
       OR CACHE_VAR MATCHES "^CMAKE_"
       OR CACHE_VAR MATCHES "^BUILD_")
      get_property(
        CACHE_VAR_HELPSTRING
        CACHE ${CACHE_VAR}
        PROPERTY HELPSTRING)
      if(CACHE_VAR_HELPSTRING AND NOT CACHE_VAR_HELPSTRING STREQUAL "")
        list(APPEND CUSTOM_CACHE_VARS "${CACHE_VAR}=${${CACHE_VAR}}")
      endif()
    endif()
  endforeach()

  if(CUSTOM_CACHE_VARS)
    list(SORT CUSTOM_CACHE_VARS)
    message(STATUS "│ Custom Cache Vars:")
    foreach(VAR ${CUSTOM_CACHE_VARS})
      message(STATUS "│   ${VAR}")
    endforeach()
  endif()

  message(
    STATUS "└────────────────────────────────────────────────────────────┘")
  message(STATUS "")
endfunction()

# -----------------------------------------------------------------------------
# 便捷函数：在特定条件下显示调试信息
# -----------------------------------------------------------------------------
function(show_debug_info_if_requested)
  # 如果用户明确要求或者设置了详细模式
  if(PROJECT_DEBUG_INFO OR CMAKE_VERBOSE_MAKEFILE)
    show_cmake_debug_info()
  endif()
endfunction()

# -----------------------------------------------------------------------------
# 测试函数（仅用于调试本模块）
# -----------------------------------------------------------------------------
function(test_debug_info_module)
  message(STATUS "Testing CMakeDebugInfo module...")
  set(PROJECT_DEBUG_INFO ON)
  show_cmake_debug_info()
endfunction()

# -----------------------------------------------------------------------------
# 自动注册调试信息显示（可选）
# -----------------------------------------------------------------------------
# 取消注释以下行以在包含此文件时自动显示调试信息（如果启用）
show_debug_info_if_requested()
