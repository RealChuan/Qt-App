#include "host.hpp"

#include <extensionsystem/pluginmanager.hpp>

namespace Shell {

namespace {
Host *s_host = nullptr;
}

void setHost(Host *host)
{
    s_host = host;
}

auto host() -> Host *
{
    return s_host;
}

auto settings() -> QSettings *
{
    return ExtensionSystem::PluginManager::settings();
}

} // namespace Shell
