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

    ~GuiPlugin() override = default;

    Utils::Result<> initialize(const QStringList &argument) override
    {
        m_guiPluginWidgetPtr.reset(new GuiPluginWidget(this));
        Core::addCoreWidget(m_guiPluginWidgetPtr.data());
        return Utils::ResultOk;
    }

    QScopedPointer<GuiPluginWidget> m_guiPluginWidgetPtr;
};

} // namespace Plugin

#include "guiplugin.moc"
