#include "systeminfoplugin.hpp"
#include "systeminfowidget.hpp"

#include <QPushButton>

namespace Plugin {

SystemInfoPluginWidget::SystemInfoPluginWidget(QObject *parent)
{
    setWidget(new SystemInfoWidget);
    setButton(new QPushButton(tr("System Info")), Core::CoreWidget::Help);
}

bool SystemInfoPlugin::initialize(const QStringList &arguments, QString *errorString)
{
    addObject(new SystemInfoPluginWidget(this));
    return true;
}

} // namespace Plugin
