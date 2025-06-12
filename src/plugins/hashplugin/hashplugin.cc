#include "hashwidget.hpp"

#include <core/corewidget.hpp>
#include <extensionsystem/iplugin.h>

#include <QPushButton>

namespace Plugin {

class HashPluginWidget : public Core::CoreWidget
{
    Q_OBJECT
public:
    explicit HashPluginWidget(QObject *parent = nullptr)
    {
        setWidget(new HashWidget);
        setButton(new QPushButton(tr("Hash")), Core::CoreWidget::Main);
    }
};

class HashPlugin : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "Youth.Qt.plugin" FILE "hashplugin.json")
public:
    Utils::Result<> initialize(const QStringList &arguments) override
    {
        addObject(new HashPluginWidget(this));
        return Utils::ResultOk;
    }
};

} // namespace Plugin

#include "hashplugin.moc"
