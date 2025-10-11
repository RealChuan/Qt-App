// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#pragma once

#include "utils_global.h"

#include "hostosinfo.h"

#include <QList>
#include <QPointer>

#include <functional>

namespace Utils {

namespace Internal {
class MacroExpanderPrivate;
}

class FilePath;
class MacroExpander;

class UTILS_EXPORT MacroExpanderProvider
{
public:
    MacroExpanderProvider(QObject *guard, const std::function<MacroExpander *()> &creator);
    MacroExpanderProvider(QObject *guard, MacroExpander *expander);
    explicit MacroExpanderProvider(MacroExpander *expander); // Guarded by qApp.

    MacroExpander *operator()() const;

    bool operator!() const { return !m_creator; }
    explicit operator bool() const { return !!m_creator; }

private:
    QPointer<QObject> m_guard;
    std::function<MacroExpander *()> m_creator;
};

using MacroExpanderProviders = QList<MacroExpanderProvider>;

class UTILS_EXPORT MacroExpander
{
    Q_DISABLE_COPY(MacroExpander)

public:
    MacroExpander();
    ~MacroExpander();

    bool resolveMacro(const QString &name, QString *ret) const;

    QString value(const QByteArray &variable, bool *found = nullptr) const;

    QString expand(const QString &stringWithVariables) const;
    FilePath expand(const FilePath &fileNameWithVariables) const;
    QByteArray expand(const QByteArray &stringWithVariables) const;
    QVariant expandVariant(const QVariant &v) const;

    Result<QString> expandProcessArgs(const QString &argsWithVariables,
                                      Utils::OsType osType = Utils::HostOsInfo::hostOs()) const;

    using PrefixFunction = std::function<QString(QString)>;
    using ResolverFunction = std::function<bool(QString, QString *)>;
    using StringFunction = std::function<QString()>;
    using FileFunction = std::function<FilePath()>;
    using IntFunction = std::function<int()>;

    void registerPrefix(const QByteArray &prefix,
                        const QByteArray &examplePostfix,
                        const QString &description,
                        const PrefixFunction &value,
                        bool visible = true,
                        bool availableForExpansion = true);

    void registerVariable(const QByteArray &variable,
                          const QString &description,
                          const StringFunction &value,
                          bool visibleInChooser = true,
                          bool availableForExpansion = true);

    void registerIntVariable(const QByteArray &variable,
                             const QString &description,
                             const IntFunction &value);

    void registerFileVariables(const QByteArray &prefix,
                               const QString &heading,
                               const FileFunction &value,
                               bool visibleInChooser = true,
                               bool availableForExpansion = true);

    void registerExtraResolver(const ResolverFunction &value);

    QList<QByteArray> visibleVariables() const;
    QString variableDescription(const QByteArray &variable) const;
    QByteArray variableExampleUsage(const QByteArray &variable) const;
    bool isPrefixVariable(const QByteArray &variable) const;

    MacroExpanderProviders subProviders() const;

    QString displayName() const;
    void setDisplayName(const QString &displayName);

    void registerSubProvider(const MacroExpanderProvider &provider);
    void clearSubProviders();

    bool isAccumulating() const;
    void setAccumulating(bool on);

private:
    friend class Internal::MacroExpanderPrivate;
    Internal::MacroExpanderPrivate *d;
};

UTILS_EXPORT MacroExpander *globalMacroExpander();

} // namespace Utils
