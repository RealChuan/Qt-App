set(PROJECT_SOURCES crashwidgets.cc crashwidgets.hpp main.cc)
if(CMAKE_HOST_WIN32)
  list(APPEND PROJECT_SOURCES app.rc)
endif()
qt_add_executable(CrashReport MANUAL_FINALIZATION ${PROJECT_SOURCES})
set_target_properties(CrashReport PROPERTIES WIN32_EXECUTABLE ON)
target_link_libraries(
  CrashReport
  PRIVATE dump
          thirdparty
          widgets
          resource
          utils
          Qt::Network
          Qt::Core5Compat
          Qt::Widgets)
qt_finalize_executable(CrashReport)

if(CMAKE_HOST_APPLE)
  set(BUNDLE_CONTENTS_DIR
      "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PROJECT_NAME}.app/Contents/MacOS")

  add_custom_command(
    TARGET CrashReport
    POST_BUILD
    COMMENT "Deploying CrashReport to: ${BUNDLE_CONTENTS_DIR}"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${BUNDLE_CONTENTS_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:CrashReport>
            "${BUNDLE_CONTENTS_DIR}/$<TARGET_FILE_NAME:CrashReport>")
endif()

install(
  TARGETS CrashReport
  RUNTIME DESTINATION ${TOOL_INSTALL_DIR}
  BUNDLE DESTINATION ${TOOL_INSTALL_DIR})
