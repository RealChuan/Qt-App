#include "guiplugin.hpp"
#include "guiwidget.hpp"

#include <QPushButton>

namespace Plugin {

GuiPluginWidget::GuiPluginWidget(QObject *parent)
{
    setWidget(new GuiWidget);
    setButton(new QPushButton(tr("Gui")), Core::CoreWidget::Main);
}

bool GuiPlugin::initialize(const QStringList &arguments, QString *errorString)
{
    addObject(new GuiPluginWidget(this));
    return true;
}

} // namespace Plugin
