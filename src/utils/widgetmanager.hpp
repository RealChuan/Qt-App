#pragma once

#include "utils_global.h"

#include <QObject>

class QSettings;

namespace Utils {

class UTILS_EXPORT WidgetManager : public QObject
{
    Q_OBJECT
public:
    explicit WidgetManager(QObject *parent = nullptr);
    ~WidgetManager();

    void setShowMask(bool show);
    bool showMask() const;

    void setBlurBackground(bool show);
    bool blurBackground() const;

    void saveSettings(QSettings &settings);
    void loadSettings(QSettings &settings);

private:
    class WidgetManagerPrivate;
    QScopedPointer<WidgetManagerPrivate> d_ptr;
};

} // namespace Utils
