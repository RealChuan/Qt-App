#include "systeminfowidget.hpp"

#include <core/corewidget.hpp>
#include <extensionsystem/iplugin.h>

#include <QPushButton>

namespace Plugin {

class SystemInfoPluginWidget : public Core::CoreWidget
{
    Q_OBJECT
public:
    explicit SystemInfoPluginWidget(QObject *parent = nullptr)
    {
        setWidget(new SystemInfoWidget);
        setButton(new QPushButton(tr("System Info")), Core::CoreWidget::Help);
    }
};

class SystemInfoPlugin : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "Youth.Qt.plugin" FILE "systeminfoplugin.json")
public:
    Utils::Result<> initialize(const QStringList &arguments) override
    {
        addObject(new SystemInfoPluginWidget(this));
        return Utils::ResultOk;
    }
};

} // namespace Plugin

#include "systeminfoplugin.moc"