#include "aboutplugin.hpp"
#include "aboutwidget.hpp"

#include <QPushButton>

namespace Plugin {

AboutPluginWidget::AboutPluginWidget(QObject *parent)
{
    auto *aboutWidget = new AboutWidget;
    aboutWidget->setObjectName("AboutWidget");
    setWidget(aboutWidget);
    setButton(new QPushButton(tr("About")), Core::CoreWidget::Help);
}

bool AboutPlugin::initialize(const QStringList &arguments, QString *errorString)
{
    addObject(new AboutPluginWidget(this));
    return true;
}

} // namespace Plugin
