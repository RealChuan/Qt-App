set(PROJECT_SOURCES main.cc)
qt_add_executable(BreakpadTest MANUAL_FINALIZATION ${PROJECT_SOURCES})
set_target_properties(BreakpadTest PROPERTIES WIN32_EXECUTABLE TRUE)

target_link_libraries(
  BreakpadTest PRIVATE dump unofficial::breakpad::libbreakpad
                       unofficial::breakpad::libbreakpad_client)
qt_finalize_executable(BreakpadTest)
