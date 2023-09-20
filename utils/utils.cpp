#include "utils.h"
#include "hostosinfo.h"

#include <QTextCodec>
#include <QtWidgets>

#ifdef Q_OS_WIN
#include <windows.h>

#include <tlhelp32.h>
#endif

void Utils::setUTF8Code()
{
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
}

void Utils::setQSS(const QStringList &qssFilePaths)
{
    QString qss;
    for (const auto &path : qAsConst(qssFilePaths)) {
        qDebug() << QObject::tr("Loading QSS file: %1.").arg(path);
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qDebug() << QObject::tr("Cannot open the file: %1!").arg(path) << file.errorString();
            continue;
        }
        qss.append(QLatin1String(file.readAll())).append("\n");
        file.close();
    }
    if (qss.isEmpty()) {
        return;
    }
    qApp->setStyleSheet(qss);
}

void Utils::loadFonts(const QString &fontPath)
{
    const QDir dir(fontPath);

    if (!dir.exists()) {
        return;
    }
    // QFontDatabase::removeAllApplicationFonts();
    const auto fonts = dir.entryInfoList(QStringList("*.ttf"), QDir::Files);
    for (const auto &fileInfo : qAsConst(fonts)) {
        int fontId = QFontDatabase::addApplicationFont(fileInfo.absoluteFilePath());
        if (fontId == -1) {
            qWarning() << QString("Loading Fonts file: %1 Failed.").arg(fileInfo.fileName());
        } else {
            qDebug() << QString("Loading Fonts file: %1.").arg(fileInfo.fileName())
                     << QFontDatabase::applicationFontFamilies(fontId);
        }
    }
}

void Utils::windowCenter(QWidget *child, QWidget *parent)
{
    const QSize size = parent->size() - child->size();
    int x = qMax(0, size.width() / 2);
    int y = qMax(0, size.height() / 2);
    child->move(x, y);
}

void Utils::windowCenter(QWidget *window)
{
    const QRect rect = qApp->primaryScreen()->availableGeometry();
    int x = (rect.width() - window->width()) / 2 + rect.x();
    int y = (rect.height() - window->height()) / 2 + rect.y();
    x = qMax(0, x);
    y = qMax(0, y);
    window->move(x, y);
}

auto compilerString() -> QString
{
#if defined(__apple_build_version__) // Apple clang has other version numbers
    QString isAppleString = QLatin1String(" (Apple)");
    return QLatin1String("Clang ") + QString::number(__clang_major__) + QLatin1Char('.')
           + QString::number(__clang_minor__) + isAppleString;
#elif defined(Q_CC_GNU)
    return QLatin1String("GCC ") + QLatin1String(__VERSION__);
#elif defined(Q_CC_MSVC)
    return QString("MSVC Version: %1").arg(_MSC_VER);
#endif
    return QLatin1String("<unknown compiler>");
}

void Utils::printBuildInfo()
{
    //    qInfo() << QSysInfo::buildAbi() << QSysInfo::machineUniqueId()
    //            << QOperatingSystemVersion::current();
    const QString info = QString("Qt %1 (%2, %3 bit)")
                             .arg(qVersion(), compilerString(), QString::number(QSysInfo::WordSize));
    qInfo() << QObject::tr("Build with: ") << info;
}

void Utils::setHighDpiEnvironmentVariable()
{
    if (Utils::HostOsInfo::isMacHost()) {
        return;
    }

    if (Utils::HostOsInfo::isWindowsHost()
        && !qEnvironmentVariableIsSet("QT_DEVICE_PIXEL_RATIO") // legacy in 5.6, but still functional
        && !qEnvironmentVariableIsSet("QT_AUTO_SCREEN_SCALE_FACTOR")
        && !qEnvironmentVariableIsSet("QT_SCALE_FACTOR")
        && !qEnvironmentVariableIsSet("QT_SCREEN_SCALE_FACTORS")) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    }

#if defined(Q_OS_WIN) && (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(
        Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif
}

void Utils::reboot()
{
    QProcess::startDetached(QApplication::applicationFilePath(),
                            QApplication::arguments(),
                            QDir::currentPath());
    QApplication::exit();
}

