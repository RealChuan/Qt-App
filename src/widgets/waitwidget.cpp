#include "waitwidget.h"

#include <utils/utils.hpp>

#include <QtWidgets>

namespace Widgets {

class WaitWidget::WaitWidgetPrivate
{
public:
    explicit WaitWidgetPrivate(WaitWidget *q)
        : q_ptr(q)
    {
        processBar = new QProgressBar(q_ptr);
        processBar->setMaximumHeight(5);
        processBar->setTextVisible(false);
        processBar->setRange(0, 100);

        timer = new QTimer(q_ptr);
    }

    void setupUI()
    {
        auto *layout = new QHBoxLayout(q_ptr);
        layout->setContentsMargins({});
        layout->setSpacing(0);
        layout->addWidget(processBar);
    }

    QWidget *q_ptr;
    QProgressBar *processBar;
    QTimer *timer;
};

WaitWidget::WaitWidget()
    : d_ptr(new WaitWidgetPrivate(this))
{
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::WindowCloseButtonHint
                   | Qt::MSWindowsFixedSizeDialogHint);
    //setAttribute(Qt::WA_StyledBackground);
    setAttribute(Qt::WA_TranslucentBackground);
    d_ptr->setupUI();
    connect(d_ptr->timer, &QTimer::timeout, this, &WaitWidget::updateProgressBar);
    d_ptr->timer->start(100);
    resize(600, 5);
    Utils::windowCenter(this);
}

WaitWidget::~WaitWidget() {}

void WaitWidget::fullProgressBar()
{
    int value = d_ptr->processBar->value();
    if (value < 100) {
        d_ptr->processBar->setValue(100);
    }
}

void WaitWidget::updateProgressBar()
{
    int value = d_ptr->processBar->value() + 5;
    if (value >= 95) {
        d_ptr->timer->stop();
        return;
    }
    d_ptr->processBar->setValue(value);
}

} // namespace Widgets
