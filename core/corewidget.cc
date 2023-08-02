#include "corewidget.hpp"

#include <QtWidgets>

namespace Core {

class CoreWidget::CoreWidgetPrivate
{
public:
    CoreWidgetPrivate(CoreWidget *q)
        : q_ptr(q)
    {}
    ~CoreWidgetPrivate()
    {
        if (!buttonPtr.isNull()) {
            delete buttonPtr.data();
        }
        if (!widgetPtr.isNull()) {
            delete widgetPtr.data();
        }
    }

    CoreWidget *q_ptr;

    QPointer<QPushButton> buttonPtr;
    QPointer<QWidget> widgetPtr;
};

CoreWidget::CoreWidget(QObject *parent)
    : QObject{parent}
    , d_ptr(new CoreWidgetPrivate(this))
{}

CoreWidget::~CoreWidget() {}

QPushButton *CoreWidget::button() const
{
    return d_ptr->buttonPtr.data();
}

QWidget *CoreWidget::widget() const
{
    return d_ptr->widgetPtr.data();
}

void CoreWidget::setWidget(QWidget *widget)
{
    d_ptr->widgetPtr = widget;
}

void CoreWidget::setButton(QPushButton *button, Type type)
{
    d_ptr->buttonPtr = button;
    d_ptr->buttonPtr->setProperty("Type", type);
}

} // namespace Core