auto calculateDir(const QString &localPath) -> qint64
{
    qint64 size = 0;
    QDir dir(localPath);
    if (!dir.exists()) {
        return size;
    }
    QFileInfoList list = dir.entryInfoList(QDir::AllEntries | QDir::Hidden | QDir::NoDotAndDotDot);
    for (int i = 0; i < list.count(); i++) {
        QFileInfo info = list.at(i);
        if (info.isDir()) {
            size += calculateDir(info.filePath());
        } else {
            size += info.size();
        }
    }
    return size;
}

auto Utils::fileSize(const QString &localPath) -> qint64
{
    QFileInfo info(localPath);
    if (info.isDir()) {
        return calculateDir(localPath);
    }
    return info.size();
}

auto Utils::generateDirectorys(const QString &directory) -> bool
{
    QDir sourceDir(directory);
    if (sourceDir.exists()) {
        return true;
    }

    QString tempDir;
    auto directorys = directory.split("/");
    for (int i = 0; i < directorys.count(); i++) {
        auto path = directorys[i];
        tempDir += path + "/";

        QDir dir(tempDir);
        if (!dir.exists() && !dir.mkdir(tempDir)) {
            return false;
        }
    }

    return true;
}

void removeFiles(const QString &path)
{
    QDir dir(path);
    if (!dir.exists()) {
        return;
    }
    const QFileInfoList entries = dir.entryInfoList(QDir::AllEntries | QDir::Hidden);
    for (const QFileInfo &fi : qAsConst(entries)) {
        if (fi.isSymLink() || fi.isFile()) {
            QFile f(fi.filePath());
            if (!f.remove()) {
                const QString errorMessage = QObject::tr("Cannot remove file \"%1\": %2")
                                                 .arg(QDir::toNativeSeparators(f.fileName()),
                                                      f.errorString());
                qWarning() << errorMessage;
            }
        }
    }
}

static auto errnoToQString(int error) -> QString
{
#if defined(Q_OS_WIN) && !defined(Q_CC_MINGW)
    char msg[128];
    if (strerror_s(msg, sizeof msg, error) != 0)
        return QString::fromLocal8Bit(msg);
    return QString();
#else
    return QString::fromLocal8Bit(strerror(error));
#endif
}

void Utils::removeDirectory(const QString &path)
{
    if (path.isEmpty()) { // QDir("") points to the working directory! We never want to remove that one.
        return;
    }

    QStringList dirs;
    QDirIterator it(path,
                    QDir::NoDotAndDotDot | QDir::Dirs | QDir::NoSymLinks | QDir::Hidden,
                    QDirIterator::Subdirectories);
    while (it.hasNext()) {
        dirs.prepend(it.next());
        removeFiles(dirs.at(0));
    }

    QDir d;
    dirs.append(path);
    removeFiles(path);
    for (const QString &dir : qAsConst(dirs)) {
        errno = 0;
        if (d.exists(path) && !d.rmdir(dir)) {
            const QString errorMessage = QObject::tr("Cannot remove directory \"%1\": %2")
                                             .arg(QDir::toNativeSeparators(dir),
                                                  errnoToQString(errno));
            qWarning() << errorMessage;
        }
    }
}

auto Utils::convertBytesToString(qint64 bytes) -> QString
{
    const QStringList list = {"B", "KB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB"};
    const int unit = 1024;
    int index = 0;
    double size = bytes;
    while (size >= unit) {
        size /= unit;
        index++;
    }
    return QString("%1 %2").arg(QString::number(size, 'f', 2)).arg(list.at(index));
}

QJsonObject Utils::jsonFromFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << QObject::tr("Cannot open the file: %1").arg(filePath);
        return QJsonObject();
    }
    const QByteArray buf(file.readAll());
    file.close();
    return jsonFromBytes(buf);
}

QJsonObject Utils::jsonFromBytes(const QByteArray &bytes)
{
    QJsonParseError jsonParseError;
    auto jsonDocument = QJsonDocument::fromJson(bytes, &jsonParseError);
    if (QJsonParseError::NoError != jsonParseError.error) {
        qWarning() << QObject::tr("%1\nOffset: %2")
                          .arg(jsonParseError.errorString(), jsonParseError.offset);
        return QJsonObject();
    }
    return jsonDocument.object();
}

