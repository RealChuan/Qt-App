option(PROJECT_DEBUG_INFO "Print detailed CMake configuration" ON)
function(show_cmake_debug_info)
  if(NOT PROJECT_DEBUG_INFO)
    return()
  endif()

  message(STATUS "")
  message(STATUS "[========= CMake Debug Info =========]")
  message(STATUS "CMake Version  : ${CMAKE_VERSION}")
  message(STATUS "Generator      : ${CMAKE_GENERATOR}")
  message(
    STATUS "Platform       : ${CMAKE_SYSTEM_NAME} ${CMAKE_SYSTEM_VERSION}")
  message(STATUS "Architecture   : ${CMAKE_SYSTEM_PROCESSOR}")
  # 32/64 位快速判断
  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(STATUS "Bitness        : 64-bit")
  else()
    message(STATUS "Bitness        : 32-bit")
  endif()

  message(STATUS "")
  message(STATUS "------- Compiler --------")
  message(STATUS "C   Compiler   : ${CMAKE_C_COMPILER}")
  message(
    STATUS "C   Compiler ID: ${CMAKE_C_COMPILER_ID} ${CMAKE_C_COMPILER_VERSION}"
  )
  message(STATUS "CXX Compiler   : ${CMAKE_CXX_COMPILER}")
  message(
    STATUS
      "CXX Compiler ID: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
  message(
    STATUS
      "CXX Standard   : ${CMAKE_CXX_STANDARD} (extensions: ${CMAKE_CXX_EXTENSIONS})"
  )

  message(STATUS "")
  message(STATUS "------- Build --------")
  message(STATUS "Build Type     : ${CMAKE_BUILD_TYPE}")
  message(STATUS "C   Flags      : ${CMAKE_C_FLAGS}")
  message(STATUS "CXX Flags      : ${CMAKE_CXX_FLAGS}")
  message(STATUS "Exe Linker     : ${CMAKE_EXE_LINKER_FLAGS}")
  message(STATUS "Shared Linker  : ${CMAKE_SHARED_LINKER_FLAGS}")

  message(STATUS "")
  message(STATUS "------- Directory --------")
  message(STATUS "Source Dir     : ${CMAKE_SOURCE_DIR}")
  message(STATUS "Binary Dir     : ${CMAKE_BINARY_DIR}")
  message(STATUS "Install Prefix : ${CMAKE_INSTALL_PREFIX}")

  message(STATUS "")
  message(STATUS "------- Output --------")
  message(STATUS "RUNTIME_OUTPUT : ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
  message(STATUS "LIBRARY_OUTPUT : ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
  message(STATUS "ARCHIVE_OUTPUT : ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}")

  message(STATUS "")
  message(STATUS "------- Misc --------")
  message(STATUS "BUILD_SHARED_LIBS        : ${BUILD_SHARED_LIBS}")
  message(STATUS "EXPORT_COMPILE_COMMANDS  : ${CMAKE_EXPORT_COMPILE_COMMANDS}")
  message(STATUS "[====================================]")
  message(STATUS "")
endfunction()

# 自动调用（也可手动调用）
show_cmake_debug_info()
