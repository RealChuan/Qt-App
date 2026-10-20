set(PROJECT_SOURCES
    algorithm.h
    appdata.hpp
    appinfo.cpp
    appinfo.h
    asynclog.cc
    asynclog.hpp
    filepath.cpp
    filepath.h
    hostosinfo.cpp
    hostosinfo.h
    languagemanager.cc
    languagemanager.hpp
    logbuffer.cc
    logbuffer.hpp
    logsink.cc
    logsink.hpp
    osspecificaspects.h
    predicates.h
    qtcassert.cpp
    qtcassert.h
    result.cpp
    result.h
    settings.cpp
    settings.h
    singleton.hpp
    singletonmanager.cc
    singletonmanager.hpp
    utils.cc
    utils.hpp
    utils_global.h
    utilstr.h
    widgetmanager.cc
    widgetmanager.hpp)

add_platform_library(utils ${PROJECT_SOURCES})
target_link_libraries(utils PUBLIC Qt::Widgets)

if(WIN32)
  target_link_libraries(utils PUBLIC dbghelp user32 shell32)
elseif(APPLE)
  find_library(Foundation_LIBRARY Foundation REQUIRED)
  find_library(AppKit_LIBRARY AppKit REQUIRED)
  target_link_libraries(utils PUBLIC ${Foundation_LIBRARY} ${AppKit_LIBRARY})
endif()
