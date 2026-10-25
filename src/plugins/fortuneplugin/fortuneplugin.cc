#include "fortuneplugin.hpp"

#include "fortunelunar.hpp"
#include "fortunepage.hpp"

#include <extensionsystem/plugincontext.hpp>
#include <shell/functioncommand.hpp>
#include <shell/host.hpp>
#include <shell/registry.hpp>

#include <QKeySequence>
#include <QSettings>

namespace Fortune {

namespace {

constexpr QLatin1StringView kNameKey("profile/name");
constexpr QLatin1StringView kBirthDateKey("profile/birthDate");
constexpr QLatin1StringView kBirthHourKey("profile/birthHour");
constexpr QLatin1StringView kProfileGroup("profile");
constexpr QLatin1StringView kHistoryGroup("history");
constexpr QLatin1StringView kAtKey("at");
constexpr QLatin1StringView kHexagramKey("hexagram");
constexpr QLatin1StringView kLineKey("line");
constexpr QLatin1StringView kGradeKey("grade");

} // namespace

class FortunePlugin::FortunePluginPrivate
{
public:
    QSettings *settings = nullptr;
    Fortunepage *page = nullptr;
    std::unique_ptr<Shell::FunctionCommand> command;
};

FortunePlugin::FortunePlugin()
    : d_ptr(std::make_unique<FortunePluginPrivate>())
{}

FortunePlugin::~FortunePlugin() = default;

auto FortunePlugin::initialize(const ExtensionSystem::PluginContext &context) -> Utils::Result<>
{
    d_ptr->settings = context.settings();
    d_ptr->page = new Fortunepage;

    const QDate birthDate
        = QDate::fromString(d_ptr->settings->value(kBirthDateKey).toString(), Qt::ISODate);
    if (isLunarDateSupported(birthDate)) {
        d_ptr->page->setProfile(
            Profile{
                d_ptr->settings->value(kNameKey).toString(),
                birthDate,
                d_ptr->settings->value(kBirthHourKey, 0).toInt()});
    }

    QList<HistoryEntry> history;
    const int historyCount = d_ptr->settings->beginReadArray(kHistoryGroup);
    for (int i = 0; i < historyCount; ++i) {
        d_ptr->settings->setArrayIndex(i);
        HistoryEntry entry;
        entry.castAt = QDateTime::fromString(d_ptr->settings->value(kAtKey).toString(), Qt::ISODate);
        entry.hexagram = d_ptr->settings->value(kHexagramKey, 1).toInt();
        entry.movingLine = d_ptr->settings->value(kLineKey, 1).toInt();
        entry.grade = Grade(d_ptr->settings->value(kGradeKey, int(Grade::Fair)).toInt());
        history.append(entry);
    }
    d_ptr->settings->endArray();
    d_ptr->page->setHistory(history);

    d_ptr->command = std::make_unique<Shell::FunctionCommand>(
        QStringLiteral("fortuneplugin.cast"),
        tr("Cyber Fortune: cast"),
        std::bind_front(&FortunePlugin::castFromCommand, this));
    d_ptr->command->withShortcut(QKeySequence(QStringLiteral("Ctrl+Alt+G")));

    connect(d_ptr->page, &Fortunepage::profileSaved, this, &FortunePlugin::onProfileSaved);
    connect(d_ptr->page, &Fortunepage::profileCleared, this, &FortunePlugin::onProfileCleared);
    connect(d_ptr->page, &Fortunepage::fortuneCasted, this, &FortunePlugin::onFortuneCasted);

    return Utils::ResultOk;
}

void FortunePlugin::extensionsInitialized()
{
    auto *registry = Shell::Registry::instance();
    registry->addPage(d_ptr->page);
    registry->addCommand(d_ptr->command.get());
}

void FortunePlugin::castFromCommand()
{
    if (auto *shellHost = Shell::host())
        shellHost->showPage(QStringLiteral("fortuneplugin.page"));

    if (d_ptr->page->hasProfile())
        d_ptr->page->castNow();
    else
        d_ptr->page->focusProfileForm();
}

void FortunePlugin::onProfileSaved(const Profile &profile)
{
    d_ptr->settings->setValue(kNameKey, profile.name);
    d_ptr->settings->setValue(kBirthDateKey, profile.birthDate.toString(Qt::ISODate));
    d_ptr->settings->setValue(kBirthHourKey, profile.birthHour);
}

void FortunePlugin::onProfileCleared()
{
    d_ptr->settings->remove(kProfileGroup);
    d_ptr->settings->remove(kHistoryGroup);
}

void FortunePlugin::onFortuneCasted(const HistoryEntry &entry)
{
    Q_UNUSED(entry)

    const QList<HistoryEntry> history = d_ptr->page->history();
    d_ptr->settings->beginWriteArray(kHistoryGroup, int(history.size()));
    for (int i = 0; i < history.size(); ++i) {
        d_ptr->settings->setArrayIndex(i);
        d_ptr->settings->setValue(kAtKey, history.at(i).castAt.toString(Qt::ISODate));
        d_ptr->settings->setValue(kHexagramKey, history.at(i).hexagram);
        d_ptr->settings->setValue(kLineKey, history.at(i).movingLine);
        d_ptr->settings->setValue(kGradeKey, int(history.at(i).grade));
    }
    d_ptr->settings->endArray();
}

} // namespace Fortune
