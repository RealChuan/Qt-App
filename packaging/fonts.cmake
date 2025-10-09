cmake_minimum_required(VERSION 3.15)

message(STATUS "Current directory: ${CMAKE_CURRENT_BINARY_DIR}")

# 设置项目变量 - 使用当前工作目录
set(FONT_URL
    "https://github.com/RealChuan/Qt-App/releases/download/0.0.1/fonts.7z")
set(FONT_ARCHIVE "${CMAKE_CURRENT_BINARY_DIR}/fonts.7z")
if(CMAKE_HOST_APPLE)
  set(FONT_DIR "Qt-App.app/Contents/Resources/fonts")
else()
  set(FONT_DIR "resources/fonts")
endif()

# 创建目标目录
file(MAKE_DIRECTORY "${FONT_DIR}")

# 下载文件
message(STATUS "Downloading fonts from ${FONT_URL}")

file(
  DOWNLOAD "${FONT_URL}" "${FONT_ARCHIVE}"
  STATUS status
  SHOW_PROGRESS)

list(GET status 0 error_code)
list(GET status 1 error_message)

if(NOT error_code EQUAL 0)
  message(FATAL_ERROR "Error downloading file: ${error_message}")
endif()

# 解压文件
execute_process(COMMAND 7z x "${FONT_ARCHIVE}" -o${FONT_DIR} -y
                RESULT_VARIABLE error_code)

if(NOT error_code EQUAL 0)
  message(
    FATAL_ERROR
      "Error extracting file with code: ${error_code}. Make sure 7z is installed and in PATH."
  )
endif()

# 清理压缩文件
file(REMOVE "${FONT_ARCHIVE}")
message(STATUS "Successfully cleaned up archive file")

# 验证解压结果
file(GLOB_RECURSE extracted_files "${FONT_DIR}/*")
list(LENGTH extracted_files file_count)

if(file_count EQUAL 0)
  message(WARNING "No files were extracted to ${FONT_DIR}")
else()
  message(
    STATUS "Successfully extracted ${file_count} font files to ${FONT_DIR}")

  # 使用自定义变量控制详细输出
  if(DEFINED ENV{VERBOSE} OR DEFINED VERBOSE)
    foreach(file IN LISTS extracted_files)
      message(STATUS "Extracted: ${file}")
    endforeach()
  endif()
endif()

message(STATUS "Fonts have been downloaded and extracted to ${FONT_DIR}")

if(CMAKE_HOST_WIN32)
  execute_process(COMMAND cmd /c tree /f)
else()
  execute_process(COMMAND tree)
endif()
