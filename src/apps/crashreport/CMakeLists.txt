set(PROJECT_SOURCES crashwidgets.cc crashwidgets.hpp main.cc)

if(WIN32)
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
          Qt::Widgets)

qtapp_apply_pch(CrashReport)

qt_finalize_executable(CrashReport)

if(APPLE)
  # CrashReport 需要放进主 app 的 Contents/MacOS，路径跟随主目标所在目录
  set(BUNDLE_CONTENTS_DIR "$<TARGET_FILE_DIR:${PROJECT_NAME}>")

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
