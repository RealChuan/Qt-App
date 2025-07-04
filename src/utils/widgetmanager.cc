#include "widgetmanager.hpp"

#include <QSettings>

namespace Utils {

class WidgetManager::WidgetManagerPrivate
{
public:
    explicit WidgetManagerPrivate(WidgetManager *q)
        : q_ptr(q)
    {}

    WidgetManager *q_ptr;

    bool showMask = true;
    bool blurBackground = true;
};

WidgetManager::WidgetManager(QObject *parent)
    : QObject{parent}
    , d_ptr(new WidgetManagerPrivate(this))
{}

WidgetManager::~WidgetManager() {}

void WidgetManager::setShowMask(bool show)
{
    d_ptr->showMask = show;
}

bool WidgetManager::showMask() const
{
    return d_ptr->showMask;
}

void WidgetManager::setBlurBackground(bool show)
{
    d_ptr->blurBackground = show;
}

bool WidgetManager::blurBackground() const
{
    return d_ptr->blurBackground;
}

void WidgetManager::saveSettings(QSettings &settings)
{
    settings.beginGroup("Widget");
    settings.setValue("showMask", d_ptr->showMask);
    settings.setValue("blurBackground", d_ptr->blurBackground);
    settings.endGroup();
}

void WidgetManager::loadSettings(QSettings &settings)
{
    settings.beginGroup("Widget");
    d_ptr->showMask = settings.value("showMask", true).toBool();
    d_ptr->blurBackground = settings.value("blurBackground", true).toBool();
    settings.endGroup();
}

} // namespace Utils
