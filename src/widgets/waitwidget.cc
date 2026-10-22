#include "waitwidget.hpp"

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
constexpr int kBarHeight = 5;
} // namespace

class WaitWidget::WaitWidgetPrivate
{
public:
    explicit WaitWidgetPrivate(WaitWidget *q)
        : q_ptr(q)
        , progressBar(new QProgressBar(q))
        , timer(new QTimer(q))
    {
        progressBar->setMaximumHeight(kBarHeight);
        progressBar->setTextVisible(false);
        progressBar->setRange(0, kMaxValue);

        auto *layout = new QHBoxLayout(q);
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
    setWindowFlags(
        windowFlags() | Qt::FramelessWindowHint | Qt::WindowCloseButtonHint
        | Qt::MSWindowsFixedSizeDialogHint);
    setAttribute(Qt::WA_TranslucentBackground);

    connect(d_ptr->timer, &QTimer::timeout, this, &WaitWidget::onProgressTick);
    d_ptr->timer->start(kTickMs);

    resize(600, kBarHeight);
    Utils::windowCenter(this);
}

WaitWidget::~WaitWidget() = default;

void WaitWidget::fullProgressBar()
{
    d_ptr->timer->stop();
    d_ptr->progressBar->setValue(kMaxValue);
}

void WaitWidget::onProgressTick()
{
    const int value = d_ptr->progressBar->value() + kStep;
    if (value >= kStopValue) {
        // 先落到 kStopValue 再停表，避免停在 90% 这种半截状态。
        d_ptr->progressBar->setValue(kStopValue);
        d_ptr->timer->stop();
        return;
    }
    d_ptr->progressBar->setValue(value);
}

} // namespace Widgets
