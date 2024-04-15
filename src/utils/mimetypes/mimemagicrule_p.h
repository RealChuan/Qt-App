// Copyright (C) 2016 The Qt Company Ltd.
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

#include <utils/utils_global.h>

#include <QtCore/qbytearray.h>
#include <QtCore/qlist.h>
#include <QtCore/qregularexpression.h>
#include <QtCore/qscopedpointer.h>

namespace Utils {

class UTILS_EXPORT MimeMagicRule
{
public:
    enum Type {
        Invalid = 0,
        String,
        RegExp,
        Host16,
        Host32,
        Big16,
        Big32,
        Little16,
        Little32,
        Byte
    };

    MimeMagicRule(const QString &typeStr, const QByteArray &value, const QString &offsets,
                   const QByteArray &mask, QString *errorString);
    // added for Qt Creator
    MimeMagicRule(const Type &type, const QByteArray &value, int startPos, int endPos,
                  const QByteArray &mask = {}, QString *errorString = nullptr);

    void swap(MimeMagicRule &other) noexcept
    {
        qSwap(m_type,          other.m_type);
        qSwap(m_value,         other.m_value);
        qSwap(m_startPos,      other.m_startPos);
        qSwap(m_endPos,        other.m_endPos);
        qSwap(m_mask,          other.m_mask);
        qSwap(m_pattern,       other.m_pattern);
        qSwap(m_number,        other.m_number);
        qSwap(m_numberMask,    other.m_numberMask);
        qSwap(m_matchFunction, other.m_matchFunction);
    }

    auto operator==(const MimeMagicRule &other) const -> bool;

    [[nodiscard]] auto type() const -> Type { return m_type; }
    [[nodiscard]] auto value() const -> QByteArray { return m_value; }
    [[nodiscard]] auto startPos() const -> int { return m_startPos; }
    [[nodiscard]] auto endPos() const -> int { return m_endPos; }
    [[nodiscard]] auto mask() const -> QByteArray;

    [[nodiscard]] auto isValid() const -> bool { return m_matchFunction != nullptr; }

    [[nodiscard]] auto matches(const QByteArray &data) const -> bool;

    QList<MimeMagicRule> m_subMatches;

    static auto type(const QByteArray &type) -> Type;
    static auto typeName(Type type) -> QByteArray;

    static auto matchSubstring(const char *dataPtr, int dataSize, int rangeStart, int rangeLength, int valueLength, const char *valueData, const char *mask) -> bool;

private:
    // added for Qt Creator
    void init(QString *errorString);

    Type m_type;
    QByteArray m_value;
    int m_startPos;
    int m_endPos;
    QByteArray m_mask;

    QRegularExpression m_regexp;
    QByteArray m_pattern;
    quint32 m_number;
    quint32 m_numberMask;

    typedef bool (MimeMagicRule::*MatchFunction)(const QByteArray &data) const;
    MatchFunction m_matchFunction;

private:
    // match functions
    [[nodiscard]] auto matchString(const QByteArray &data) const -> bool;
    template <typename T>
    [[nodiscard]] auto matchNumber(const QByteArray &data) const -> bool;
    [[nodiscard]] auto matchRegExp(const QByteArray &data) const -> bool;
};

} // namespace Utils

QT_BEGIN_NAMESPACE
Q_DECLARE_SHARED(Utils::MimeMagicRule)
QT_END_NAMESPACE
