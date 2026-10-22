#pragma once

#include <QtCore/qglobal.h>

// 目录名是 3rdparty、target 是 thirdparty，这里统一成 THIRDPARTY（修正历史拼写 THRIDPARTY）。
#if defined(THIRDPARTY_LIBRARY)
#  define THIRDPARTY_EXPORT Q_DECL_EXPORT
#elif defined(THIRDPARTY_STATIC_LIBRARY)
#  define THIRDPARTY_EXPORT
#else
#  define THIRDPARTY_EXPORT Q_DECL_IMPORT
#endif
