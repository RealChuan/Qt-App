#include "serialplugin.hpp"
#include "serialwidget.h"

#include <QPushButton>

namespace Plugin {

SerialPluginWidget::SerialPluginWidget(QObject *parent)
{
    setWidget(new SerialWidget);
    setButton(new QPushButton(tr("Serial Tool")), Core::CoreWidget::Tool);
}

auto SerialPlugin::initialize(const QStringList &arguments, QString *errorString) -> bool
{
    addObject(new SerialPluginWidget(this));
    return true;
}

} // namespace Plugin
