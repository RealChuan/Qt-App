// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#pragma once

#include "hostosinfo.h"
#include "utils_global.h"

#include "filepath.h"

class tst_unixdevicefileaccess; // For testing.

namespace Utils {

class CommandLine;
class RunResult;
class TextCodec;

// Base class including dummy implementation usable as fallback.
class UTILS_EXPORT DeviceFileAccess
{
public:
    DeviceFileAccess();
    virtual ~DeviceFileAccess();

    virtual Environment deviceEnvironment() const;

protected:
    friend class FilePath;
    friend class ::tst_unixdevicefileaccess; // For testing.

    virtual QString mapToDevicePath(const QString &hostPath) const;

    virtual bool isExecutableFile(const FilePath &filePath) const;
    virtual bool isReadableFile(const FilePath &filePath) const;
    virtual bool isWritableFile(const FilePath &filePath) const;
    virtual bool isReadableDirectory(const FilePath &filePath) const;
    virtual bool isWritableDirectory(const FilePath &filePath) const;
    virtual bool isFile(const FilePath &filePath) const;
    virtual bool isDirectory(const FilePath &filePath) const;
    virtual bool isSymLink(const FilePath &filePath) const;
    virtual bool hasHardLinks(const FilePath &filePath) const;
    virtual Result<> ensureWritableDirectory(const FilePath &filePath) const;
    virtual bool ensureExistingFile(const FilePath &filePath) const;
    virtual bool createDirectory(const FilePath &filePath) const;
    virtual bool exists(const FilePath &filePath) const;
    virtual Result<> removeFile(const FilePath &filePath) const;
    virtual Result<> removeRecursively(const FilePath &filePath) const;
    virtual Result<> copyFile(const FilePath &filePath, const FilePath &target) const;
    virtual Result<> copyRecursively(const FilePath &filePath, const FilePath &target) const;
    virtual Result<> renameFile(const FilePath &filePath, const FilePath &target) const;

    virtual FilePath symLinkTarget(const FilePath &filePath) const;
    virtual FilePathInfo filePathInfo(const FilePath &filePath) const;
    virtual QDateTime lastModified(const FilePath &filePath) const;
    virtual QFile::Permissions permissions(const FilePath &filePath) const;
    virtual bool setPermissions(const FilePath &filePath, QFile::Permissions) const;
    virtual qint64 fileSize(const FilePath &filePath) const;
    virtual QString owner(const FilePath &filePath) const;
    virtual uint ownerId(const FilePath &filePath) const;
    virtual QString group(const FilePath &filePath) const;
    virtual uint groupId(const FilePath &filePath) const;
    virtual qint64 bytesAvailable(const FilePath &filePath) const;
    virtual QByteArray fileId(const FilePath &filePath) const;

    virtual std::optional<FilePath> refersToExecutableFile(const FilePath &filePath,
                                                           FilePath::MatchScope matchScope) const;

    virtual void iterateDirectory(const FilePath &filePath,
                                  const FilePath::IterateDirCallback &callBack,
                                  const FileFilter &filter) const;

    virtual Result<QByteArray> fileContents(const FilePath &filePath,
                                            qint64 limit,
                                            qint64 offset) const;

    virtual Result<qint64> writeFileContents(const FilePath &filePath, const QByteArray &data) const;

    virtual Result<FilePath> createTempFile(const FilePath &filePath);

    virtual Utils::Result<std::unique_ptr<FilePathWatcher>> watch(const FilePath &path) const;

    virtual TextCodec processStdOutCodec(const FilePath &executable) const;
    virtual TextCodec processStdErrCodec(const FilePath &executable) const;
};

class UTILS_EXPORT UnavailableDeviceFileAccess : public DeviceFileAccess
{
public:
    UnavailableDeviceFileAccess();
    ~UnavailableDeviceFileAccess() override;

protected:
    QString mapToDevicePath(const QString &hostPath) const override;

