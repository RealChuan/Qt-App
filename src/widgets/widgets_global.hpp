#pragma once

#include <QtCore/qglobal.h>

#if defined(WIDGETS_LIBRARY)
#define WIDGETS_EXPORT Q_DECL_EXPORT
#elif defined(WIDGETS_STATIC_LIBRARY)
#define WIDGETS_EXPORT
#else
#define WIDGETS_EXPORT Q_DECL_IMPORT
#endif
