#include "settings.h"

#include <memory>
#include <utility>

namespace Utils {

namespace {
std::unique_ptr<QSettings> s_userSettings;
std::unique_ptr<QSettings> s_installSettings;
} // namespace

auto userSettings() -> QSettings &
{
    static QSettings fallback;
    return s_userSettings ? *s_userSettings : fallback;
}

auto installSettings() -> QSettings &
{
    static QSettings fallback;
    return s_installSettings ? *s_installSettings : fallback;
}

void setupSettings(QSettings *user, QSettings *install)
{
    s_userSettings.reset(user);
    s_installSettings.reset(install);
}

void destroySettings()
{
    s_userSettings.reset();
    s_installSettings.reset();
}

} // namespace Utils