void Utils::setGlobalThreadPoolMaxSize(int maxSize)
{
    auto instance = QThreadPool::globalInstance();
    if (maxSize > 0) {
        instance->setMaxThreadCount(maxSize);
        return;
    }
    instance->setMaxThreadCount(qMax(4, 2 * instance->maxThreadCount()));
}

auto Utils::getConfigPath() -> QString
{
    static QString path;
    if (path.isEmpty()) {
        path = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
        if (path.isEmpty()) {
            path = QDir::homePath();
        }
        if (!path.endsWith(qAppName())) {
            path = path + "/" + qAppName();
        }
    }
    //qInfo() << path;
    Utils::generateDirectorys(path);
    return path;
}

auto Utils::execMenuAtWidget(QMenu *menu, QWidget *widget) -> QAction *
{
    QPoint p;
    QRect screen = widget->screen()->availableGeometry();
    QSize sh = menu->sizeHint();
    QRect rect = widget->rect();
    if (widget->isRightToLeft()) {
        if (widget->mapToGlobal(QPoint(0, rect.bottom())).y() + sh.height() <= screen.height()) {
            p = widget->mapToGlobal(rect.bottomRight());
        } else {
            p = widget->mapToGlobal(rect.topRight() - QPoint(0, sh.height()));
        }
        p.rx() -= sh.width();
    } else {
        if (widget->mapToGlobal(QPoint(0, rect.bottom())).y() + sh.height() <= screen.height()) {
            p = widget->mapToGlobal(rect.bottomLeft());
        } else {
            p = widget->mapToGlobal(rect.topLeft() - QPoint(0, sh.height()));
        }
    }
    p.rx() = qMax(screen.left(), qMin(p.x(), screen.right() - sh.width()));
    p.ry() += 1;

    return menu->exec(p);
}

auto Utils::getPidFromProcessName(const QString &processName) -> qint64
{
#if defined(Q_OS_WIN)
    auto *hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return -1;
    }
    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(PROCESSENTRY32);
    if (!Process32First(hSnapshot, &pe)) {
        CloseHandle(hSnapshot);
        return -1;
    }
    do {
        if (QString::fromWCharArray(pe.szExeFile) == processName) {
            CloseHandle(hSnapshot);
            return pe.th32ProcessID;
        }
    } while (Process32Next(hSnapshot, &pe));
    CloseHandle(hSnapshot);
    return -1;
#else
    // 创建一个QProcess对象
    QProcess process;
    // 启动ps -A命令
    process.start("ps -A");
    // 等待命令执行完成
    process.waitForFinished();
    // 读取命令输出并转换为QString
    auto output = QString::fromLocal8Bit(process.readAllStandardOutput());
    // 按行分割输出
    auto lines = output.split("\n");
    // 创建一个正则表达式对象，匹配以空格+processName结尾的字符串
    QRegularExpression re("\\s" + processName + "$");
    // 遍历输出的每一行
    for (const auto &line : lines) {
        // 如果匹配成功
        if (re.match(line).hasMatch()) {
            // 按空格分割字符串
            auto parts = line.split(" ");
            // 返回pid
            return parts.at(0).toLongLong();
        }
    }
    return -1;
#endif
}

auto Utils::killProcess(qint64 pid) -> bool
{
    qWarning() << "kill process: " << pid;
#if defined(Q_OS_WIN)
    auto *hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
    if (hProcess == nullptr) {
        return false;
    }
    auto result = TerminateProcess(hProcess, 0);
    CloseHandle(hProcess);
    return result != 0;
#else
    return QProcess::execute("kill", QStringList() << QString::number(pid)) == 0;
#endif
}

void Utils::setMacComboBoxStyle(QWidget *parent)
{
#ifndef Q_OS_MACOS
    return;
#endif
    auto comboBoxs = parent->findChildren<QComboBox *>();
    for (const auto comboBox : comboBoxs) {
        comboBox->setStyle(QStyleFactory::create("Fusion"));
    }
}
