#include "utils.hpp"

#include "hostosinfo.h"
#include "utilstr.h"

#include <QtWidgets>

// clang-format off
#ifdef Q_OS_WIN
#include <windows.h>
#include <tlhelp32.h>
#endif
// clang-format on

#include <array>
#include <memory>
#include <ranges>
#include <vector>

using namespace Qt::StringLiterals;

namespace Utils {

namespace {

// Windows 句柄用 RAII 管住，避免每个提前返回分支都要手写 CloseHandle。
#ifdef Q_OS_WIN
using UniqueHandle = std::unique_ptr<void, decltype(&::CloseHandle)>;

auto makeHandle(HANDLE handle) -> UniqueHandle
{
    return UniqueHandle(handle, &::CloseHandle);
}
#endif

auto ensureSubDirectory(const QString &name) -> QString
{
    const QString path = QDir(configLocation()).filePath(name);
    QDir().mkpath(path);
    return path;
}

} // namespace

void setQSS(const QStringList &qssFilePaths)
{
    QStringList sheets;
    for (const QString &path : qssFilePaths) {
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qWarning().noquote()
                << u"Cannot open the file: %1 (%2)"_s.arg(path, file.errorString());
            continue;
        }
        qInfo().noquote() << u"Load QSS: %1"_s.arg(path);
        sheets.append(QString::fromUtf8(file.readAll()));
    }

    if (sheets.isEmpty())
        return;
    qApp->setStyleSheet(sheets.join(u"\n"_s));
}

void loadFonts(const QString &fontPath)
{
    const QDir dir(fontPath);
    if (!dir.exists())
        return;

    const QFileInfoList fonts = dir.entryInfoList({u"*.ttf"_s}, QDir::Files);
    for (const QFileInfo &info : fonts) {
        const int fontId = QFontDatabase::addApplicationFont(info.absoluteFilePath());
        if (fontId == -1) {
            qWarning().noquote() << u"Loading Fonts file: %1 Failed."_s.arg(info.fileName());
            continue;
        }
        qInfo().noquote() << u"Loading Fonts file: %1."_s.arg(info.fileName())
                          << QFontDatabase::applicationFontFamilies(fontId);
    }
}

void windowCenter(QWidget *child, QWidget *parent)
{
    const QRect target = QStyle::alignedRect(Qt::LeftToRight,
                                             Qt::AlignCenter,
                                             child->size(),
                                             parent->rect());
    child->move(target.topLeft());
}

void windowCenter(QWidget *window)
{
    const QScreen *screen = window->screen() ? window->screen() : QGuiApplication::primaryScreen();
    if (!screen)
        return;

    const QRect target = QStyle::alignedRect(Qt::LeftToRight,
                                             Qt::AlignCenter,
                                             window->size(),
                                             screen->availableGeometry());
    window->move(target.topLeft());
}

auto compilerString() -> QString
{
#if defined(__apple_build_version__) // Apple clang has other version numbers
    return u"Clang %1.%2 (Apple)"_s.arg(__clang_major__).arg(__clang_minor__);
#elif defined(Q_CC_GNU)
    return u"GCC %1"_s.arg(QLatin1StringView(__VERSION__));
#elif defined(Q_CC_MSVC)
    return u"MSVC Version: %1"_s.arg(_MSC_VER);
#else
    return u"<unknown compiler>"_s;
#endif
}

static QWidget *(*s_dialogParentGetter)() = nullptr;

void setDialogParentGetter(QWidget *(*getter)())
{
    s_dialogParentGetter = getter;
}

auto dialogParent() -> QWidget *
{
    return s_dialogParentGetter ? s_dialogParentGetter() : nullptr;
}

void restoreAndActivate(QWidget *window)
{
    if (window->isMinimized())
        window->setWindowState(window->windowState() & ~Qt::WindowMinimized);

    window->show();
    window->raise();
    window->activateWindow();
}

void addGraphicsDropShadowEffect(QWidget *widget, int blurRadius)
{
    auto *effect = new QGraphicsDropShadowEffect(widget);
    effect->setOffset(0, 0);
    effect->setColor(Qt::gray);
    effect->setBlurRadius(blurRadius);
    widget->setGraphicsEffect(effect);
}

