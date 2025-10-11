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

    ~SystemInfoPlugin() override = default;

    Utils::Result<> initialize(const QStringList &arguments) override
    {
        m_systemInfoPluginWidgetPtr.reset(new SystemInfoPluginWidget(this));
        Core::addCoreWidget(m_systemInfoPluginWidgetPtr.data());
        return Utils::ResultOk;
    }

    QScopedPointer<SystemInfoPluginWidget> m_systemInfoPluginWidgetPtr;
};

} // namespace Plugin

#include "systeminfoplugin.moc"