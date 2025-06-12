#include "guiwidget.hpp"

#include <core/corewidget.hpp>
#include <extensionsystem/iplugin.h>

#include <QPushButton>

namespace Plugin {

class GuiPluginWidget : public Core::CoreWidget
{
    Q_OBJECT
public:
    explicit GuiPluginWidget(QObject *parent = nullptr)
    {
        setWidget(new GuiWidget);
        setButton(new QPushButton(tr("Gui")), Core::CoreWidget::Main);
    }
};

class GuiPlugin : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "Youth.Qt.plugin" FILE "guiplugin.json")
public:
    Utils::Result<> initialize(const QStringList &argument) override
    {
        addObject(new GuiPluginWidget(this));
        return Utils::ResultOk;
    }
};

} // namespace Plugin

#include "guiplugin.moc"
