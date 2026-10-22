#pragma once

// =============================================================================
// 可选预编译头（默认关闭），两套构建系统共用：
//   CMake : -DQT_APP_PCH=ON        （target_precompile_headers）
//   qmake : qmake QT_APP_PCH=1     （PRECOMPILED_HEADER）
//
// 只放最稳定的 Qt 模块头，改动此文件会导致全量重编。
// 有意不包含 <QtCore> 之外的标准库头：不同编译器/标准库版本的 PCH 兼容性差异大。
// =============================================================================

#include <QtCore/QtCore>
#include <QtGui/QtGui>
#include <QtNetwork/QtNetwork>
#include <QtWidgets/QtWidgets>
