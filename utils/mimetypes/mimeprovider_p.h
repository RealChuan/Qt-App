// Copyright (C) 2016 The Qt Company Ltd.
// Copyright (C) 2015 Klaralvdalens Datakonsult AB, a KDAB Group company, info@kdab.com, author David Faure <david.faure@kdab.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#pragma once

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "mimedatabase_p.h"

#include "mimeglobpattern_p.h"
#include <QtCore/qdatetime.h>
#include <QtCore/qset.h>

namespace Utils {

class MimeMagicRuleMatcher;

class MimeProviderBase
{
public:
    MimeProviderBase(MimeDatabasePrivate *db, const QString &directory);
    virtual ~MimeProviderBase() {}

    virtual auto isValid() -> bool = 0;
    [[nodiscard]] virtual auto isInternalDatabase() const -> bool = 0;
    virtual auto mimeTypeForName(const QString &name) -> MimeType = 0;
    virtual void addFileNameMatches(const QString &fileName, MimeGlobMatchResult &result) = 0;
    virtual void addParents(const QString &mime, QStringList &result) = 0;
    virtual auto resolveAlias(const QString &name) -> QString = 0;
    virtual void addAliases(const QString &name, QStringList &result) = 0;
    virtual void findByMagic(const QByteArray &data, int *accuracyPtr, MimeType &candidate) = 0;
    virtual void addAllMimeTypes(QList<MimeType> &result) = 0;
    virtual auto loadMimeTypePrivate(MimeTypePrivate &) -> bool { return false; }
    virtual void loadIcon(MimeTypePrivate &) {}
    virtual void loadGenericIcon(MimeTypePrivate &) {}
    virtual void ensureLoaded() {}

    [[nodiscard]] auto directory() const -> QString { return m_directory; }

    // added for Qt Creator
    virtual auto hasMimeTypeForName(const QString &name) -> bool = 0;
    virtual auto allMimeTypeNames() -> QStringList = 0;
    [[nodiscard]] virtual auto magicRulesForMimeType(const MimeType &mimeType) const -> QMap<int, QList<MimeMagicRule>> = 0;
    virtual void setMagicRulesForMimeType(const MimeType &mimeType,
                                          const QMap<int, QList<MimeMagicRule>> &rules) = 0;
    virtual void setGlobPatternsForMimeType(const MimeType &mimeType, const QStringList &patterns)
        = 0;

    MimeDatabasePrivate *m_db;
    QString m_directory;
    QSet<QString> m_overriddenMimeTypes;
};

/*
   Parses the files 'mime.cache' and 'types' on demand
 */
class MimeBinaryProvider : public MimeProviderBase
{
public:
    MimeBinaryProvider(MimeDatabasePrivate *db, const QString &directory);
    ~MimeBinaryProvider() override;

    auto isValid() -> bool override;
    [[nodiscard]] auto isInternalDatabase() const -> bool override;
    auto mimeTypeForName(const QString &name) -> MimeType override;
    void addFileNameMatches(const QString &fileName, MimeGlobMatchResult &result) override;
    void addParents(const QString &mime, QStringList &result) override;
    auto resolveAlias(const QString &name) -> QString override;
    void addAliases(const QString &name, QStringList &result) override;
    void findByMagic(const QByteArray &data, int *accuracyPtr, MimeType &candidate) override;
    void addAllMimeTypes(QList<MimeType> &result) override;
    auto loadMimeTypePrivate(MimeTypePrivate &) -> bool override;
    void loadIcon(MimeTypePrivate &) override;
    void loadGenericIcon(MimeTypePrivate &) override;
    void ensureLoaded() override;

    // added for Qt Creator
    auto hasMimeTypeForName(const QString &name) -> bool override;
    auto allMimeTypeNames() -> QStringList override;
    [[nodiscard]] auto magicRulesForMimeType(const MimeType &mimeType) const -> QMap<int, QList<MimeMagicRule>> override;
    void setMagicRulesForMimeType(const MimeType &mimeType,
                                  const QMap<int, QList<MimeMagicRule>> &rules) override;
    void setGlobPatternsForMimeType(const MimeType &mimeType, const QStringList &patterns) override;

private:
    struct CacheFile;

