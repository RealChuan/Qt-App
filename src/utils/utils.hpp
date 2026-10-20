#pragma once

#include "utils_global.h"

#include "result.h"

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

[[nodiscard]] UTILS_EXPORT auto configLocation() -> QString;
[[nodiscard]] UTILS_EXPORT auto configPath() -> QString;
[[nodiscard]] UTILS_EXPORT auto configFilePath() -> QString;
[[nodiscard]] UTILS_EXPORT auto crashPath() -> QString;
[[nodiscard]] UTILS_EXPORT auto logPath() -> QString;
[[nodiscard]] UTILS_EXPORT auto cachePath() -> QString;

UTILS_EXPORT void restoreAndActivate(QWidget *window);
UTILS_EXPORT void addGraphicsDropShadowEffect(QWidget *widget, int blurRadius = 10);

// 对话框默认父窗口：由 core 插件在启动时注入。
[[nodiscard]] UTILS_EXPORT auto dialogParent() -> QWidget *;
UTILS_EXPORT void setDialogParentGetter(QWidget *(*getter)());
[[nodiscard]] UTILS_EXPORT auto systemInfo() -> QString;
UTILS_EXPORT void setHighDpiEnvironmentVariable();
UTILS_EXPORT void quitApplication();
UTILS_EXPORT void setQSS(const QStringList &qssFilePaths);
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
[[nodiscard]] UTILS_EXPORT auto cpuBench(
    int iterations,
    int durationMs,
    int dataSize = 1024 * 1024,
    QCryptographicHash::Algorithm algorithm = QCryptographicHash::Sha256) -> double;

} // namespace Utils
