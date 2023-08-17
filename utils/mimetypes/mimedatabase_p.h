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

#include "mimetype.h"

#include "mimeglobpattern_p.h"
#include "mimemagicrule_p.h"
#include "mimetype_p.h"

#include <QtCore/qelapsedtimer.h>
#include <QtCore/qlist.h>
#include <QtCore/qmutex.h>

#include <vector>
#include <memory>

QT_BEGIN_NAMESPACE
class QIODevice;
QT_END_NAMESPACE

namespace Utils {

class MimeDatabase;
class MimeProviderBase;

class MimeDatabasePrivate
{
public:
    Q_DISABLE_COPY_MOVE(MimeDatabasePrivate)

    MimeDatabasePrivate();
    ~MimeDatabasePrivate();

    static auto instance() -> MimeDatabasePrivate *;

    [[nodiscard]] inline auto defaultMimeType() const -> QString { return m_defaultMimeType; }

    auto inherits(const QString &mime, const QString &parent) -> bool;

    auto allMimeTypes() -> QList<MimeType>;

    auto resolveAlias(const QString &nameOrAlias) -> QString;
    auto parents(const QString &mimeName) -> QStringList;
    auto mimeTypeForName(const QString &nameOrAlias) -> MimeType;
    auto mimeTypeForFileNameAndData(const QString &fileName, QIODevice *device, int *priorityPtr) -> MimeType;
    auto findByData(const QByteArray &data, int *priorityPtr) -> MimeType;
    auto mimeTypeForFileName(const QString &fileName) -> QStringList;
    auto findByFileName(const QString &fileName) -> MimeGlobMatchResult;

    // API for MimeType. Takes care of locking the mutex.
    void loadMimeTypePrivate(MimeTypePrivate &mimePrivate);
    void loadGenericIcon(MimeTypePrivate &mimePrivate);
    void loadIcon(MimeTypePrivate &mimePrivate);
    auto mimeParents(const QString &mimeName) -> QStringList;
    auto listAliases(const QString &mimeName) -> QStringList;
    auto mimeInherits(const QString &mime, const QString &parent) -> bool;

    // added for Qt Creator
    void addMimeData(const QString &id, const QByteArray &data);
    auto magicRulesForMimeType(const MimeType &mimeType) -> QMap<int, QList<MimeMagicRule>>;
    void setMagicRulesForMimeType(const MimeType &mimeType,
                                  const QMap<int, QList<MimeMagicRule>> &rules);
    void setGlobPatternsForMimeType(const MimeType &mimeType, const QStringList &patterns);

private:
    using Providers = std::vector<std::unique_ptr<MimeProviderBase>>;
    const Providers &providers();
    auto shouldCheck() -> bool;
    void loadProviders();

    mutable Providers m_providers;
    QElapsedTimer m_lastCheck;

    // added for Qt Creator
    QHash<QString, QByteArray> m_additionalData; // id -> data
    bool m_forceLoad = true;

public:
    const QString m_defaultMimeType;
    QMutex mutex;

    // added for Qt Creator
    int m_startupPhase = 0;
};

} // namespace Utils
