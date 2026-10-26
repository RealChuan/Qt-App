#include "pomodoropage.hpp"

#include <QEvent>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QProgressBar>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

#include <algorithm>

namespace Pomodoro {

namespace {

constexpr int kDefaultMinutes = 25;
constexpr int kDemoSeconds = 10;
constexpr int kSecondsPerMinute = 60;
// 文字与边框的间距。QSS 的 padding 在 windowsvista 样式下不会作用到自旋框内部的
// 编辑框（实测文字始终贴着边框），只能直接设 textMargins。
constexpr int kTextMargin = 8;

// 每次重新 polish（换主题、换样式）都会重置内部编辑框的 textMargins，
// 所以放在 StyleChange 里补，而不是构造时设一次。
class MinutesSpinBox final : public QSpinBox
{
public:
    explicit MinutesSpinBox(QWidget *parent = nullptr)
        : QSpinBox(parent)
    {
        applyTextMargin();
    }

protected:
    void changeEvent(QEvent *event) override
    {
        QSpinBox::changeEvent(event);

        if (event->type() == QEvent::StyleChange)
            applyTextMargin();
    }

private:
    void applyTextMargin()
    {
        if (QLineEdit *edit = lineEdit())
            edit->setTextMargins(kTextMargin, 0, kTextMargin, 0);
    }
};

auto formatTime(int seconds) -> QString
{
    return u"%1:%2"_s.arg(seconds / kSecondsPerMinute, 2, 10, QLatin1Char('0'))
        .arg(seconds % kSecondsPerMinute, 2, 10, QLatin1Char('0'));
}

} // namespace

class PomodoroPage::PomodoroPagePrivate
{
public:
    explicit PomodoroPagePrivate(PomodoroPage *q)
        : q_ptr(q)
    {}

    void setupUi();

    PomodoroPage *q_ptr = nullptr;
    QLabel *time = nullptr;
    QLabel *lengthLabel = nullptr;
    QProgressBar *progress = nullptr;
    QPushButton *start = nullptr;
    QPushButton *pause = nullptr;
    QPushButton *reset = nullptr;
    QPushButton *demo = nullptr;
    QSpinBox *minutes = nullptr;
    bool paused = false;
};

// 只建控件、排版；文案在 retranslateUi()，信号在 setupConnections()。
void PomodoroPage::PomodoroPagePrivate::setupUi()
{
    time = new QLabel(q_ptr);
    time->setObjectName("HeroNumber");
    time->setAlignment(Qt::AlignCenter);

    progress = new QProgressBar(q_ptr);
    progress->setTextVisible(false);
    progress->setFixedHeight(6);

    minutes = new MinutesSpinBox(q_ptr);
    minutes->setObjectName("PomodoroMinutes");
    minutes->setRange(1, 60);
    minutes->setValue(kDefaultMinutes);

    lengthLabel = new QLabel(q_ptr);
    lengthLabel->setObjectName("FieldLabel");

    start = new QPushButton(q_ptr);
    start->setObjectName("PrimaryButton");
    start->setCursor(Qt::PointingHandCursor);
    pause = new QPushButton(q_ptr);
    pause->setCursor(Qt::PointingHandCursor);
    reset = new QPushButton(q_ptr);
    reset->setCursor(Qt::PointingHandCursor);
    demo = new QPushButton(q_ptr);
    demo->setCursor(Qt::PointingHandCursor);

    auto *controls = new QHBoxLayout;
    controls->setContentsMargins({});
    controls->setSpacing(8);
    controls->addWidget(start);
    controls->addWidget(pause);
    controls->addWidget(reset);
    controls->addStretch();

    auto *lengthRow = new QHBoxLayout;
    lengthRow->setContentsMargins({});
    lengthRow->setSpacing(8);
    lengthRow->addWidget(lengthLabel);
    lengthRow->addWidget(minutes);
    lengthRow->addStretch();
    lengthRow->addWidget(demo);

    auto *divider = new QFrame(q_ptr);
    divider->setObjectName("Divider");
    divider->setFrameShape(QFrame::NoFrame);

    auto *card = new QFrame(q_ptr);
    card->setObjectName("Card");
    card->setFixedWidth(480);
    auto *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(32, 28, 32, 26);
    cardLayout->setSpacing(14);
    cardLayout->addWidget(time);
    cardLayout->addWidget(progress);
    cardLayout->addSpacing(6);
    cardLayout->addLayout(controls);
    cardLayout->addSpacing(2);
    cardLayout->addWidget(divider);
    cardLayout->addLayout(lengthRow);

    auto *centerRow = new QHBoxLayout;
    centerRow->addStretch();
    centerRow->addWidget(card);
    centerRow->addStretch();

    auto *layout = new QVBoxLayout(q_ptr);
    layout->setContentsMargins({});
    layout->addStretch();
    layout->addLayout(centerRow);
    layout->addStretch();
}

PomodoroPage::PomodoroPage(QWidget *parent)
    : QWidget(parent)
    , d_ptr(std::make_unique<PomodoroPagePrivate>(this))
{
    d_ptr->setupUi();
    retranslateUi();
    setupConnections();
    showRemaining(kDefaultMinutes * kSecondsPerMinute, kDefaultMinutes * kSecondsPerMinute);
}

PomodoroPage::~PomodoroPage() = default;

void PomodoroPage::retranslateUi()
{
    d_ptr->minutes->setSuffix(tr(" min"));
    d_ptr->lengthLabel->setText(tr("Session"));
    d_ptr->start->setText(tr("Start"));
    d_ptr->reset->setText(tr("Reset"));
    d_ptr->demo->setText(tr("Demo (10 s)"));
    d_ptr->pause->setText(d_ptr->paused ? tr("Resume") : tr("Pause"));
}

void PomodoroPage::setupConnections()
{
    connect(d_ptr->start, &QPushButton::clicked, this, &PomodoroPage::onStartClicked);
    connect(d_ptr->demo, &QPushButton::clicked, this, &PomodoroPage::onDemoClicked);
    connect(d_ptr->pause, &QPushButton::clicked, this, &PomodoroPage::pauseToggled);
    connect(d_ptr->reset, &QPushButton::clicked, this, &PomodoroPage::resetRequested);
}

void PomodoroPage::onStartClicked()
{
    emit startRequested(d_ptr->minutes->value() * kSecondsPerMinute);
}

void PomodoroPage::onDemoClicked()
{
    emit startRequested(kDemoSeconds);
}

void PomodoroPage::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
        retranslateUi();
    QWidget::changeEvent(event);
}

void PomodoroPage::showRemaining(int seconds, int total)
{
    const int clampedTotal = std::max(1, total);
    d_ptr->progress->setRange(0, clampedTotal);
    d_ptr->progress->setValue(clampedTotal - seconds);
    d_ptr->time->setText(formatTime(seconds));
}

void PomodoroPage::setRunning(bool running)
{
    d_ptr->start->setEnabled(!running);
    d_ptr->demo->setEnabled(!running);
    d_ptr->minutes->setEnabled(!running);
    d_ptr->pause->setEnabled(running);
}

void PomodoroPage::setPaused(bool paused)
{
    d_ptr->paused = paused;
    d_ptr->pause->setText(paused ? tr("Resume") : tr("Pause"));
}

} // namespace Pomodoro
