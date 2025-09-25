// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "icore.h"
#include "coreplugin.hpp"
#include "coreplugintr.h"

#include <extensionsystem/pluginmanager.h>
#include <utils/appinfo.h>
#include <utils/utils.hpp>

#include <QtWidgets>

namespace Plugin {

static void setRestart(bool restart)
{
    qApp->setProperty("restart", restart);
}

static bool isRestartRequested()
{
    return qApp->property("restart").toBool();
}

static ICore *m_core = nullptr;

ICore::ICore()
{
    m_core = this;
}

ICore::~ICore() {}

ICore *ICore::instance()
{
    return m_core;
}

Utils::QtcSettings *ICore::settings(QSettings::Scope scope)
{
    if (scope == QSettings::UserScope) {
        return ExtensionSystem::PluginManager::settings();
    } else {
        return ExtensionSystem::PluginManager::globalSettings();
    }
}

bool ICore::askForRestart(const QString &text, const QString &altButtonText)
{
    QMessageBox mb(dialogParent());
    mb.setWindowTitle(Tr::tr("Restart Required"));
    mb.setText(text);
    mb.setIcon(QMessageBox::Information);

    QString translatedAltButtonText = altButtonText.isEmpty() ? Tr::tr("Later") : altButtonText;

    mb.addButton(translatedAltButtonText, QMessageBox::NoRole);
    mb.addButton(Tr::tr("Restart Now"), QMessageBox::YesRole);

    mb.connect(&mb, &QDialog::accepted, ICore::instance(), &ICore::restart, Qt::QueuedConnection);
    mb.exec();

    return mb.buttonRole(mb.clickedButton()) == QMessageBox::YesRole;
}

bool ICore::isQtDesignStudio()
{
    auto *settings = ICore::settings();
    return settings->value("QML/Designer/StandAloneMode", false).toBool();
}

Utils::FilePath ICore::resourcePath(const QString &rel)
{
    return Utils::appInfo().resources / rel;
}

void ICore::restart()
{
    setRestart(true);
    Utils::quitApplication();
}

} // namespace Plugin
