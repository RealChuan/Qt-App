#include "pluginspec.hpp"

#include "extensionsystemtr.h"

#include <QJsonArray>
#include <QJsonObject>

#include <algorithm>
#include <array>
#include <charconv>

namespace ExtensionSystem {

struct PluginSpec::Private
{
    QString id;
    QString name;
    QString displayName;
    QString version;
    QString compatVersion;
    QString vendor;
    QString copyright;
    QString license;
    QString category;
    QString description;
    QString longDescription;
    QString url;

    bool required = false;
    bool enabledBySettings = true;

    QString error;
    Utils::FilePath filePath;
    IPlugin *plugin = nullptr;
    qint64 loadTimeMs = 0;

    PluginSpec::State state = PluginSpec::State::Invalid;
    QList<PluginDependency> dependencies;
    QList<PluginSpec *> dependencySpecs;
};

namespace {

auto readString(const QJsonObject &object, QLatin1StringView key) -> QString
{
    return object.value(key).toString();
}

auto parseDependency(const QJsonObject &object) -> PluginDependency
{
    PluginDependency dependency;
    dependency.id = readString(object, QLatin1StringView("Id"));
    dependency.version = readString(object, QLatin1StringView("Version"));
    const QString type = readString(object, QLatin1StringView("Type")).toLower();
    dependency.type = type == QLatin1String("optional") ? PluginDependency::Type::Optional
                                                        : PluginDependency::Type::Required;
    return dependency;
}

} // namespace

PluginSpec::PluginSpec()
    : d(std::make_unique<Private>())
{}

PluginSpec::~PluginSpec() = default;

auto PluginSpec::readMetaData(const QJsonObject &metaData) -> Utils::Result<>
{
    d->id = readString(metaData, QLatin1StringView("Id"));
    if (d->id.isEmpty())
        return Utils::ResultError(Tr::tr("Plugin metadata is missing the \"Id\" field."));

    d->name = readString(metaData, QLatin1StringView("Name"));
    d->displayName = readString(metaData, QLatin1StringView("DisplayName"));
    d->version = readString(metaData, QLatin1StringView("Version"));
    d->compatVersion = readString(metaData, QLatin1StringView("CompatVersion"));
    d->vendor = readString(metaData, QLatin1StringView("Vendor"));
    d->copyright = readString(metaData, QLatin1StringView("Copyright"));
    d->license = readString(metaData, QLatin1StringView("License"));
    d->category = readString(metaData, QLatin1StringView("Category"));
    d->description = readString(metaData, QLatin1StringView("Description"));
    d->longDescription = readString(metaData, QLatin1StringView("LongDescription"));
    d->url = readString(metaData, QLatin1StringView("Url"));
    d->required = metaData.value(QLatin1StringView("Required")).toBool(false);

    const QJsonValue dependencies = metaData.value(QLatin1StringView("Dependencies"));
    if (!dependencies.isUndefined() && !dependencies.isArray())
        return Utils::ResultError(Tr::tr("\"Dependencies\" must be an array."));
    for (const QJsonValue &value : dependencies.toArray()) {
        if (!value.isObject())
            return Utils::ResultError(Tr::tr("\"Dependencies\" must only contain objects."));
        d->dependencies.append(parseDependency(value.toObject()));
    }

    d->state = State::Read;
    return Utils::ResultOk;
}

auto PluginSpec::id() const -> QString
{
    return d->id;
}

auto PluginSpec::name() const -> QString
{
    return d->name;
}

auto PluginSpec::displayName() const -> QString
{
    return d->displayName.isEmpty() ? d->name : d->displayName;
}

auto PluginSpec::version() const -> QString
{
    return d->version;
}

auto PluginSpec::compatVersion() const -> QString
{
    return d->compatVersion.isEmpty() ? d->version : d->compatVersion;
}

auto PluginSpec::vendor() const -> QString
{
    return d->vendor;
}

auto PluginSpec::copyright() const -> QString
{
    return d->copyright;
}

auto PluginSpec::license() const -> QString
{
    return d->license;
}

auto PluginSpec::category() const -> QString
{
    return d->category;
}

auto PluginSpec::description() const -> QString
{
    return d->description;
}

auto PluginSpec::longDescription() const -> QString
{
    return d->longDescription.isEmpty() ? d->description : d->longDescription;
}

auto PluginSpec::url() const -> QString
{
    return d->url;
}

auto PluginSpec::state() const -> State
{
    return d->state;
}

auto PluginSpec::stateText() const -> QString
{
    switch (d->state) {
    case State::Invalid: return Tr::tr("Invalid");
    case State::Read: return Tr::tr("Read");
    case State::Resolved: return Tr::tr("Resolved");
    case State::Loaded: return Tr::tr("Loaded");
    case State::Initialized: return Tr::tr("Initialized");
    case State::Running: return Tr::tr("Running");
    case State::Stopped: return Tr::tr("Stopped");
    }
    return {};
}

auto PluginSpec::hasError() const -> bool
{
    return !d->error.isEmpty();
}

auto PluginSpec::errorString() const -> QString
{
    return d->error;
}

auto PluginSpec::isRequired() const -> bool
{
    return d->required;
}

auto PluginSpec::isEnabledBySettings() const -> bool
{
    return d->required || d->enabledBySettings;
}

auto PluginSpec::isEffectivelyEnabled() const -> bool
{
    if (isRequired())
        return true;
    if (!d->enabledBySettings)
        return false;
    return std::ranges::all_of(d->dependencySpecs, [](const PluginSpec *dependency) {
        return dependency->isEffectivelyEnabled();
    });
}

void PluginSpec::setEnabledBySettings(bool value)
{
    if (!d->required)
        d->enabledBySettings = value;
}

auto PluginSpec::dependencies() const -> QList<PluginDependency>
{
    return d->dependencies;
}

auto PluginSpec::dependencySpecs() const -> QList<PluginSpec *>
{
    return d->dependencySpecs;
}

auto PluginSpec::filePath() const -> Utils::FilePath
{
    return d->filePath;
}

auto PluginSpec::plugin() const -> IPlugin *
{
    return d->plugin;
}

auto PluginSpec::loadTimeMs() const -> qint64
{
    return d->loadTimeMs;
}

void PluginSpec::setState(State state)
{
    d->state = state;
}

void PluginSpec::setError(QString error)
{
    if (d->error.isEmpty())
        d->error = std::move(error);
}

void PluginSpec::setPlugin(IPlugin *plugin)
{
    d->plugin = plugin;
}

void PluginSpec::setFilePath(Utils::FilePath path)
{
    d->filePath = std::move(path);
}

void PluginSpec::setLoadTimeMs(qint64 ms)
{
    d->loadTimeMs = ms;
}

void PluginSpec::setDependencySpecs(QList<PluginSpec *> specs)
{
    d->dependencySpecs = std::move(specs);
}

auto PluginSpec::versionCompare(const QString &lhs, const QString &rhs) -> int
{
    const auto parse = [](const QString &value) {
        std::array<int, 4> parts{};
        const QByteArray utf8 = value.toUtf8();
        const char *begin = utf8.constData();
        const char *end = begin + utf8.size();
        for (auto &part : parts) {
            if (begin == end)
                break;
            const auto [ptr, ec] = std::from_chars(begin, end, part);
            if (ec != std::errc())
                break;
            begin = ptr;
            if (begin != end && *begin == '.')
                ++begin;
        }
        return parts;
    };

    const auto left = parse(lhs);
    const auto right = parse(rhs);
    if (left == right)
        return 0;
    return left < right ? -1 : 1;
}

} // namespace ExtensionSystem
