#include "singletonmanager.hpp"
#include "utils.h"

#include <QSettings>

namespace Utils {

class SingletonManager::SingletonManagerPrivate
{
public:
    explicit SingletonManagerPrivate(SingletonManager *q)
        : q_ptr(q)
        , languageManager(new LanguageManager(q_ptr))
        , widgetManager(new WidgetManager(q_ptr))
    {}

    SingletonManager *q_ptr;

    LanguageManager *languageManager;
    WidgetManager *widgetManager;
};

LanguageManager *SingletonManager::languageManager()
{
    return d_ptr->languageManager;
}

WidgetManager *SingletonManager::widgetManager()
{
    return d_ptr->widgetManager;
}

void SingletonManager::saveSettings()
{
    QSettings settings(Utils::configFilePath(), QSettings::IniFormat);
    d_ptr->languageManager->saveSettings(settings);
    d_ptr->widgetManager->saveSettings(settings);
}

SingletonManager::SingletonManager(QObject *parent)
    : QObject{parent}
    , d_ptr(new SingletonManagerPrivate(this))
{
    loadSettings();
}

SingletonManager::~SingletonManager()
{
    saveSettings();
}

void SingletonManager::loadSettings()
{
    QSettings settings(Utils::configFilePath(), QSettings::IniFormat);
    d_ptr->languageManager->loadSettings(settings);
    d_ptr->widgetManager->loadSettings(settings);
}

} // namespace Utils
