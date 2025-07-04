#pragma once

#include "languagemanager.hpp"
#include "singleton.hpp"
#include "widgetmanager.hpp"

namespace Utils {

class UTILS_EXPORT SingletonManager : public QObject
{
    Q_OBJECT
public:
    LanguageManager *languageManager();
    WidgetManager *widgetManager();

    void saveSettings();

private:
    explicit SingletonManager(QObject *parent = nullptr);
    ~SingletonManager();

    void loadSettings();

    class SingletonManagerPrivate;
    QScopedPointer<SingletonManagerPrivate> d_ptr;

    SINGLETON(SingletonManager)
};

} // namespace Utils

#define LANGUAGE_MANAGER Utils::SingletonManager::instance()->languageManager()
#define WIDGET_MANAGER Utils::SingletonManager::instance()->widgetManager()
