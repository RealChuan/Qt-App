#include "hellowidget.hpp"

#include <core/corewidget.hpp>
#include <extensionsystem/iplugin.h>

#include <QPushButton>

namespace Plugin {

class HelloPluginWidget : public Core::CoreWidget
{
    Q_OBJECT
public:
    explicit HelloPluginWidget(QObject *parent = nullptr)
    {
        setWidget(new HelloWidget);
        setButton(new QPushButton(tr("Hello")), Core::CoreWidget::Main);
    }
};

class HelloPlugin : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "Youth.Qt.plugin" FILE "helloplugin.json")
public:
    Utils::Result<> initialize(const QStringList &arguments) override
    {
        addObject(new HelloPluginWidget(this));
        return Utils::ResultOk;
    }
};

} // namespace Plugin

#include "helloplugin.moc"