auto systemInfo() -> QString
{
    const QStringList lines{
        u"%1 (%2) on %3 (%4) with CPU Cores: %5"_s.arg(QSysInfo::prettyProductName(),
                                                       QSysInfo::kernelVersion(),
                                                       QSysInfo::currentCpuArchitecture(),
                                                       QSysInfo::machineHostName(),
                                                       QString::number(QThread::idealThreadCount())),
        u"Build with: Qt %1 (%2, %3)"_s.arg(QString::fromLatin1(qVersion()),
                                            compilerString(),
                                            QSysInfo::buildAbi()),
    };
    return lines.join(u"\n"_s);
}

void setHighDpiEnvironmentVariable()
{
#ifdef Q_OS_WIN
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(
        Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif
}

void reboot()
{
    QProcess::startDetached(QApplication::applicationFilePath(),
                            QApplication::arguments(),
                            QDir::currentPath());
    QApplication::exit();
}

auto createDirectoryRecursively(const QString &path) -> bool
{
    return QDir().mkpath(path);
}

auto removeDirectory(const QString &path) -> bool
{
    // QDir("") 指向工作目录，永远不要删它。
    if (path.isEmpty())
        return false;

    QDir dir(path);
    if (!dir.exists()) {
        qWarning().noquote() << Tr::tr("Directory does not exist: %1").arg(path);
        return false;
    }
    return dir.removeRecursively();
}

auto removeFile(const QString &path) -> bool
{
    if (path.isEmpty())
        return false;
    if (!QFile::exists(path)) {
        qWarning().noquote() << Tr::tr("File does not exist: %1").arg(path);
        return false;
    }
    return QFile::remove(path);
}

auto formatBytes(qint64 bytes, int precision) -> QString
{
    static constexpr std::array<const char *, 9> units{"B", "KB", "MB", "GB", "TB",
                                                       "PB", "EB", "ZB", "YB"};
    constexpr double unit = 1024.0;

    double size = static_cast<double>(bytes);
    std::size_t index = 0;
    while (size >= unit && index + 1 < units.size()) {
        size /= unit;
        ++index;
    }

    return u"%1 %2"_s.arg(QString::number(size, 'f', precision),
                          QLatin1StringView(units[index]));
}

auto jsonFromFile(const QString &filePath) -> Result<QJsonObject>
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
        return ResultError(Tr::tr("Cannot open the file: %1").arg(filePath));
    return jsonFromBytes(file.readAll());
}

auto jsonFromBytes(const QByteArray &bytes) -> Result<QJsonObject>
{
    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(bytes, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        return ResultError(Tr::tr("%1\nOffset: %2")
                               .arg(parseError.errorString(), QString::number(parseError.offset)));
    }
    return document.object();
}

auto configLocation() -> QString
{
    // 函数内 static 的初始化是线程安全的，顺带把路径创建也一次性做掉。
    static const QString location = [] {
        QString path = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
        if (path.isEmpty())
            path = QDir::homePath();
        QDir().mkpath(path);
        return path;
    }();
    return location;
}

auto getPidFromProcessName(QStringView processName) -> qint64
{
#if defined(Q_OS_WIN)
    const UniqueHandle snapshot = makeHandle(CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0));
    if (!snapshot || snapshot.get() == INVALID_HANDLE_VALUE)
        return -1;

    PROCESSENTRY32 entry{};
    entry.dwSize = sizeof(PROCESSENTRY32);
    if (!Process32First(snapshot.get(), &entry))
        return -1;

    do {
        if (QString::fromWCharArray(entry.szExeFile) == processName)
            return entry.th32ProcessID;
    } while (Process32Next(snapshot.get(), &entry));
    return -1;
#else
    QProcess process;
    process.start(u"ps"_s, {u"-A"_s});
    if (!process.waitForFinished())
        return -1;

    const QString output = QString::fromLocal8Bit(process.readAllStandardOutput());
    const QRegularExpression pattern(u"\\s%1$"_s.arg(QRegularExpression::escape(
        processName.toString())));
    const QStringList lines = output.split(u'\n');
    for (const QString &line : lines) {
        if (!pattern.match(line).hasMatch())
            continue;
        const QStringList parts = line.split(u' ', Qt::SkipEmptyParts);
        if (!parts.isEmpty())
            return parts.constFirst().toLongLong();
    }
    return -1;
