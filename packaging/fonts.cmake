cmake_minimum_required(VERSION 3.10)

# 下载文件
file(
  DOWNLOAD
  "https://github.com/RealChuan/Qt-App/releases/download/0.0.1/fonts.7z"
  "fonts.7z"
  STATUS status
  SHOW_PROGRESS)

list(GET status 0 error_code)
list(GET status 1 error_message)

if(error_code GREATER 0)
  message(FATAL_ERROR "Error downloading file: ${error_message}")
endif()

# 解压文件
execute_process(COMMAND 7z x fonts.7z -o./fonts RESULT_VARIABLE error_code)

if(NOT error_code EQUAL 0)
  message(FATAL_ERROR "Error extracting file")
endif()

execute_process(COMMAND rm -f fonts.7z)

# 列出文件
file(GLOB_RECURSE files ./fonts/*)
foreach(file IN LISTS files)
  message("${file}")
endforeach()
