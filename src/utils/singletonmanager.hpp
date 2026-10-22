#pragma once

#include "languagemanager.hpp"
#include "singleton.hpp"
#include "theme.hpp"
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
    [[nodiscard]] auto theme() -> Theme *;

    void saveSettings();

private:
    explicit SingletonManager(QObject *parent = nullptr);
    ~SingletonManager() override;

    void loadSettings();

    class SingletonManagerPrivate;
    std::unique_ptr<SingletonManagerPrivate> d_ptr;

    SINGLETON(SingletonManager)
};

inline SingletonManager *singletonManager() noexcept
{
    return SingletonManager::instance();
}

inline Utils::LanguageManager *languageManager() noexcept
{
    return singletonManager()->languageManager();
}

inline Utils::WidgetManager *widgetManager() noexcept
{
    return singletonManager()->widgetManager();
}

inline Utils::Theme *theme() noexcept
{
    return singletonManager()->theme();
}

} // namespace Utils
