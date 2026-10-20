#include "hellowidget.hpp"

#include <QLabel>
#include <QVBoxLayout>

namespace Hello {

class HelloWidget::HelloWidgetPrivate
{
public:
    explicit HelloWidgetPrivate(HelloWidget *q)
        : q_ptr(q)
    {}

    void updateText();

    HelloWidget *q_ptr = nullptr;
    QLabel *label = nullptr;
    int count = 0;
};

void HelloWidget::HelloWidgetPrivate::updateText()
{
    label->setText(QObject::tr("Hello, Qt plugin!\n\nCount: %1\n\n"
                               "Try Ctrl+K and type \"hello\".")
                       .arg(count));
}

HelloWidget::HelloWidget(QWidget *parent)
    : QWidget(parent)
    , d_ptr(std::make_unique<HelloWidgetPrivate>(this))
{
    d_ptr->label = new QLabel(this);
    d_ptr->label->setAlignment(Qt::AlignCenter);
    d_ptr->label->setObjectName("HomeLabel");

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(d_ptr->label, 1);

    d_ptr->updateText();
}

HelloWidget::~HelloWidget() = default;

void HelloWidget::setCount(int count)
{
    d_ptr->count = count;
    d_ptr->updateText();
}

void HelloWidget::increment()
{
    ++d_ptr->count;
    d_ptr->updateText();
    emit countChanged(d_ptr->count);
}

} // namespace Hello
