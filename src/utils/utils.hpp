#pragma once

#include "utils_global.hpp"

#include "result.hpp"

#include <QByteArrayView>
#include <QJsonObject>
#include <QString>
#include <QStringView>
#include <QtCore>

class QWidget;
class QAction;
class QMenu;

namespace Utils {

struct UTILS_EXPORT DirectoryStats
{
    qint64 folders = 0;
    qint64 files = 0;
    qint64 size = 0;
};
[[nodiscard]] UTILS_EXPORT auto calculateDirectoryStats(const QString &path) -> DirectoryStats;

UTILS_EXPORT void restoreAndActivate(QWidget *window);
UTILS_EXPORT void addGraphicsDropShadowEffect(QWidget *widget, int blurRadius = 10);

// 对话框默认父窗口：由 core 插件在启动时注入。
[[nodiscard]] UTILS_EXPORT auto dialogParent() -> QWidget *;
UTILS_EXPORT void setDialogParentGetter(QWidget *(*getter)());
[[nodiscard]] UTILS_EXPORT auto systemInfo() -> QString;
// 设置高 DPI 缩放策略（名字里没有"环境变量"这回事）
UTILS_EXPORT void setupHighDpi();
UTILS_EXPORT void quitApplication();
UTILS_EXPORT void setPixmapCacheLimit();
UTILS_EXPORT void loadFonts(const QString &fontPath);
UTILS_EXPORT void windowCenter(QWidget *child, QWidget *parent);
UTILS_EXPORT void windowCenter(QWidget *window);
UTILS_EXPORT void reboot();
[[nodiscard]] UTILS_EXPORT auto createDirectoryRecursively(const QString &path) -> bool;
[[nodiscard]] UTILS_EXPORT auto removeDirectory(const QString &path) -> bool;
[[nodiscard]] UTILS_EXPORT auto removeFile(const QString &path) -> bool;
[[nodiscard]] UTILS_EXPORT auto formatBytes(qint64 bytes, int precision = 2) -> QString;
[[nodiscard]] UTILS_EXPORT auto jsonFromFile(const QString &filePath) -> Result<QJsonObject>;
[[nodiscard]] UTILS_EXPORT auto jsonFromBytes(const QByteArray &bytes) -> Result<QJsonObject>;
UTILS_EXPORT void setMacComboBoxStyle(QWidget *parent);
[[nodiscard]] UTILS_EXPORT auto getPidFromProcessName(QStringView processName) -> qint64;
[[nodiscard]] UTILS_EXPORT auto killProcess(qint64 pid) -> bool;
// 单线程哈希吞吐：在 durationMs 内反复计算同一块数据，返回 MiB/s，多轮取最好值。
// 走的是 Qt 自带实现，数字只适合同版本、同参数下横向比较，不代表 CPU 峰值。
[[nodiscard]] UTILS_EXPORT auto hashBench(
    int iterations,
    int durationMs,
    int dataSize = 1024 * 1024,
    QCryptographicHash::Algorithm algorithm = QCryptographicHash::Sha256) -> double;

} // namespace Utils
