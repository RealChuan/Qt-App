// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#pragma once

#include <utils/filepath.h>
#include <utils/qtcsettings.h>

namespace Plugin {

class ICore : public QObject
{
    Q_OBJECT
public:
    ICore();
    ~ICore() override;

    static ICore *instance();

    static Utils::QtcSettings *settings(QSettings::Scope scope = QSettings::UserScope);

    static bool askForRestart(const QString &text, const QString &altButtonText = {});

    static bool isQtDesignStudio();

    static Utils::FilePath resourcePath(const QString &rel);

public slots:
    void restart();
};

} // namespace Plugin
