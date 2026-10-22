#pragma once

#include <QtCore/qglobal.h>

#if defined(RESOURCE_LIBRARY)
#define RESOURCE_EXPORT Q_DECL_EXPORT
#elif defined(RESOURCE_STATIC_LIBRARY)
#define RESOURCE_EXPORT
#else
#define RESOURCE_EXPORT Q_DECL_IMPORT
#endif
