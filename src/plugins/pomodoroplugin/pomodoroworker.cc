#include "pomodoroworker.hpp"

#include <QTimer>

namespace Pomodoro {

PomodoroWorker::PomodoroWorker(QObject *parent)
    : QObject(parent)
{}

void PomodoroWorker::start(int seconds)
{
    // 第一次 start() 在 worker 线程里执行，因此定时器天然属于该线程。
    if (!m_timer) {
        m_timer = new QTimer(this);
        m_timer->setInterval(1000);
        m_timer->setTimerType(Qt::PreciseTimer);
        connect(m_timer, &QTimer::timeout, this, &PomodoroWorker::onTimeout);
    }

    m_remaining = qMax(1, seconds);
    m_paused = false;
    m_timer->start();
    emit tick(m_remaining);
}

void PomodoroWorker::pause()
{
    if (!m_timer || !m_timer->isActive())
        return;
    m_timer->stop();
    m_paused = true;
}

void PomodoroWorker::resume()
{
    if (!m_timer || !m_paused)
        return;
    m_paused = false;
    m_timer->start();
}

void PomodoroWorker::reset()
{
    if (m_timer)
        m_timer->stop();
    m_remaining = 0;
    m_paused = false;
    emit tick(0);
}

void PomodoroWorker::onTimeout()
{
    --m_remaining;
    emit tick(m_remaining);
    if (m_remaining <= 0) {
        m_timer->stop();
        emit finished();
    }
}

} // namespace Pomodoro