    void matchGlobList(MimeGlobMatchResult &result,
                       CacheFile *cacheFile,
                       int offset,
                       const QString &fileName);
    auto matchSuffixTree(MimeGlobMatchResult &result,
                         CacheFile *cacheFile,
                         int numEntries,
                         int firstOffset,
                         const QString &fileName,
                         int charPos,
                         bool caseSensitiveCheck) -> bool;
    auto matchMagicRule(CacheFile *cacheFile, int numMatchlets, int firstOffset, const QByteArray &data) -> bool;
    auto iconForMime(CacheFile *cacheFile, int posListOffset, const QByteArray &inputMime) -> QLatin1String;
    void loadMimeTypeList();
    auto checkCacheChanged() -> bool;

    CacheFile *m_cacheFile = nullptr;
    QStringList m_cacheFileNames;
    QSet<QString> m_mimetypeNames;
    bool m_mimetypeListLoaded;
    struct MimeTypeExtra
    {
        QHash<QString, QString> localeComments;
        QStringList globPatterns;
    };
    QMap<QString, MimeTypeExtra> m_mimetypeExtra;
};

/*
   Parses the raw XML files (slower)
 */
class MimeXMLProvider : public MimeProviderBase
{
public:
    enum InternalDatabaseEnum { InternalDatabase };
#if 1 // QT_CONFIG(mimetype_database)
    enum : bool { InternalDatabaseAvailable = true };
#else
    enum : bool { InternalDatabaseAvailable = false };
#endif
    MimeXMLProvider(MimeDatabasePrivate *db, InternalDatabaseEnum);
    MimeXMLProvider(MimeDatabasePrivate *db, const QString &directory);
    // added for Qt Creator
    MimeXMLProvider(MimeDatabasePrivate *db, const QString &directory, const QByteArray &data);
    ~MimeXMLProvider() override;

    auto isValid() -> bool override;
    [[nodiscard]] auto isInternalDatabase() const -> bool override;
    auto mimeTypeForName(const QString &name) -> MimeType override;
    void addFileNameMatches(const QString &fileName, MimeGlobMatchResult &result) override;
    void addParents(const QString &mime, QStringList &result) override;
    auto resolveAlias(const QString &name) -> QString override;
    void addAliases(const QString &name, QStringList &result) override;
    void findByMagic(const QByteArray &data, int *accuracyPtr, MimeType &candidate) override;
    void addAllMimeTypes(QList<MimeType> &result) override;
    void ensureLoaded() override;

    auto load(const QString &fileName, QString *errorMessage) -> bool;

    // Called by the mimetype xml parser
    void addMimeType(const MimeType &mt);
    void addGlobPattern(const MimeGlobPattern &glob);
    void addParent(const QString &child, const QString &parent);
    void addAlias(const QString &alias, const QString &name);
    void addMagicMatcher(const MimeMagicRuleMatcher &matcher);

    // added for Qt Creator
    auto hasMimeTypeForName(const QString &name) -> bool override;
    auto allMimeTypeNames() -> QStringList override;
    [[nodiscard]] auto magicRulesForMimeType(const MimeType &mimeType) const -> QMap<int, QList<MimeMagicRule>> override;
    void setMagicRulesForMimeType(const MimeType &mimeType,
                                  const QMap<int, QList<MimeMagicRule>> &rules) override;
    void setGlobPatternsForMimeType(const MimeType &mimeType, const QStringList &patterns) override;

private:
    void load(const QString &fileName);
    void load(const char *data, qsizetype len);

    typedef QHash<QString, MimeType> NameMimeTypeMap;
    NameMimeTypeMap m_nameMimeTypeMap;

    typedef QHash<QString, QString> AliasHash;
    AliasHash m_aliases;

    typedef QHash<QString, QStringList> ParentsHash;
    ParentsHash m_parents;
    MimeAllGlobPatterns m_mimeTypeGlobs;

    QList<MimeMagicRuleMatcher> m_magicMatchers;
    QStringList m_allFiles;
};

} // namespace Utils
