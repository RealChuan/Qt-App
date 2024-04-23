set(PROJECT_SOURCES main.cc)
qt_add_executable(CrashpadTest MANUAL_FINALIZATION ${PROJECT_SOURCES})
set_target_properties(CrashpadTest PROPERTIES WIN32_EXECUTABLE TRUE)

target_link_libraries(CrashpadTest PRIVATE dump crashpad::crashpad)
qt_finalize_executable(CrashpadTest)

string(REPLACE "share/crashpad" "tools/crashpad" crash_handler_path
               ${crashpad_DIR})
message(STATUS "crashpad tools directory: ${crash_handler_path}")

if(CMAKE_HOST_APPLE)
  add_custom_command(
    TARGET CrashpadTest
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${crash_handler_path}
            ${EXECUTABLE_OUTPUT_PATH})
endif()
