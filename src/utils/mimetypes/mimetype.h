// Copyright (C) 2016 The Qt Company Ltd.
// Copyright (C) 2015 Klaralvdalens Datakonsult AB, a KDAB Group company, info@kdab.com, author David Faure <david.faure@kdab.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#pragma once

#include <utils/utils_global.h>

#include <QtCore/qobjectdefs.h>
#include <QtCore/qshareddata.h>
#include <QtCore/qstring.h>
#include <QtCore/qstringlist.h>

namespace Utils {

class MimeTypePrivate;
class MimeType;

UTILS_EXPORT size_t qHash(const MimeType &key, size_t seed = 0) noexcept;

class UTILS_EXPORT MimeType
{
    Q_GADGET
    Q_PROPERTY(bool valid READ isValid CONSTANT)
    Q_PROPERTY(bool isDefault READ isDefault CONSTANT)
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(QString comment READ comment CONSTANT)
    Q_PROPERTY(QString genericIconName READ genericIconName CONSTANT)
    Q_PROPERTY(QString iconName READ iconName CONSTANT)
    Q_PROPERTY(QStringList globPatterns READ globPatterns CONSTANT)
    Q_PROPERTY(QStringList parentMimeTypes READ parentMimeTypes CONSTANT)
    Q_PROPERTY(QStringList allAncestors READ allAncestors CONSTANT)
    Q_PROPERTY(QStringList aliases READ aliases CONSTANT)
    Q_PROPERTY(QStringList suffixes READ suffixes CONSTANT)
    Q_PROPERTY(QString preferredSuffix READ preferredSuffix CONSTANT)
    Q_PROPERTY(QString filterString READ filterString CONSTANT)

public:
    MimeType();
    MimeType(const MimeType &other);
    auto operator=(const MimeType &other) -> MimeType &;
    auto operator=(MimeType &&other) noexcept -> MimeType &
    {
        swap(other);
        return *this;
    }
    void swap(MimeType &other) noexcept { d.swap(other.d); }
    explicit MimeType(const MimeTypePrivate &dd);
    ~MimeType();

    auto operator==(const MimeType &other) const -> bool;

    inline auto operator!=(const MimeType &other) const -> bool { return !operator==(other); }

    [[nodiscard]] auto isValid() const -> bool;

    [[nodiscard]] auto isDefault() const -> bool;

    [[nodiscard]] auto name() const -> QString;
    [[nodiscard]] auto comment() const -> QString;
    [[nodiscard]] auto genericIconName() const -> QString;
    [[nodiscard]] auto iconName() const -> QString;
    [[nodiscard]] auto globPatterns() const -> QStringList;
    [[nodiscard]] auto parentMimeTypes() const -> QStringList;
    [[nodiscard]] auto allAncestors() const -> QStringList;
    [[nodiscard]] auto aliases() const -> QStringList;
    [[nodiscard]] auto suffixes() const -> QStringList;
    [[nodiscard]] auto preferredSuffix() const -> QString;

    Q_INVOKABLE [[nodiscard]] bool inherits(const QString &mimeTypeName) const;

    [[nodiscard]] auto filterString() const -> QString;

    // Qt Creator additions
    [[nodiscard]] auto matchesName(const QString &nameOrAlias) const -> bool;
    void setPreferredSuffix(const QString &suffix);

protected:
    friend class MimeTypeParserBase;
    friend class MimeTypeMapEntry;
    friend class MimeDatabasePrivate;
    friend class MimeXMLProvider;
    friend class MimeBinaryProvider;
    friend class MimeTypePrivate;
    friend UTILS_EXPORT size_t qHash(const MimeType &key, size_t seed) noexcept;

    QExplicitlySharedDataPointer<MimeTypePrivate> d;
};

} // namespace Utils

QT_BEGIN_NAMESPACE
#ifndef QT_NO_DEBUG_STREAM
class QDebug;
UTILS_EXPORT auto operator<<(QDebug debug, const Utils::MimeType &mime) -> QDebug;
#endif

Q_DECLARE_SHARED(Utils::MimeType)
QT_END_NAMESPACE
