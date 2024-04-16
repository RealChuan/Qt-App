set(PROJECT_SOURCES main.cc)
qt_add_executable(I18N MANUAL_FINALIZATION ${PROJECT_SOURCES})
set_target_properties(I18N PROPERTIES MACOSX_BUNDLE FALSE WIN32_EXECUTABLE TRUE)
set_target_properties(I18N PROPERTIES RUNTIME_OUTPUT_DIRECTORY
                                      "${EXECUTABLE_OUTPUT_PATH}/examples")

target_link_libraries(I18N PRIVATE Qt6::Core)
qt_finalize_executable(I18N)
