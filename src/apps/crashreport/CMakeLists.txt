set(PROJECT_SOURCES crashwidgets.cc crashwidgets.hpp main.cc)
if(CMAKE_HOST_WIN32)
  list(APPEND PROJECT_SOURCES app.rc)
endif()
qt_add_executable(CrashReport MANUAL_FINALIZATION ${PROJECT_SOURCES})
add_rpath(CrashReport)
set_target_properties(CrashReport PROPERTIES WIN32_EXECUTABLE ON)
target_link_libraries(
  CrashReport
  PRIVATE dump
          thirdparty
          gui
          resource
          utils
          Qt6::Network
          Qt6::Core5Compat
          Qt6::Widgets)
qt_finalize_executable(CrashReport)
