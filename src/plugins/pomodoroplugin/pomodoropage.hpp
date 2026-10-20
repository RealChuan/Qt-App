#pragma once

#include <shell/page.hpp>

#include <QWidget>

#include <memory>

namespace Pomodoro {

class PomodoroPage final : public QWidget, public Shell::IPage
{
    Q_OBJECT

public:
    explicit PomodoroPage(QWidget *parent = nullptr);
    ~PomodoroPage() override;

    auto id() const -> QString override { return QStringLiteral("pomodoroplugin.page"); }
    auto title() const -> QString override { return tr("Pomodoro"); }
    auto order() const -> int override { return 40; }
    auto widget() -> QWidget * override { return this; }

    void setRemaining(int seconds, int total);
    void setRunning(bool running);

signals:
    void startRequested(int seconds);
    void pauseRequested();
    void resumeRequested();
    void resetRequested();

private:
    class PomodoroPagePrivate;
    std::unique_ptr<PomodoroPagePrivate> d_ptr;
};

} // namespace Pomodoro
