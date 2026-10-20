#pragma once

#include "utils_global.h"

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

    void saveSettings(QSettings &settings) const;
    void loadSettings(QSettings &settings);

private:
    class WidgetManagerPrivate;
    std::unique_ptr<WidgetManagerPrivate> d_ptr;
};

} // namespace Utils