    Environment deviceEnvironment() const override;
    bool isExecutableFile(const FilePath &filePath) const override;
    bool isReadableFile(const FilePath &filePath) const override;
    bool isWritableFile(const FilePath &filePath) const override;
    bool isReadableDirectory(const FilePath &filePath) const override;
    bool isWritableDirectory(const FilePath &filePath) const override;
    bool isFile(const FilePath &filePath) const override;
    bool isDirectory(const FilePath &filePath) const override;
    bool isSymLink(const FilePath &filePath) const override;
    bool hasHardLinks(const FilePath &filePath) const override;
    Result<> ensureWritableDirectory(const FilePath &filePath) const override;
    bool ensureExistingFile(const FilePath &filePath) const override;
    bool createDirectory(const FilePath &filePath) const override;
    bool exists(const FilePath &filePath) const override;
    Result<> removeFile(const FilePath &filePath) const override;
    Result<> removeRecursively(const FilePath &filePath) const override;
    Result<> copyFile(const FilePath &filePath, const FilePath &target) const override;
    Result<> copyRecursively(const FilePath &filePath, const FilePath &target) const override;
    Result<> renameFile(const FilePath &filePath, const FilePath &target) const override;

    FilePath symLinkTarget(const FilePath &filePath) const override;
    FilePathInfo filePathInfo(const FilePath &filePath) const override;
    QDateTime lastModified(const FilePath &filePath) const override;
    QFile::Permissions permissions(const FilePath &filePath) const override;
    bool setPermissions(const FilePath &filePath, QFile::Permissions) const override;
    qint64 fileSize(const FilePath &filePath) const override;
    QString owner(const FilePath &filePath) const override;
    uint ownerId(const FilePath &filePath) const override;
    QString group(const FilePath &filePath) const override;
    uint groupId(const FilePath &filePath) const override;
    qint64 bytesAvailable(const FilePath &filePath) const override;
    QByteArray fileId(const FilePath &filePath) const override;

    std::optional<FilePath> refersToExecutableFile(const FilePath &filePath,
                                                   FilePath::MatchScope matchScope) const override;

    void iterateDirectory(const FilePath &filePath,
                          const FilePath::IterateDirCallback &callBack,
                          const FileFilter &filter) const override;

    Result<QByteArray> fileContents(const FilePath &filePath,
                                    qint64 limit,
                                    qint64 offset) const override;

    Result<qint64> writeFileContents(const FilePath &filePath,
                                     const QByteArray &data) const override;

    Result<FilePath> createTempFile(const FilePath &filePath) override;

    Utils::Result<std::unique_ptr<FilePathWatcher>> watch(const FilePath &path) const override;
};

class UTILS_EXPORT DesktopDeviceFileAccess : public DeviceFileAccess
{
public:
    DesktopDeviceFileAccess();
    ~DesktopDeviceFileAccess() override;

    static DesktopDeviceFileAccess *instance();

protected:
    bool isExecutableFile(const FilePath &filePath) const override;
    bool isReadableFile(const FilePath &filePath) const override;
    bool isWritableFile(const FilePath &filePath) const override;
    bool isReadableDirectory(const FilePath &filePath) const override;
    bool isWritableDirectory(const FilePath &filePath) const override;
    bool isFile(const FilePath &filePath) const override;
    bool isDirectory(const FilePath &filePath) const override;
    bool isSymLink(const FilePath &filePath) const override;
    bool hasHardLinks(const FilePath &filePath) const override;
    Result<> ensureWritableDirectory(const FilePath &filePath) const override;
    bool ensureExistingFile(const FilePath &filePath) const override;
    bool createDirectory(const FilePath &filePath) const override;
    bool exists(const FilePath &filePath) const override;
    Result<> removeFile(const FilePath &filePath) const override;
    Result<> removeRecursively(const FilePath &filePath) const override;
    Result<> copyFile(const FilePath &filePath, const FilePath &target) const override;
    Result<> renameFile(const FilePath &filePath, const FilePath &target) const override;

    FilePath symLinkTarget(const FilePath &filePath) const override;
    FilePathInfo filePathInfo(const FilePath &filePath) const override;
    QDateTime lastModified(const FilePath &filePath) const override;
    QFile::Permissions permissions(const FilePath &filePath) const override;
    bool setPermissions(const FilePath &filePath, QFile::Permissions) const override;
    qint64 fileSize(const FilePath &filePath) const override;
    QString owner(const FilePath &filePath) const override;
    uint ownerId(const FilePath &filePath) const override;
    QString group(const FilePath &filePath) const override;
    uint groupId(const FilePath &filePath) const override;
    qint64 bytesAvailable(const FilePath &filePath) const override;
    QByteArray fileId(const FilePath &filePath) const override;

