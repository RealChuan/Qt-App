#include "tcpplugin.hpp"
#include "tcpwidget.h"

#include <QPushButton>

namespace Plugin {

auto TcpPlugin::initialize(const QStringList &arguments, QString *errorString) -> bool
{
    addObject(new TcpPluginWidget(this));
    return true;
}

TcpPluginWidget::TcpPluginWidget(QObject *parent)
{
    setWidget(new TcpWidget);
    setButton(new QPushButton(tr("Tcp Tool")), Core::CoreWidget::Tool);
}

} // namespace Plugin
