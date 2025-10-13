set(PROJECT_SOURCES main.cc)
qt_add_executable(CrashpadTest MANUAL_FINALIZATION ${PROJECT_SOURCES})
set_target_properties(CrashpadTest PROPERTIES WIN32_EXECUTABLE TRUE)

target_link_libraries(CrashpadTest PRIVATE dump crashpad::crashpad)
qt_finalize_executable(CrashpadTest)

setup_crashpad_handler("${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
