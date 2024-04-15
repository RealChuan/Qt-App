#ifndef RESOURCE_GLOBAL_HPP
#define RESOURCE_GLOBAL_HPP

#include <QtCore/qglobal.h>

#if defined(RESOURCE_LIBRARY)
#  define RESOURCE_EXPORT Q_DECL_EXPORT
#else
#  define RESOURCE_EXPORT Q_DECL_IMPORT
#endif

#endif // RESOURCE_GLOBAL_HPP
