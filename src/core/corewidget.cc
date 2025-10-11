#include "corewidget.hpp"

#include <QtWidgets>

namespace Core {

class CoreWidget::CoreWidgetPrivate
{
public:
    explicit CoreWidgetPrivate(CoreWidget *q)
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
    Type type = Type::Main;
};

CoreWidget::CoreWidget(QObject *parent)
    : QObject{parent}
    , d_ptr(new CoreWidgetPrivate(this))
{}

CoreWidget::~CoreWidget() {}

auto CoreWidget::button() const -> QPushButton *
{
    return d_ptr->buttonPtr.data();
}

auto CoreWidget::widget() const -> QWidget *
{
    return d_ptr->widgetPtr.data();
}

auto CoreWidget::type() const -> Type
{
    return d_ptr->type;
}

void CoreWidget::setWidget(QWidget *widget)
{
    d_ptr->widgetPtr = widget;
}

void CoreWidget::setButton(QPushButton *button, Type type)
{
    d_ptr->type = type;
    button->setProperty("Type", type);
    d_ptr->buttonPtr = button;
}

static CoreWidgetList coreWidgets;

void addCoreWidget(CoreWidget *widget)
{
    coreWidgets.append(widget);
}

auto getCoreWidgets() -> CoreWidgetList &
{
    return coreWidgets;
}

} // namespace Core