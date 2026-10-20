#include "pomodoropage.hpp"

#include <QHBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

namespace Pomodoro {

class PomodoroPage::PomodoroPagePrivate
{
public:
    explicit PomodoroPagePrivate(PomodoroPage *q)
        : q_ptr(q)
    {}

    void setupUi();
    void updateTime(int seconds);

    PomodoroPage *q_ptr = nullptr;
    QLabel *time = nullptr;
    QProgressBar *progress = nullptr;
    QPushButton *start = nullptr;
    QPushButton *pause = nullptr;
    QPushButton *reset = nullptr;
    QSpinBox *minutes = nullptr;
    bool paused = false;
    int total = 1;
};

void PomodoroPage::PomodoroPagePrivate::setupUi()
{
    time = new QLabel(QStringLiteral("25:00"), q_ptr);
    time->setAlignment(Qt::AlignCenter);
    time->setObjectName("HomeLabel");

    progress = new QProgressBar(q_ptr);
    progress->setRange(0, total);
    progress->setValue(0);
    progress->setTextVisible(false);

    minutes = new QSpinBox(q_ptr);
    minutes->setRange(1, 60);
    minutes->setValue(25);
    minutes->setSuffix(QObject::tr(" min"));

    start = new QPushButton(QObject::tr("Start"), q_ptr);
    pause = new QPushButton(QObject::tr("Pause"), q_ptr);
    reset = new QPushButton(QObject::tr("Reset"), q_ptr);
    auto *demo = new QPushButton(QObject::tr("Demo (10 s)"), q_ptr);

    auto *buttons = new QHBoxLayout;
    buttons->addWidget(start);
    buttons->addWidget(pause);
    buttons->addWidget(reset);
    buttons->addWidget(demo);
    buttons->addStretch();
    buttons->addWidget(minutes);

    auto *layout = new QVBoxLayout(q_ptr);
    layout->addStretch();
    layout->addWidget(time);
    layout->addWidget(progress);
    layout->addLayout(buttons);
    layout->addStretch();

    QObject::connect(start, &QPushButton::clicked, q_ptr, [this] {
        total = minutes->value() * 60;
        progress->setRange(0, total);
        paused = false;
        emit q_ptr->startRequested(total);
    });
    QObject::connect(demo, &QPushButton::clicked, q_ptr, [this] {
        total = 10;
        progress->setRange(0, total);
        paused = false;
        emit q_ptr->startRequested(total);
    });
    QObject::connect(pause, &QPushButton::clicked, q_ptr, [this] {
        if (paused) {
            paused = false;
            pause->setText(QObject::tr("Pause"));
            emit q_ptr->resumeRequested();
        } else {
            paused = true;
            pause->setText(QObject::tr("Resume"));
            emit q_ptr->pauseRequested();
        }
    });
    QObject::connect(reset, &QPushButton::clicked, q_ptr, [this] {
        paused = false;
        pause->setText(QObject::tr("Pause"));
        progress->setValue(0);
        updateTime(0);
        emit q_ptr->resetRequested();
    });
}

void PomodoroPage::PomodoroPagePrivate::updateTime(int seconds)
{
    time->setText(QStringLiteral("%1:%2")
                      .arg(seconds / 60, 2, 10, QLatin1Char('0'))
                      .arg(seconds % 60, 2, 10, QLatin1Char('0')));
}

PomodoroPage::PomodoroPage(QWidget *parent)
    : QWidget(parent)
    , d_ptr(std::make_unique<PomodoroPagePrivate>(this))
{
    d_ptr->setupUi();
}

PomodoroPage::~PomodoroPage() = default;

void PomodoroPage::setRemaining(int seconds, int total)
{
    if (total > 0)
        d_ptr->total = total;
    d_ptr->progress->setRange(0, qMax(1, d_ptr->total));
    d_ptr->progress->setValue(d_ptr->total - seconds);
    d_ptr->updateTime(seconds);
}

void PomodoroPage::setRunning(bool running)
{
    d_ptr->pause->setEnabled(running);
    d_ptr->start->setEnabled(!running);
}

} // namespace Pomodoro
