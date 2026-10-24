#pragma once

#include "utils_global.hpp"

#include <QCryptographicHash>
#include <QString>
#include <QStringView>

// 与 QWidget / QPainter 打交道的自由函数在 uihelpers.hpp。
// 这里只留没有平台分支、确实在用的几件：系统信息、退出/重启、字节格式化、哈希基准。
// 目录与文件的增删走 FilePath（返回 Result<>），不再提供 bool 版的自由函数。

namespace Utils {

[[nodiscard]] UTILS_EXPORT auto systemInfo() -> QString;
UTILS_EXPORT void quitApplication();
UTILS_EXPORT void reboot();
[[nodiscard]] UTILS_EXPORT auto formatBytes(qint64 bytes, int precision = 2) -> QString;

// 单线程哈希吞吐：在 durationMs 内反复计算同一块数据，返回 MiB/s，多轮取最好值。
// 走的是 Qt 自带实现，数字只适合同版本、同参数下横向比较，不代表 CPU 峰值。
[[nodiscard]] UTILS_EXPORT auto hashBench(
    int iterations,
    int durationMs,
    int dataSize = 1024 * 1024,
    QCryptographicHash::Algorithm algorithm = QCryptographicHash::Sha256) -> double;

} // namespace Utils
