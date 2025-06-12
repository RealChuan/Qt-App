#pragma once

#include <QtCore/qglobal.h>

#if defined(UTILS_LIBRARY)
#define UTILS_EXPORT Q_DECL_EXPORT
#elif defined(UTILS_STATIC_LIBRARY)
#define UTILS_EXPORT
#else
#define UTILS_EXPORT Q_DECL_IMPORT
#endif
