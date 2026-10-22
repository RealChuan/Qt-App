#pragma once

#include "theme.hpp"
#include "utils_global.hpp"

#include <QObject>

#include <memory>

class QSettings;

namespace Utils {

class UTILS_EXPORT WidgetManager : public QObject
{
    Q_OBJECT
public:
    explicit WidgetManager(QObject *parent = nullptr);
    ~WidgetManager() override;

    void setShowMask(bool show);
    [[nodiscard]] auto showMask() const -> bool;

    void setBlurBackground(bool show);
    [[nodiscard]] auto blurBackground() const -> bool;

    // 深色/浅色方案，默认跟随系统。
    void setThemeMode(ThemeMode mode);
    [[nodiscard]] auto themeMode() const -> ThemeMode;

    void saveSettings(QSettings &settings) const;
    void loadSettings(QSettings &settings);

private:
    class WidgetManagerPrivate;
    std::unique_ptr<WidgetManagerPrivate> d_ptr;
};

} // namespace Utils
