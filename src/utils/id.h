// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#pragma once

#include "utils_global.h"

#include "storekey.h"

#include <QList>
#include <QMetaType>
#include <QString>

QT_BEGIN_NAMESPACE
class QDataStream;
class QVariant;
QT_END_NAMESPACE

namespace Utils {

class UTILS_EXPORT Id
{
public:
    Id() = default;

    template<int N>
    Id(const char (&s)[N])
        : Id(s, N - 1)
    {}

    Id(const QLatin1String &) = delete;

    static Id generate();

    Id withSuffix(qsizetype suffix) const;
#if QT_POINTER_SIZE != 4
    Id withSuffix(int suffix) const;
#endif
    Id withSuffix(char suffix) const;
    Id withSuffix(const char *suffix) const;
    Id withSuffix(QStringView suffix) const;
    Id withPrefix(const char *prefix) const;

    QByteArrayView name() const;
    QByteArray toByteArray() const; // Avoid
    QString toString() const;       // Avoid.
    Key toKey() const;              // FIXME: Replace uses with .name() after Store/key transition.
    QVariant toSetting() const;     // Good to use.
    QString suffixAfter(Id baseId) const;
    bool isValid() const { return m_id; }
    bool operator==(Id id) const { return m_id == id.m_id; }
    bool operator==(const char *name) const;
    bool operator!=(Id id) const { return m_id != id.m_id; }
    bool operator!=(const char *name) const { return !operator==(name); }
    bool operator<(Id id) const { return m_id < id.m_id; }
    bool operator>(Id id) const { return m_id > id.m_id; }
    bool alphabeticallyBefore(Id other) const;

    static Id fromString(QStringView str);          // FIXME: avoid.
    static Id fromName(QByteArrayView ba);          // FIXME: avoid.
    static Id fromSetting(const QVariant &variant); // Good to use.

    static QSet<Id> fromStringList(const QStringList &list);
    static QStringList toStringList(const QSet<Id> &ids);

    friend size_t qHash(Id id) { return static_cast<size_t>(id.m_id); }
    friend UTILS_EXPORT QDataStream &operator<<(QDataStream &ds, Id id);
    friend UTILS_EXPORT QDataStream &operator>>(QDataStream &ds, Id &id);
    friend UTILS_EXPORT QDebug operator<<(QDebug dbg, const Id &id);

private:
    Id(const char *s, size_t len);
    explicit Id(quintptr uid)
        : m_id(uid)
    {}

    quintptr m_id = 0;
};

} // namespace Utils

Q_DECLARE_METATYPE(Utils::Id)
Q_DECLARE_METATYPE(QList<Utils::Id>)
