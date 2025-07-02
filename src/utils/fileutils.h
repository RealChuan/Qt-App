// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#pragma once

#include "utils_global.h"

#include "filepath.h"
#include "result.h"

#ifdef QT_WIDGETS_LIB
#include <QFileDialog>
#endif

#include <functional>
#include <memory>

QT_BEGIN_NAMESPACE
class QDataStream;
class QTextStream;
class QXmlStreamWriter;

// for withNtfsPermissions
#ifdef Q_OS_WIN
extern Q_CORE_EXPORT int qt_ntfs_permission_lookup;
#endif
QT_END_NAMESPACE

namespace Utils {
namespace FileUtils {

using CopyHelper = std::function<bool(const FilePath &, const FilePath &, QString *)>;
#ifdef QT_GUI_LIB
class UTILS_EXPORT CopyAskingForOverwrite
{
public:
    explicit CopyAskingForOverwrite(const std::function<void(FilePath)> &postOperation = {});
    CopyHelper operator()();
    FilePaths files() const;

private:
    FilePaths m_files;
    std::function<void(FilePath)> m_postOperation;
    bool m_overwriteAll = false;
    bool m_skipAll = false;
};
#endif // QT_GUI_LIB

UTILS_EXPORT bool copyRecursively(const FilePath &srcFilePath,
                                  const FilePath &tgtFilePath,
                                  QString *error,
                                  CopyHelper helper);

UTILS_EXPORT Result<> copyIfDifferent(const FilePath &srcFilePath, const FilePath &tgtFilePath);
UTILS_EXPORT QString fileSystemFriendlyName(const QString &name);
UTILS_EXPORT int indexOfQmakeUnfriendly(const QString &name, int startpos = 0);
UTILS_EXPORT QString qmakeFriendlyName(const QString &name);
UTILS_EXPORT QString normalizedPathName(const QString &name);

UTILS_EXPORT FilePath commonPath(const FilePath &oldCommonPath, const FilePath &fileName);
UTILS_EXPORT FilePath commonPath(const FilePaths &paths);
UTILS_EXPORT FilePath homePath();
UTILS_EXPORT Result<FilePath> scratchBufferFilePath(const QString &pattern);

UTILS_EXPORT FilePaths toFilePathList(const QStringList &paths);

UTILS_EXPORT qint64 bytesAvailableFromDFOutput(const QByteArray &dfOutput);

UTILS_EXPORT FilePathInfo filePathInfoFromTriple(const QString &infos, int modeBase);

//! Returns known paths like /opt/homebrew on macOS that might not be in PATH
UTILS_EXPORT FilePaths usefulExtraSearchPaths();

#ifdef QT_WIDGETS_LIB
UTILS_EXPORT bool hasNativeFileDialog();

UTILS_EXPORT FilePath getOpenFilePath(const QString &caption,
                                      const FilePath &dir = {},
                                      const QString &filter = {},
                                      QString *selectedFilter = nullptr,
                                      QFileDialog::Options options = {},
                                      bool fromDeviceIfShiftIsPressed = false,
                                      bool forceNonNativeDialog = false);

UTILS_EXPORT FilePath getSaveFilePath(const QString &caption,
                                      const FilePath &dir = {},
                                      const QString &filter = {},
                                      QString *selectedFilter = nullptr,
                                      QFileDialog::Options options = {},
                                      bool forceNonNativeDialog = false);

UTILS_EXPORT FilePath getExistingDirectory(const QString &caption,
                                           const FilePath &dir = {},
                                           QFileDialog::Options options = QFileDialog::ShowDirsOnly,
                                           bool fromDeviceIfShiftIsPressed = false,
                                           bool forceNonNativeDialog = false);

UTILS_EXPORT FilePaths getOpenFilePaths(const QString &caption,
                                        const FilePath &dir = {},
                                        const QString &filter = {},
                                        QString *selectedFilter = nullptr,
                                        QFileDialog::Options options = {});

UTILS_EXPORT void showError(const QString &errorMessage);

#endif

UTILS_EXPORT QString fetchQrc(const QString &fileName); // Only for internal resourcesm

} // namespace FileUtils

// for actually finding out if e.g. directories are writable on Windows
#ifdef Q_OS_WIN

template<typename T>
T withNtfsPermissions(const std::function<T()> &task)
{
    qt_ntfs_permission_lookup++;
    T result = task();
    qt_ntfs_permission_lookup--;
    return result;
}

template<>
UTILS_EXPORT void withNtfsPermissions(const std::function<void()> &task);

#else // Q_OS_WIN

template<typename T>
T withNtfsPermissions(const std::function<T()> &task)
{
    return task();
}

#endif // Q_OS_WIN

class UTILS_EXPORT FileSaverBase
{
public:
    FileSaverBase();
    virtual ~FileSaverBase();

    FilePath filePath() const { return m_filePath; }
    bool hasError() const { return !m_result; }
    QString errorString() const { return m_result.error(); }
    virtual Utils::Result<> finalize();

    bool write(QByteArrayView bytes);
    bool setResult(QTextStream *stream);
    bool setResult(QDataStream *stream);
    bool setResult(QXmlStreamWriter *stream);
    bool setResult(bool ok);

    QFile *file() { return m_file.get(); }

protected:
    std::unique_ptr<QFile> m_file;
    FilePath m_filePath;
    Result<> m_result;

private:
    Q_DISABLE_COPY(FileSaverBase)
};

class UTILS_EXPORT FileSaver : public FileSaverBase
{
public:
    // QIODevice::WriteOnly is implicit
    explicit FileSaver(const FilePath &filePath, QIODevice::OpenMode mode = QIODevice::NotOpen);

    Utils::Result<> finalize() override;
    using FileSaverBase::finalize;

private:
    bool m_isSafe = false;
};

class UTILS_EXPORT TempFileSaver : public FileSaverBase
{
public:
    explicit TempFileSaver(const QString &templ = QString());
    explicit TempFileSaver(const FilePath &templ);
    ~TempFileSaver() override;

    void setAutoRemove(bool on) { m_autoRemove = on; }

protected:
    void initFromString(const QString &templ);

private:
    bool m_autoRemove = true;
};

UTILS_EXPORT QTextStream &operator<<(QTextStream &s, const FilePath &fn);

} // namespace Utils
