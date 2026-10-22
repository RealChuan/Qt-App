#pragma once

#include <QtCore/qglobal.h>

#if defined(SHELL_LIBRARY)
#define SHELL_EXPORT Q_DECL_EXPORT
#else
#define SHELL_EXPORT Q_DECL_IMPORT
#endif
