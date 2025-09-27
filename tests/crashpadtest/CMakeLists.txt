set(PROJECT_SOURCES main.cc)
qt_add_executable(CrashpadTest MANUAL_FINALIZATION ${PROJECT_SOURCES})
set_target_properties(CrashpadTest PROPERTIES WIN32_EXECUTABLE TRUE)

target_link_libraries(CrashpadTest PRIVATE dump crashpad::crashpad)
qt_finalize_executable(CrashpadTest)

string(REPLACE "share/crashpad" "tools/crashpad" crash_handler_path
               "${crashpad_DIR}")
message(STATUS "Crashpad tools directory: ${crash_handler_path}")

add_custom_command(
  TARGET CrashpadTest
  POST_BUILD
  COMMENT "Copying crashpad handler to output directory"
  COMMAND ${CMAKE_COMMAND} -E copy_directory "${crash_handler_path}"
          "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
  VERBATIM)

if(CMAKE_HOST_UNIX)
  add_custom_command(
    TARGET CrashpadTest
    POST_BUILD
    COMMENT "Setting execute permissions for crashpad_handler on Linux"
    COMMAND chmod +x "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/crashpad_handler"
    VERBATIM)
endif()
