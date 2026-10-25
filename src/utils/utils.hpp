#pragma once

#include "utils_global.hpp"

#include <QCryptographicHash>
#include <QString>
#include <QStringView>

namespace Utils {

[[nodiscard]] UTILS_EXPORT auto systemInfo() -> QString;
UTILS_EXPORT void quitApplication();

// 重启请求：这里只置标记，真正的重启由 main() 在事件循环退出、日志收尾之后做。
// 不能当场拉新进程——那时老进程还握着单实例锁，新进程会被当成第二个实例踢掉。
UTILS_EXPORT void requestRestart();
[[nodiscard]] UTILS_EXPORT auto restartRequested() -> bool;

[[nodiscard]] UTILS_EXPORT auto formatBytes(qint64 bytes, int precision = 2) -> QString;

// 单线程哈希吞吐：在 durationMs 内反复计算同一块数据，返回 MiB/s，多轮取最好值。
// 走的是 Qt 自带实现，数字只适合同版本、同参数下横向比较，不代表 CPU 峰值。
[[nodiscard]] UTILS_EXPORT auto hashBench(
    int iterations,
    int durationMs,
    int dataSize = 1024 * 1024,
    QCryptographicHash::Algorithm algorithm = QCryptographicHash::Sha256) -> double;

} // namespace Utils
