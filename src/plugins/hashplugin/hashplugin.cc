#include "hashplugin.hpp"
#include "hashwidget.hpp"

#include <QPushButton>

namespace Plugin {

HashPluginWidget::HashPluginWidget(QObject *parent)
{
    setWidget(new HashWidget);
    setButton(new QPushButton(tr("Hash")), Core::CoreWidget::Main);
}

auto HashPlugin::initialize(const QStringList &arguments, QString *errorString) -> bool
{
    addObject(new HashPluginWidget(this));
    return true;
}

} // namespace Plugin
