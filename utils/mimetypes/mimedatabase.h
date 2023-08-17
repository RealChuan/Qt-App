// Copyright (C) 2016 The Qt Company Ltd.
// Copyright (C) 2015 Klaralvdalens Datakonsult AB, a KDAB Group company, info@kdab.com, author David Faure <david.faure@kdab.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#pragma once

#include "mimetype.h"

#include "mimemagicrule_p.h"

#include <utils/utils_global.h>

#include <QtCore/qstringlist.h>

QT_BEGIN_NAMESPACE
class QByteArray;
class QFileInfo;
class QIODevice;
class QUrl;
QT_END_NAMESPACE

namespace Utils {

class MimeDatabasePrivate;

class MimeDatabase
{
    Q_DISABLE_COPY(MimeDatabase)

public:
    MimeDatabase();
    ~MimeDatabase();

    [[nodiscard]] auto mimeTypeForName(const QString &nameOrAlias) const -> MimeType;

    enum MatchMode {
        MatchDefault = 0x0,
        MatchExtension = 0x1,
        MatchContent = 0x2
    };

    [[nodiscard]] auto mimeTypeForFile(const QString &fileName, MatchMode mode = MatchDefault) const -> MimeType;
    [[nodiscard]] auto mimeTypeForFile(const QFileInfo &fileInfo, MatchMode mode = MatchDefault) const -> MimeType;
    [[nodiscard]] auto mimeTypesForFileName(const QString &fileName) const -> QList<MimeType>;

    [[nodiscard]] auto mimeTypeForData(const QByteArray &data) const -> MimeType;
    auto mimeTypeForData(QIODevice *device) const -> MimeType;

    [[nodiscard]] auto mimeTypeForUrl(const QUrl &url) const -> MimeType;
    auto mimeTypeForFileNameAndData(const QString &fileName, QIODevice *device) const -> MimeType;
    [[nodiscard]] auto mimeTypeForFileNameAndData(const QString &fileName, const QByteArray &data) const -> MimeType;

    [[nodiscard]] auto suffixForFileName(const QString &fileName) const -> QString;

    [[nodiscard]] auto allMimeTypes() const -> QList<MimeType>;

private:
    MimeDatabasePrivate *d;
};

} // namespace Utils
