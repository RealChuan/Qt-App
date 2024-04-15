// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#pragma once

#include "utils_global.h"

#include <QList>
#include <QString>

#include <functional>

QT_BEGIN_NAMESPACE
class QJsonValue;
QT_END_NAMESPACE

namespace Utils {

// Create a usable settings key from a category,
// for example Editor|C++ -> Editor_C__
UTILS_EXPORT auto settingsKey(const QString &category) -> QString;

// Return the common prefix part of a string list:
// "C:\foo\bar1" "C:\foo\bar2"  -> "C:\foo\bar"
UTILS_EXPORT auto commonPrefix(const QStringList &strings) -> QString;

// Removes first unescaped ampersand in text
UTILS_EXPORT auto stripAccelerator(const QString &text) -> QString;
// Quotes all ampersands
UTILS_EXPORT auto quoteAmpersands(const QString &text) -> QString;

UTILS_EXPORT auto readMultiLineString(const QJsonValue &value, QString *out) -> bool;

// Compare case insensitive and use case sensitive comparison in case of that being equal.
UTILS_EXPORT auto caseFriendlyCompare(const QString &a, const QString &b) -> int;

class UTILS_EXPORT AbstractMacroExpander
{
public:
    virtual ~AbstractMacroExpander() {}
    // Not const, as it may change the state of the expander.
    //! Find an expando to replace and provide a replacement string.
    //! \param str The string to scan
    //! \param pos Position to start scan on input, found position on output
    //! \param ret Replacement string on output
    //! \return Length of string part to replace, zero if no (further) matches found
    virtual auto findMacro(const QString &str, int *pos, QString *ret) -> int;
    //! Provide a replacement string for an expando
    //! \param name The name of the expando
    //! \param ret Replacement string on output
    //! \return True if the expando was found
    virtual auto resolveMacro(const QString &name, QString *ret, QSet<AbstractMacroExpander *> &seen) -> bool
        = 0;

private:
    auto expandNestedMacros(const QString &str, int *pos, QString *ret) -> bool;
};

UTILS_EXPORT void expandMacros(QString *str, AbstractMacroExpander *mx);
UTILS_EXPORT auto expandMacros(const QString &str, AbstractMacroExpander *mx) -> QString;

UTILS_EXPORT auto parseUsedPortFromNetstatOutput(const QByteArray &line) -> int;

UTILS_EXPORT auto appendHelper(const QString &base, int n) -> QString;

template<typename T>
auto makeUniquelyNumbered(const T &preferred, const std::function<bool(const T &)> &isOk) -> T
{
    if (isOk(preferred))
        return preferred;
    int i = 2;
    T tryName = appendHelper(preferred, i);
    while (!isOk(tryName))
        tryName = appendHelper(preferred, ++i);
    return tryName;
}

template<typename T, typename Container>
auto makeUniquelyNumbered(const T &preferred, const Container &reserved) -> T
{
    const std::function<bool(const T &)> isOk = [&reserved](const T &v) {
        return !reserved.contains(v);
    };
    return makeUniquelyNumbered(preferred, isOk);
}

UTILS_EXPORT auto formatElapsedTime(qint64 elapsed) -> QString;

/* This function is only necessary if you need to match the wildcard expression against a
 * string that might contain path separators - otherwise
 * QRegularExpression::wildcardToRegularExpression() can be used.
 * Working around QRegularExpression::wildcardToRegularExpression() taking native separators
 * into account and handling them to disallow matching a wildcard characters.
 */
UTILS_EXPORT auto wildcardToRegularExpression(const QString &original) -> QString;

UTILS_EXPORT auto languageNameFromLanguageCode(const QString &languageCode) -> QString;

#ifdef QT_WIDGETS_LIB

// Feeds the global clipboard and, when present, the primary selection
UTILS_EXPORT void setClipboardAndSelection(const QString &text);

#endif

UTILS_EXPORT auto chopIfEndsWith(QString str, QChar c) -> QString;
UTILS_EXPORT QStringView chopIfEndsWith(QStringView str, QChar c);

UTILS_EXPORT auto normalizeNewlines(const QString &text) -> QString;

// Skips empty parts - see QTBUG-110900
UTILS_EXPORT auto joinStrings(const QStringList &strings, QChar separator) -> QString;
UTILS_EXPORT auto trimFront(const QString &string, QChar ch) -> QString;
UTILS_EXPORT auto trimBack(const QString &string, QChar ch) -> QString;
UTILS_EXPORT auto trim(const QString &string, QChar ch) -> QString;

} // namespace Utils
