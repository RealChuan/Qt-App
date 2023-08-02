set(PROJECT_SOURCES
    mimetypes/mimedatabase_p.h
    mimetypes/mimedatabase.cpp
    mimetypes/mimedatabase.h
    mimetypes/mimeglobpattern_p.h
    mimetypes/mimeglobpattern.cpp
    mimetypes/mimemagicrule_p.h
    mimetypes/mimemagicrule.cpp
    mimetypes/mimemagicrulematcher_p.h
    mimetypes/mimemagicrulematcher.cpp
    mimetypes/mimeprovider_p.h
    mimetypes/mimeprovider.cpp
    mimetypes/mimetype_p.h
    mimetypes/mimetype.cpp
    mimetypes/mimetype.h
    mimetypes/mimetypeparser_p.h
    mimetypes/mimetypeparser.cpp
    mimetypes/mimeutils.cpp
    algorithm.h
    benchmarker.cpp
    benchmarker.h
    countdownlatch.cc
    countdownlatch.hpp
    executeondestruction.h
    hostosinfo.cpp
    hostosinfo.h
    indexedcontainerproxyconstiterator.h
    languageconfig.cc
    languageconfig.hpp
    logasync.cpp
    logasync.h
    mimeutils.h
    osspecificaspects.h
    predicates.h
    qtcassert.cpp
    qtcassert.h
    qtcsettings.cpp
    qtcsettings.h
    singleton.hpp
    stringutils.cpp
    stringutils.h
    taskqueue.h
    utils_global.h
    utils.cpp
    utils.h
    utilstr.h
    validator.cc
    validator.hpp)

add_custom_library(utils ${PROJECT_SOURCES})
target_link_libraries(utils PRIVATE Qt6::Widgets Qt6::Core5Compat)

if(CMAKE_HOST_WIN32)
  target_compile_definitions(utils PRIVATE "UTILS_LIBRARY")
  target_link_libraries(utils PRIVATE dbghelp)
endif()
