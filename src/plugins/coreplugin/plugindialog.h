// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#pragma once

#include <gui/dialog.hpp>

QT_BEGIN_NAMESPACE
class QPushButton;
QT_END_NAMESPACE

namespace ExtensionSystem {
class PluginSpec;
class PluginView;
} // namespace ExtensionSystem

namespace Plugin {

class PluginDialog : public GUI::Dialog
{
    Q_OBJECT

public:
    explicit PluginDialog(QWidget *parent);

private:
    void updateButtons();
    void openDetails(ExtensionSystem::PluginSpec *spec);
    void openErrorDetails();
    void closeDialog();
    void showInstallWizard();

    ExtensionSystem::PluginView *m_view;

    QPushButton *m_detailsButton;
    QPushButton *m_errorDetailsButton;
    QPushButton *m_installButton;
    bool m_isRestartRequired = false;
};

} // namespace Plugin
