#include "plugincontext.hpp"

#include "pluginspec.hpp"

#include <QSettings>
#include <QStringView>

#include <utility>

namespace ExtensionSystem {

class PluginContext::PluginContextPrivate
{
public:
    PluginContextPrivate() = default;
    PluginContextPrivate(const PluginSpec *spec, QSettings *settings, Utils::FilePath resourceRoot)
        : spec(spec)
        , settings(settings)
        , resourceRoot(std::move(resourceRoot))
    {}

    const PluginSpec *spec = nullptr;
    QSettings *settings = nullptr;
    Utils::FilePath resourceRoot;
};

PluginContext::PluginContext()
    : d_ptr(std::make_unique<PluginContextPrivate>())
{}

PluginContext::PluginContext(
    const PluginSpec *spec, QSettings *settings, Utils::FilePath resourceRoot)
    : d_ptr(std::make_unique<PluginContextPrivate>(spec, settings, std::move(resourceRoot)))
{}

PluginContext::~PluginContext() = default;

auto PluginContext::spec() const -> const PluginSpec &
{
    Q_ASSERT(d_ptr->spec);
    return *d_ptr->spec;
}

auto PluginContext::id() const -> QString
{
    return d_ptr->spec ? d_ptr->spec->id() : QString();
}

auto PluginContext::settings() const -> QSettings *
{
    return d_ptr->settings;
}

auto PluginContext::resourcePath(QAnyStringView relative) const -> Utils::FilePath
{
    return d_ptr->resourceRoot / relative.toString();
}

} // namespace ExtensionSystem
