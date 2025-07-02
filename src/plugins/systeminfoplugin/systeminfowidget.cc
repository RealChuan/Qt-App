#include "systeminfowidget.hpp"

#include <extensionsystem/pluginmanager.h>

#include <QtWidgets>

namespace Plugin {

SystemInfoWidget::SystemInfoWidget(QWidget *parent)
    : QWidget{parent}
{
    setupUI();
}

void SystemInfoWidget::setupUI()
{
    auto *textBrowser = new QTextBrowser(this);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins({});
    layout->addWidget(textBrowser);

    // auto systemEnviroment = QProcess::systemEnvironment();
    // for (const auto &info : std::as_const(systemEnviroment)) {
    //     textBrowser->append(info);
    //     textBrowser->append("\n");
    // }

    textBrowser->setText(ExtensionSystem::PluginManager::systemInformation());

    auto cursor = textBrowser->textCursor();
    cursor.setPosition(0);
    textBrowser->setTextCursor(cursor);
}

} // namespace Plugin
