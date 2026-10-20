#pragma once

#include <QObject>

class QTimer;

namespace Pomodoro {

// 计时逻辑跑在独立线程里：QTimer 在 worker 线程创建，插件销毁时统一 quit + wait。
// 这是"插件如何管理自己的后台线程"的最小范例。
class PomodoroWorker final : public QObject
{
    Q_OBJECT

public:
    explicit PomodoroWorker(QObject *parent = nullptr);

public slots:
    void start(int seconds);
    void pause();
    void resume();
    void reset();

signals:
    void tick(int remainingSeconds);
    void finished();

private:
    void onTimeout();

    QTimer *m_timer = nullptr;
    int m_remaining = 0;
    bool m_paused = false;
};

} // namespace Pomodoro