#endif
}

auto killProcess(qint64 pid) -> bool
{
    qWarning().noquote() << "kill process:" << pid;
#if defined(Q_OS_WIN)
    const UniqueHandle process = makeHandle(OpenProcess(PROCESS_TERMINATE, FALSE, DWORD(pid)));
    if (!process)
        return false;
    return TerminateProcess(process.get(), 0) != 0;
#else
    return QProcess::execute(u"kill"_s, {QString::number(pid)}) == 0;
#endif
}

void setMacComboBoxStyle(QWidget *parent)
{
#ifdef Q_OS_MACOS
    const auto comboBoxes = parent->findChildren<QComboBox *>();
    for (QComboBox *comboBox : comboBoxes)
        comboBox->setStyle(QStyleFactory::create(u"Fusion"_s));
#else
    Q_UNUSED(parent)
#endif
}

void quitApplication()
{
    QMetaObject::invokeMethod(qApp, &QApplication::quit, Qt::QueuedConnection);
}

auto crashPath() -> QString
{
    return ensureSubDirectory(u"crash"_s);
}

auto logPath() -> QString
{
    return ensureSubDirectory(u"log"_s);
}

auto configPath() -> QString
{
    return ensureSubDirectory(u"config"_s);
}

auto configFilePath() -> QString
{
    return QDir(configPath()).filePath(u"config.ini"_s);
}

auto cachePath() -> QString
{
    return ensureSubDirectory(u"cache"_s);
}

auto calculateDirectoryStats(const QString &path) -> DirectoryStats
{
    DirectoryStats stats;
    if (path.isEmpty() || !QDir(path).exists())
        return stats;

    QDirIterator it(path,
                    QDir::AllEntries | QDir::Hidden | QDir::NoDotAndDotDot,
                    QDirIterator::Subdirectories);
    while (it.hasNext()) {
        const QFileInfo info(it.next());
        if (info.isDir()) {
            ++stats.folders;
        } else {
            ++stats.files;
            stats.size += info.size();
        }
    }
    return stats;
}

namespace {

auto generateRandomData(int size) -> QByteArray
{
    const auto numWords = static_cast<std::size_t>((size + sizeof(quint32) - 1) / sizeof(quint32));
    std::vector<quint32> buffer(numWords);
    QRandomGenerator::global()->fillRange(buffer.data(), qsizetype(numWords));

    QByteArray data(reinterpret_cast<const char *>(buffer.data()),
                    qsizetype(numWords * sizeof(quint32)));
    data.resize(size);
    return data;
}

auto cpuBenchOnce(int durationMs,
                  QCryptographicHash::Algorithm algorithm,
                  const QByteArray &data) -> double
{
    QElapsedTimer timer;
    timer.start();

    qint64 totalBytes = 0;
    while (timer.elapsed() < durationMs) {
        const QByteArray hash = QCryptographicHash::hash(data, algorithm);
        totalBytes += data.size() + hash.size();
    }

    const double elapsedSeconds = timer.elapsed() / 1000.0;
    return elapsedSeconds <= 0.0 ? 0.0 : (double(totalBytes) / elapsedSeconds) / (1024.0 * 1024.0);
}

} // namespace

auto cpuBench(int iterations, int durationMs, int dataSize, QCryptographicHash::Algorithm algorithm)
    -> double
{
    double best = 0.0;
    for (int i = 0; i < iterations; ++i) {
        const double speed = cpuBenchOnce(durationMs, algorithm, generateRandomData(dataSize));
        best = (std::ranges::max)(best, speed);
    }
    return best;
}

void setPixmapCacheLimit()
{
    const int originalLimit = QPixmapCache::cacheLimit();
    const qreal dpr = qApp->devicePixelRatio();
    const qreal multiplier = std::clamp(dpr * dpr, 1.0, 4.0);
    QPixmapCache::setCacheLimit(qRound(originalLimit * multiplier));
}

} // namespace Utils
