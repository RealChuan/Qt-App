#include "waitwidget.h"

#include <utils/utils.hpp>

#include <QHBoxLayout>
#include <QProgressBar>
#include <QTimer>

namespace Widgets {

namespace {
constexpr int kStep = 5;
constexpr int kStopValue = 95;
constexpr int kMaxValue = 100;
constexpr int kTickMs = 100;
} // namespace

class WaitWidget::WaitWidgetPrivate
{
public:
    explicit WaitWidgetPrivate(WaitWidget *q)
        : q_ptr(q)
        , progressBar(new QProgressBar(q))
        , timer(new QTimer(q))
    {
        progressBar->setMaximumHeight(5);
        progressBar->setTextVisible(false);
        progressBar->setRange(0, kMaxValue);
    }

    void setupUi()
    {
        auto *layout = new QHBoxLayout(q_ptr);
        layout->setContentsMargins({});
        layout->setSpacing(0);
        layout->addWidget(progressBar);
    }

    WaitWidget *q_ptr = nullptr;
    QProgressBar *progressBar = nullptr;
    QTimer *timer = nullptr;
};

WaitWidget::WaitWidget()
    : d_ptr(std::make_unique<WaitWidgetPrivate>(this))
{
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::WindowCloseButtonHint
                   | Qt::MSWindowsFixedSizeDialogHint);
    setAttribute(Qt::WA_TranslucentBackground);

    d_ptr->setupUi();
    connect(d_ptr->timer, &QTimer::timeout, this, &WaitWidget::updateProgressBar);
    d_ptr->timer->start(kTickMs);

    resize(600, 5);
    Utils::windowCenter(this);
}

WaitWidget::~WaitWidget() = default;

void WaitWidget::fullProgressBar()
{
    d_ptr->progressBar->setValue(kMaxValue);
}

void WaitWidget::updateProgressBar()
{
    const int value = d_ptr->progressBar->value() + kStep;
    if (value >= kStopValue) {
        d_ptr->timer->stop();
        return;
    }
    d_ptr->progressBar->setValue(value);
}

} // namespace Widgets
