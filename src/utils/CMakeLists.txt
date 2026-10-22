set(PROJECT_SOURCES
    algorithm.hpp
    appinfo.cc
    appinfo.hpp
    asynclog.cc
    asynclog.hpp
    filepath.cc
    filepath.hpp
    hostosinfo.cc
    hostosinfo.hpp
    languagemanager.cc
    languagemanager.hpp
    logbuffer.cc
    logbuffer.hpp
    logsink.cc
    logsink.hpp
    osspecificaspects.hpp
    predicates.hpp
    qtcassert.cc
    qtcassert.hpp
    result.cc
    result.hpp
    settings.cc
    settings.hpp
    singleton.hpp
    singletonmanager.cc
    singletonmanager.hpp
    theme.cc
    theme.hpp
    utils.cc
    utils.hpp
    utils_global.hpp
    utilstr.hpp
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
