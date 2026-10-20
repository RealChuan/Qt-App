#pragma once

#include "languagemanager.hpp"
#include "singleton.hpp"
#include "widgetmanager.hpp"

#include <QObject>

#include <memory>

namespace Utils {

class UTILS_EXPORT SingletonManager : public QObject
{
    Q_OBJECT
public:
    [[nodiscard]] auto languageManager() -> LanguageManager *;
    [[nodiscard]] auto widgetManager() -> WidgetManager *;

    void saveSettings();

private:
    explicit SingletonManager(QObject *parent = nullptr);
    ~SingletonManager() override;

    void loadSettings();

    class SingletonManagerPrivate;
    std::unique_ptr<SingletonManagerPrivate> d_ptr;

    SINGLETON(SingletonManager)
};

} // namespace Utils

#define LANGUAGE_MANAGER Utils::SingletonManager::instance()->languageManager()
#define WIDGET_MANAGER Utils::SingletonManager::instance()->widgetManager()
