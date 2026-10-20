#include "filepath.h"

#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QUrl>

#include <algorithm>
#include <memory>

namespace Utils {

FileFilter::FileFilter(const QStringList &nameFilters,
                       const QDir::Filters fileFilters,
                       const QDirIterator::IteratorFlags flags)
    : nameFilters(nameFilters)
    , fileFilters(fileFilters)
    , iteratorFlags(flags)
{}

static QString cleaned(const QString &input)
{
    QString path = input;
    path.replace(QLatin1Char('\\'), QLatin1Char('/'));

    const bool unc = path.startsWith(QLatin1String("//"));
    path = QDir::cleanPath(path);
    if (unc && !path.startsWith(QLatin1String("//")))
        path.prepend(QLatin1Char('/'));

    return path;
}

FilePath FilePath::fromString(const QString &path)
{
    return FilePath(cleaned(path));
}

FilePath FilePath::fromUserInput(const QString &path)
{
    QString expanded = path;
    if (expanded == QLatin1String("~"))
        expanded = QDir::homePath();
    else if (expanded.startsWith(QLatin1String("~/")))
        expanded = QDir::homePath() + expanded.mid(1);

    return FilePath(cleaned(expanded));
}

FilePath FilePath::fromFileInfo(const QFileInfo &info)
{
    return FilePath(cleaned(info.absoluteFilePath()));
}

FilePath FilePath::fromVariant(const QVariant &variant)
{
    if (variant.canConvert<FilePath>())
        return variant.value<FilePath>();
    return FilePath(cleaned(variant.toString()));
}

FilePath FilePath::fromUrl(const QUrl &url)
{
    if (url.isLocalFile())
        return FilePath(cleaned(url.toLocalFile()));
    return FilePath(cleaned(url.toString()));
}

FilePath FilePath::currentWorkingPath()
{
    return FilePath(cleaned(QDir::currentPath()));
}

QString FilePath::nativePath() const
{
#ifdef Q_OS_WIN
    QString native = m_path;
    native.replace(QLatin1Char('/'), QLatin1Char('\\'));
    return native;
#else
    return m_path;
#endif
}

QString FilePath::toUserOutput() const
{
    return nativePath();
}

QVariant FilePath::toVariant() const
{
    return QVariant::fromValue(*this);
}

QUrl FilePath::toUrl() const
{
    if (isResourceFile())
        return QUrl(QLatin1String("qrc") + m_path);
    return QUrl::fromLocalFile(m_path);
}

QString FilePath::fileName() const
{
    const qsizetype index = m_path.lastIndexOf(QLatin1Char('/'));
    return index < 0 ? m_path : m_path.mid(index + 1);
}

QString FilePath::baseName() const
{
    const QString name = fileName();
    const qsizetype index = name.indexOf(QLatin1Char('.'));
    return index < 0 ? name : name.left(index);
}

QString FilePath::completeBaseName() const
{
    const QString name = fileName();
    const qsizetype index = name.lastIndexOf(QLatin1Char('.'));
    return index < 0 ? name : name.left(index);
}

QString FilePath::suffix() const
{
    const QString name = fileName();
    const qsizetype index = name.indexOf(QLatin1Char('.'));
    return index < 0 ? QString() : name.mid(index + 1);
}

QString FilePath::completeSuffix() const
{
    const QString name = fileName();
    const qsizetype index = name.lastIndexOf(QLatin1Char('.'));
    return index < 0 ? QString() : name.mid(index + 1);
}

QString FilePath::fileNameWithPathComponents(int pathComponents) const
{
    const QList<QStringView> components = this->pathComponents();
    if (pathComponents <= 0 || pathComponents > components.size())
        return fileName();

    QString result;
    for (qsizetype i = components.size() - pathComponents; i < components.size(); ++i) {
        if (!result.isEmpty())
            result += QLatin1Char('/');
        result += components.at(i).toString();
    }
    return result;
}

QList<QStringView> FilePath::pathComponents() const
{
    QList<QStringView> result;
    const QStringView path(m_path);
    qsizetype start = 0;
    while (start < path.size()) {
        const qsizetype end = path.indexOf(QLatin1Char('/'), start);
        const QStringView component = end < 0 ? path.mid(start) : path.mid(start, end - start);
        if (!component.isEmpty())
            result.append(component);
        if (end < 0)
            break;
        start = end + 1;
    }
    return result;
}

FilePath FilePath::pathAppended(const QString &str) const
{
    if (str.isEmpty())
        return *this;

    QString other = str;
    other.replace(QLatin1Char('\\'), QLatin1Char('/'));

    if (isEmpty())
        return FilePath(cleaned(other));

    // 绝对路径直接替换
    if (QDir::isAbsolutePath(other))
        return FilePath(cleaned(other));

    QString result = m_path;
    if (result.endsWith(QLatin1Char('/')))
        result.chop(1);
    result += QLatin1Char('/');
    result += other;
    return FilePath(cleaned(result));
}

FilePath FilePath::stringAppended(const QString &str) const
{
    return FilePath(m_path + str);
}

FilePath FilePath::parentDir() const
{
    if (m_path.isEmpty())
        return {};

    qsizetype index = m_path.lastIndexOf(QLatin1Char('/'));
    if (index < 0)
        return {};
    if (index == 0)
        return FilePath(QStringLiteral("/"));
    if (index == 2 && m_path.size() > 1 && m_path.at(1) == QLatin1Char(':'))
        return FilePath(m_path.left(3));

    return FilePath(m_path.left(index));
}

bool FilePath::isChildOf(const FilePath &parent) const
{
    if (parent.isEmpty() || m_path.size() <= parent.m_path.size())
        return false;

    const Qt::CaseSensitivity cs = parent.osType() == OsTypeWindows ? Qt::CaseInsensitive
                                                                    : Qt::CaseSensitive;
    if (!m_path.startsWith(parent.m_path, cs))
        return false;

    if (parent.m_path.endsWith(QLatin1Char('/')))
        return true;
    return m_path.at(parent.m_path.size()) == QLatin1Char('/');
}

FilePath FilePath::relativeChildPath(const FilePath &parent) const
{
    if (!isChildOf(parent))
        return {};
    return FilePath(m_path.mid(parent.m_path.size() + 1));
}

bool FilePath::exists() const
{
    if (m_path.isEmpty())
        return false;
    if (isResourceFile())
        return QFile::exists(m_path);
    return QFileInfo::exists(m_path);
}

bool FilePath::isFile() const
{
    return !m_path.isEmpty() && QFileInfo(m_path).isFile();
}

bool FilePath::isDir() const
{
    return !m_path.isEmpty() && QFileInfo(m_path).isDir();
}

bool FilePath::isSymLink() const
{
    return !m_path.isEmpty() && QFileInfo(m_path).isSymLink();
}

bool FilePath::isExecutableFile() const
{
    return !m_path.isEmpty() && QFileInfo(m_path).isExecutable() && isFile();
}

bool FilePath::isReadableFile() const
{
    return !m_path.isEmpty() && QFileInfo(m_path).isReadable() && isFile();
}

bool FilePath::isReadableDir() const
{
    return !m_path.isEmpty() && QFileInfo(m_path).isReadable() && isDir();
}

bool FilePath::isWritableDir() const
{
    return !m_path.isEmpty() && QFileInfo(m_path).isWritable() && isDir();
}

bool FilePath::isWritableFile() const
{
    return !m_path.isEmpty() && QFileInfo(m_path).isWritable() && isFile();
}

bool FilePath::isRelativePath() const
{
    return !isAbsolutePath();
}

bool FilePath::isAbsolutePath() const
{
    if (isResourceFile())
        return true;
    return QDir::isAbsolutePath(m_path);
}

bool FilePath::isRootPath() const
{
    if (m_path == QLatin1String("/"))
        return true;
    return m_path.size() == 3 && m_path.at(1) == QLatin1Char(':') && m_path.at(2) == QLatin1Char('/');
}

bool FilePath::isResourceFile() const
{
    return m_path.startsWith(QLatin1Char(':'));
}

bool FilePath::isNewerThan(const QDateTime &timeStamp) const
{
    return lastModified() > timeStamp;
}

bool FilePath::createDir() const
{
    if (m_path.isEmpty())
        return false;
    return QDir().mkpath(m_path);
}

Result<> FilePath::ensureWritableDir() const
{
    if (isDir())
        return isWritableDir() ? ResultOk : ResultError(QStringLiteral("Directory is not writable"));
    if (!createDir())
        return ResultError(QStringLiteral("Cannot create directory"));
    return ResultOk;
}

bool FilePath::ensureExistingFile() const
{
    if (isFile())
        return true;
    QFile file(m_path);
    return file.open(QIODevice::WriteOnly);
}

Result<> FilePath::removeFile() const
{
    if (!isFile())
        return ResultError(QStringLiteral("Not a file"));
    return QFile::remove(m_path) ? ResultOk : ResultError(QStringLiteral("Cannot remove file"));
}

Result<> FilePath::removeRecursively() const
{
    if (!isDir())
        return ResultError(QStringLiteral("Not a directory"));
    return QDir(m_path).removeRecursively() ? ResultOk
                                            : ResultError(QStringLiteral("Cannot remove directory"));
}

Result<> FilePath::copyFile(const FilePath &target) const
{
    const FilePath destination = target.isDir() ? target / fileName() : target;
    if (destination.exists() && !destination.isDir() && !QFile::remove(destination.path()))
        return ResultError(QStringLiteral("Cannot overwrite target"));
    return QFile::copy(m_path, destination.path())
               ? ResultOk
               : ResultError(QStringLiteral("Cannot copy file"));
}

Result<> FilePath::copyRecursively(const FilePath &target) const
{
    if (!isDir())
        return ResultError(QStringLiteral("Not a directory"));

    if (!QDir().mkpath(target.path()))
        return ResultError(QStringLiteral("Cannot create target directory"));

    const FilePaths entries = dirEntries(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot
                                         | QDir::Hidden | QDir::System);
    for (const FilePath &entry : entries) {
        const FilePath destination = target / entry.fileName();
        if (entry.isDir()) {
            if (const Result<> result = entry.copyRecursively(destination); !result)
                return result;
        } else if (const Result<> result = entry.copyFile(destination); !result) {
            return result;
        }
    }
    return ResultOk;
}

Result<> FilePath::renameFile(const FilePath &target) const
{
    const FilePath destination = target.isDir() ? target / fileName() : target;
    return QFile::rename(m_path, destination.path())
               ? ResultOk
               : ResultError(QStringLiteral("Cannot rename"));
}

Result<> FilePath::createSymLink(const FilePath &symLink) const
{
#ifdef Q_OS_WIN
    Q_UNUSED(symLink)
    return ResultError(QStringLiteral("Symlinks are not supported"));
#else
    return QFile::link(m_path, symLink.path()) ? ResultOk
                                               : ResultError(QStringLiteral("Cannot create link"));
#endif
}

qint64 FilePath::fileSize() const
{
    return QFileInfo(m_path).size();
}

qint64 FilePath::bytesAvailable() const
{
    return fileSize();
}

QDateTime FilePath::lastModified() const
{
    return QFileInfo(m_path).lastModified();
}

QFile::Permissions FilePath::permissions() const
{
    return QFileInfo(m_path).permissions();
}

Result<> FilePath::setPermissions(QFile::Permissions permissions) const
{
    return QFile::setPermissions(m_path, permissions) ? ResultOk
                                                      : ResultError(QStringLiteral("Cannot set permissions"));
}

Result<> FilePath::makeWritable() const
{
    if (isWritableFile() || isWritableDir())
        return ResultOk;
    return setPermissions(permissions() | QFile::WriteOwner);
}

Result<QByteArray> FilePath::fileContents(qint64 maxSize, qint64 offset) const
{
    QFile file(m_path);
    if (!file.open(QIODevice::ReadOnly))
        return ResultError(QStringLiteral("Cannot open file"));

    if (offset > 0 && !file.seek(offset))
        return ResultError(QStringLiteral("Cannot seek in file"));

    const QByteArray contents = maxSize < 0 ? file.readAll() : file.read(maxSize);
    if (maxSize >= 0 && contents.size() < maxSize && file.error() != QFileDevice::NoError)
        return ResultError(QStringLiteral("Cannot read file"));

    return contents;
}

Result<qint64> FilePath::writeFileContents(const QByteArray &data) const
{
    QFile file(m_path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
        return ResultError(QStringLiteral("Cannot open file for writing"));

    const qint64 written = file.write(data);
    if (written != data.size())
        return ResultError(QStringLiteral("Cannot write file"));
    return written;
}

FilePaths FilePath::dirEntries(QDir::Filters filters) const
{
    FilePaths result;
    const QDir dir(m_path);
    const QFileInfoList entries = dir.entryInfoList(filters, QDir::NoSort);
    for (const QFileInfo &info : entries)
        result.append(FilePath::fromFileInfo(info));
    return result;
}

FilePaths FilePath::dirEntries(const FileFilter &filter, QDir::SortFlags sort) const
{
    FilePaths result;

    if (filter.iteratorFlags == QDirIterator::NoIteratorFlags) {
        const QDir dir(m_path);
        const QFileInfoList entries = dir.entryInfoList(filter.nameFilters,
                                                        filter.fileFilters,
                                                        sort);
        for (const QFileInfo &info : entries)
            result.append(FilePath::fromFileInfo(info));
        return result;
    }

    QDirIterator it(m_path, filter.nameFilters, filter.fileFilters, filter.iteratorFlags);
    while (it.hasNext())
        result.append(FilePath::fromString(it.next()));
    return result;
}

FilePath FilePath::absoluteFilePath() const
{
    if (m_path.isEmpty() || isAbsolutePath())
        return *this;
    return FilePath(cleaned(QDir::current().absoluteFilePath(m_path)));
}

FilePath FilePath::absolutePath() const
{
    return absoluteFilePath().parentDir();
}

FilePath FilePath::cleanPath() const
{
    return FilePath(cleaned(m_path));
}

FilePath FilePath::canonicalPath() const
{
    const QFileInfo info(m_path);
    if (!info.exists())
        return cleanPath().absoluteFilePath();
    const QString canonical = info.canonicalFilePath();
    return canonical.isEmpty() ? cleanPath().absoluteFilePath() : FilePath(cleaned(canonical));
}

FilePath FilePath::withExecutableSuffix() const
{
    return FilePath(cleaned(OsSpecificAspects::withExecutableSuffix(osType(), m_path)));
}

FilePath FilePath::withSuffix(const QString &suffix) const
{
    return FilePath(m_path + suffix);
}

QString FilePath::displayName(const QString &args) const
{
    return args.isEmpty() ? fileName() : fileName() + QLatin1Char(' ') + args;
}

Result<FilePath> FilePath::tmpDir() const
{
    return FilePath::fromString(QDir::tempPath());
}

Result<FilePath> FilePath::createTempFile() const
{
    auto tempFile = std::make_unique<QTemporaryFile>(QDir::tempPath() + QStringLiteral("/qtc-XXXXXX"));
    if (!tempFile->open())
        return ResultError(QStringLiteral("Cannot create temporary file"));
    const QString name = tempFile->fileName();
    tempFile->setAutoRemove(false);
    return FilePath::fromString(name);
}

Result<FilePath> FilePath::createTempDir() const
{
    auto tempDir = std::make_unique<QTemporaryDir>(QDir::tempPath() + QStringLiteral("/qtc-XXXXXX"));
    if (!tempDir->isValid())
        return ResultError(QStringLiteral("Cannot create temporary directory"));
    const QString name = tempDir->path();
    tempDir->setAutoRemove(false);
    return FilePath::fromString(name);
}

OsType FilePath::osType() const
{
#if defined(Q_OS_WIN)
    return OsTypeWindows;
#elif defined(Q_OS_MACOS)
    return OsTypeMac;
#elif defined(Q_OS_LINUX)
    return OsTypeLinux;
#elif defined(Q_OS_UNIX)
    return OsTypeOtherUnix;
#else
    return OsTypeOther;
#endif
}

int FilePath::compare(const FilePath &other) const
{
    const Qt::CaseSensitivity cs = osType() == OsTypeWindows ? Qt::CaseInsensitive
                                                             : Qt::CaseSensitive;
    return m_path.compare(other.m_path, cs);
}

QDebug operator<<(QDebug dbg, const FilePath &path)
{
    dbg << path.path();
    return dbg;
}

FilePaths FilePaths::fromStrings(const QStringList &fileNames)
{
    FilePaths result;
    result.reserve(fileNames.size());
    for (const QString &fileName : fileNames)
        result.append(FilePath::fromString(fileName));
    return result;
}

FilePaths FilePaths::fromSettings(const QVariant &variant)
{
    FilePaths result;
    for (const QVariant &entry : variant.toList())
        result.append(FilePath::fromVariant(entry));
    return result;
}

QVariant FilePaths::toSettings() const
{
    QVariantList list;
    list.reserve(size());
    for (const FilePath &path : *this)
        list.append(path.toVariant());
    return list;
}

QStringList FilePaths::toFsPathStrings() const
{
    QStringList result;
    result.reserve(size());
    for (const FilePath &path : *this)
        result.append(path.toFSPathString());
    return result;
}

QString FilePaths::toUserOutput(const QString &separator) const
{
    QStringList result;
    result.reserve(size());
    for (const FilePath &path : *this)
        result.append(path.toUserOutput());
    return result.join(separator);
}

FilePath FilePaths::commonPath() const
{
    if (isEmpty())
        return {};

    FilePath result = first();
    for (const FilePath &path : *this) {
        while (!path.isChildOf(result) && path != result)
            result = result.parentDir();
        if (result.isEmpty())
            return {};
    }
    return result;
}

void FilePaths::sort()
{
    std::sort(begin(), end());
}

} // namespace Utils
