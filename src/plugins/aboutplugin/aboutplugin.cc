#include "aboutwidget.hpp"

#include <core/corewidget.hpp>
#include <extensionsystem/iplugin.h>

#include <QPushButton>

namespace Plugin {

class AboutPluginWidget : public Core::CoreWidget
{
    Q_OBJECT
public:
    explicit AboutPluginWidget(QObject *parent = nullptr)
    {
        auto *aboutWidget = new AboutWidget;
        aboutWidget->setObjectName("AboutWidget");
        setWidget(aboutWidget);
        setButton(new QPushButton(tr("About")), Core::CoreWidget::Help);
    }
};

class AboutPlugin : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "Youth.Qt.plugin" FILE "aboutplugin.json")
public:
    Utils::Result<> initialize(const QStringList &arguments) override
    {
        addObject(new AboutPluginWidget(this));
        return Utils::ResultOk;
    }
};

} // namespace Plugin

#include "aboutplugin.moc"
