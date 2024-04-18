#include "helloplugin.hpp"
#include "hellowidget.hpp"

#include <QPushButton>

namespace Plugin {

HelloPluginWidget::HelloPluginWidget(QObject *parent)
{
    setWidget(new HelloWidget);
    setButton(new QPushButton(tr("Hello")), Core::CoreWidget::Main);
}

bool HelloPlugin::initialize(const QStringList &arguments, QString *errorString)
{
    addObject(new HelloPluginWidget(this));
    return true;
}

} // namespace Plugin
