#pragma once

#include "utils_global.h"

#include "osspecificaspects.h"
#include "result.h"

#include <QDir>
#include <QDirIterator>
#include <QMetaType>
#include <QString>

QT_BEGIN_NAMESPACE
class QDateTime;
class QDebug;
class QFileInfo;
class QUrl;
QT_END_NAMESPACE

class QDebug;

namespace Utils {

class FilePaths;
class FilePath;

UTILS_EXPORT QDebug operator<<(QDebug dbg, const FilePath &path);

class UTILS_EXPORT FileFilter
{
public:
    FileFilter(const QStringList &nameFilters,
               QDir::Filters fileFilters = QDir::NoFilter,
               QDirIterator::IteratorFlags flags = QDirIterator::NoIteratorFlags);

    const QStringList nameFilters;
    const QDir::Filters fileFilters = QDir::NoFilter;
    const QDirIterator::IteratorFlags iteratorFlags = QDirIterator::NoIteratorFlags;
};

// 本地路径的轻量封装：内部只保存一个以 '/' 分隔的路径字符串。
// 远程/设备路径（scheme://host）支持已移除，本项目只需要本地文件与 qrc 资源。
class UTILS_EXPORT FilePath
{
public:
    FilePath() = default;
    explicit FilePath(const QString &path) : m_path(path) {}

    template<size_t N>
    FilePath(const char (&literal)[N])
        : m_path(QString::fromUtf8(literal))
    {}

    [[nodiscard]] static FilePath fromString(const QString &path);
    [[nodiscard]] static FilePath fromUserInput(const QString &path);
    [[nodiscard]] static FilePath fromFileInfo(const QFileInfo &info);
    [[nodiscard]] static FilePath fromVariant(const QVariant &variant);
    [[nodiscard]] static FilePath fromUrl(const QUrl &url);
    [[nodiscard]] static FilePath currentWorkingPath();

    QString path() const { return m_path; }
    QString toUrlishString() const { return m_path; }
    QString toFSPathString() const { return m_path; }
    QString nativePath() const;
    QString toUserOutput() const;
    QVariant toVariant() const;
    QUrl toUrl() const;

    QString fileName() const;
    QString baseName() const;
    QString completeBaseName() const;
    QString suffix() const;
    QString completeSuffix() const;
    QString fileNameWithPathComponents(int pathComponents) const;
    QList<QStringView> pathComponents() const;

    [[nodiscard]] FilePath pathAppended(const QString &str) const;
    [[nodiscard]] FilePath stringAppended(const QString &str) const;
    bool startsWith(const QString &s) const { return m_path.startsWith(s); }
    bool endsWith(const QString &s) const { return m_path.endsWith(s); }
    bool contains(const QString &s) const { return m_path.contains(s); }

    [[nodiscard]] FilePath parentDir() const;
    bool isChildOf(const FilePath &parent) const;
    [[nodiscard]] FilePath relativeChildPath(const FilePath &parent) const;

    bool exists() const;
    bool isFile() const;
    bool isDir() const;
    bool isSymLink() const;
    bool isExecutableFile() const;
    bool isReadableFile() const;
    bool isReadableDir() const;
    bool isWritableDir() const;
    bool isWritableFile() const;
    bool isRelativePath() const;
    bool isAbsolutePath() const;
    bool isRootPath() const;
    bool isResourceFile() const;
    bool isNewerThan(const QDateTime &timeStamp) const;

    bool createDir() const;
    Result<> ensureWritableDir() const;
    bool ensureExistingFile() const;

    Result<> removeFile() const;
    Result<> removeRecursively() const;
    Result<> copyFile(const FilePath &target) const;
    Result<> copyRecursively(const FilePath &target) const;
    Result<> renameFile(const FilePath &target) const;
    Result<> createSymLink(const FilePath &symLink) const;

    qint64 fileSize() const;
    qint64 bytesAvailable() const;
    QDateTime lastModified() const;
    QFile::Permissions permissions() const;
    Result<> setPermissions(QFile::Permissions permissions) const;
    Result<> makeWritable() const;

    Result<QByteArray> fileContents(qint64 maxSize = -1, qint64 offset = 0) const;
    Result<qint64> writeFileContents(const QByteArray &data) const;

    FilePaths dirEntries(QDir::Filters filters) const;
    FilePaths dirEntries(const FileFilter &filter, QDir::SortFlags sort = QDir::NoSort) const;

    [[nodiscard]] FilePath absoluteFilePath() const;
    [[nodiscard]] FilePath absolutePath() const;
    [[nodiscard]] FilePath cleanPath() const;
    [[nodiscard]] FilePath canonicalPath() const;
    [[nodiscard]] FilePath withExecutableSuffix() const;
    [[nodiscard]] FilePath withSuffix(const QString &suffix) const;

    QString displayName(const QString &args = {}) const;

    [[nodiscard]] Result<FilePath> tmpDir() const;
    [[nodiscard]] Result<FilePath> createTempFile() const;
    [[nodiscard]] Result<FilePath> createTempDir() const;

    void clear() { m_path.clear(); }
    bool isEmpty() const { return m_path.isEmpty(); }
    bool isLocal() const { return true; }
    OsType osType() const;

    [[nodiscard]] FilePath operator/(const QString &str) const { return pathAppended(str); }
    FilePath &operator/=(const QString &str)
    {
        *this = pathAppended(str);
        return *this;
    }

    friend bool operator==(const FilePath &first, const FilePath &second)
    {
        return first.compare(second) == 0;
    }
    friend bool operator!=(const FilePath &first, const FilePath &second)
    {
        return !(first == second);
    }
    friend bool operator<(const FilePath &first, const FilePath &second)
    {
        return first.compare(second) < 0;
    }

    friend size_t qHash(const FilePath &path, size_t seed = 0)
    {
        return ::qHash(path.m_path, seed);
    }

private:
    int compare(const FilePath &other) const;

    QString m_path;
};

class UTILS_EXPORT FilePaths final : public QList<FilePath>
{
public:
    using QList<FilePath>::QList;
    FilePaths(const QList<FilePath> &filePaths)
        : QList<FilePath>(filePaths)
    {}

    [[nodiscard]] static FilePaths fromStrings(const QStringList &fileNames);
    [[nodiscard]] static FilePaths fromSettings(const QVariant &variant);

    [[nodiscard]] QVariant toSettings() const;
    [[nodiscard]] QStringList toFsPathStrings() const;
    [[nodiscard]] QString toUserOutput(const QString &separator) const;
    [[nodiscard]] FilePath commonPath() const;
    void sort();
};

} // namespace Utils

Q_DECLARE_METATYPE(Utils::FilePath)
