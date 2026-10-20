#include "singletonmanager.hpp"

#include "utils.hpp"

using namespace Qt::StringLiterals;

namespace Utils {

class SingletonManager::SingletonManagerPrivate
{
public:
    explicit SingletonManagerPrivate(SingletonManager *q)
        : languageManager(new LanguageManager(q))
        , widgetManager(new WidgetManager(q))
    {}

    LanguageManager *languageManager = nullptr;
    WidgetManager *widgetManager = nullptr;
};

auto SingletonManager::languageManager() -> LanguageManager *
{
    return d_ptr->languageManager;
}

auto SingletonManager::widgetManager() -> WidgetManager *
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
    : QObject(parent)
    , d_ptr(std::make_unique<SingletonManagerPrivate>(this))
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
