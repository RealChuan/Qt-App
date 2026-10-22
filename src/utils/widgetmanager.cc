#include "widgetmanager.hpp"

#include <QSettings>

using namespace Qt::StringLiterals;

namespace Utils {

class WidgetManager::WidgetManagerPrivate
{
public:
    bool showMask = true;
    bool blurBackground = true;
    ThemeMode themeMode = ThemeMode::System;
};

WidgetManager::WidgetManager(QObject *parent)
    : QObject(parent)
    , d_ptr(std::make_unique<WidgetManagerPrivate>())
{}

WidgetManager::~WidgetManager() = default;

void WidgetManager::setShowMask(bool show)
{
    d_ptr->showMask = show;
}

auto WidgetManager::showMask() const -> bool
{
    return d_ptr->showMask;
}

void WidgetManager::setBlurBackground(bool show)
{
    d_ptr->blurBackground = show;
}

auto WidgetManager::blurBackground() const -> bool
{
    return d_ptr->blurBackground;
}

void WidgetManager::setThemeMode(ThemeMode mode)
{
    d_ptr->themeMode = mode;
}

auto WidgetManager::themeMode() const -> ThemeMode
{
    return d_ptr->themeMode;
}

void WidgetManager::saveSettings(QSettings &settings) const
{
    settings.beginGroup(u"Widget"_s);
    settings.setValue(u"showMask"_s, d_ptr->showMask);
    settings.setValue(u"blurBackground"_s, d_ptr->blurBackground);
    settings.setValue(u"themeMode"_s, static_cast<int>(d_ptr->themeMode));
    settings.endGroup();
}

void WidgetManager::loadSettings(QSettings &settings)
{
    settings.beginGroup(u"Widget"_s);
    d_ptr->showMask = settings.value(u"showMask"_s, true).toBool();
    d_ptr->blurBackground = settings.value(u"blurBackground"_s, true).toBool();
    const int mode = settings.value(u"themeMode"_s, 0).toInt();
    d_ptr->themeMode = static_cast<ThemeMode>(
        mode >= static_cast<int>(ThemeMode::System) && mode <= static_cast<int>(ThemeMode::Dark)
            ? mode
            : 0);
    settings.endGroup();
}

} // namespace Utils
