#include "plugincontext.hpp"

#include "pluginspec.hpp"

#include <QSettings>
#include <QStringView>

#include <utility>

namespace ExtensionSystem {

PluginContext::PluginContext(const PluginSpec *spec, QSettings *settings, Utils::FilePath resourceRoot)
    : m_spec(spec)
    , m_settings(settings)
    , m_resourceRoot(std::move(resourceRoot))
{}

auto PluginContext::spec() const -> const PluginSpec &
{
    Q_ASSERT(m_spec);
    return *m_spec;
}

auto PluginContext::id() const -> QString
{
    return m_spec ? m_spec->id() : QString();
}

auto PluginContext::settings() const -> QSettings *
{
    return m_settings;
}

auto PluginContext::resourcePath(QAnyStringView relative) const -> Utils::FilePath
{
    return m_resourceRoot / relative.toString();
}

} // namespace ExtensionSystem