    std::optional<FilePath> refersToExecutableFile(const FilePath &filePath,
                                                   FilePath::MatchScope matchScope) const override;

    void iterateDirectory(const FilePath &filePath,
                          const FilePath::IterateDirCallback &callBack,
                          const FileFilter &filter) const override;

    Environment deviceEnvironment() const override;

    Result<QByteArray> fileContents(const FilePath &filePath,
                                    qint64 limit,
                                    qint64 offset) const override;
    Result<qint64> writeFileContents(const FilePath &filePath,
                                     const QByteArray &data) const override;

    Result<FilePath> createTempFile(const FilePath &filePath) override;

    Utils::Result<std::unique_ptr<FilePathWatcher>> watch(const FilePath &path) const override;

    TextCodec processStdOutCodec(const FilePath &executable) const override;
    TextCodec processStdErrCodec(const FilePath &executable) const override;
};

class UTILS_EXPORT UnixDeviceFileAccess : public DeviceFileAccess
{
public:
    ~UnixDeviceFileAccess() override;

protected:
    virtual RunResult runInShell(const CommandLine &cmdLine, const QByteArray &inputData = {}) const
        = 0;
    Result<> runInShellSuccess(const CommandLine &cmdLine, const QByteArray &stdInData = {}) const;

    bool isExecutableFile(const FilePath &filePath) const override;
    bool isReadableFile(const FilePath &filePath) const override;
    bool isWritableFile(const FilePath &filePath) const override;
    bool isReadableDirectory(const FilePath &filePath) const override;
    bool isWritableDirectory(const FilePath &filePath) const override;
    bool isFile(const FilePath &filePath) const override;
    bool isDirectory(const FilePath &filePath) const override;
    bool isSymLink(const FilePath &filePath) const override;
    bool hasHardLinks(const FilePath &filePath) const override;
    bool ensureExistingFile(const FilePath &filePath) const override;
    bool createDirectory(const FilePath &filePath) const override;
    bool exists(const FilePath &filePath) const override;
    Result<> removeFile(const FilePath &filePath) const override;
    Result<> removeRecursively(const FilePath &filePath) const override;
    Result<> copyFile(const FilePath &filePath, const FilePath &target) const override;
    Result<> renameFile(const FilePath &filePath, const FilePath &target) const override;

    FilePathInfo filePathInfo(const FilePath &filePath) const override;
    FilePath symLinkTarget(const FilePath &filePath) const override;
    QDateTime lastModified(const FilePath &filePath) const override;
    QFile::Permissions permissions(const FilePath &filePath) const override;
    bool setPermissions(const FilePath &filePath, QFile::Permissions) const override;
    qint64 fileSize(const FilePath &filePath) const override;
    QString owner(const FilePath &filePath) const override;
    uint ownerId(const FilePath &filePath) const override;
    QString group(const FilePath &filePath) const override;
    uint groupId(const FilePath &filePath) const override;
    qint64 bytesAvailable(const FilePath &filePath) const override;
    QByteArray fileId(const FilePath &filePath) const override;

    void iterateDirectory(const FilePath &filePath,
                          const FilePath::IterateDirCallback &callBack,
                          const FileFilter &filter) const override;

    Environment deviceEnvironment() const override;
    Result<QByteArray> fileContents(const FilePath &filePath,
                                    qint64 limit,
                                    qint64 offset) const override;
    Result<qint64> writeFileContents(const FilePath &filePath,
                                     const QByteArray &data) const override;

    Result<FilePath> createTempFile(const FilePath &filePath) override;

    void findUsingLs(const QString &current,
                     const FileFilter &filter,
                     QStringList *found,
                     const QString &start) const;

private:
    bool iterateWithFind(const FilePath &filePath,
                         const FileFilter &filter,
                         const FilePath::IterateDirCallback &callBack) const;

    QStringList statArgs(const FilePath &filePath,
                         const QString &linuxFormat,
                         const QString &macFormat) const;

    mutable bool m_tryUseFind = true;
    mutable std::optional<bool> m_hasMkTemp;
};

} // namespace Utils
