#ifndef GUI_GLOBAL_HPP
#define GUI_GLOBAL_HPP

#include <QtCore/qglobal.h>

#if defined(GUI_LIBRARY)
#  define GUI_EXPORT Q_DECL_EXPORT
#else
#  define GUI_EXPORT Q_DECL_IMPORT
#endif

#endif // GUI_GLOBAL_HPP
