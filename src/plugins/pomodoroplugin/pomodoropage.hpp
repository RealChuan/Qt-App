#pragma once

#include <shell/page.hpp>

#include <QWidget>

#include <memory>

namespace Pomodoro {

// 纯视图：只做布局，并把用户操作转成信号；倒计时状态由 PomodoroPlugin 持有。
class PomodoroPage final : public QWidget, public Shell::IPage
{
    Q_OBJECT

public:
    explicit PomodoroPage(QWidget *parent = nullptr);
    ~PomodoroPage() override;

    [[nodiscard]] auto id() const -> QString override
    {
        return QStringLiteral("pomodoroplugin.page");
    }
    [[nodiscard]] auto title() const -> QString override { return tr("Pomodoro"); }
    [[nodiscard]] auto subtitle() const -> QString override
    {
        return tr("A focus session with a countdown owned by the plugin, not by this page.");
    }
    [[nodiscard]] auto order() const -> int override { return Shell::PageOrder::kFourth; }
    [[nodiscard]] auto widget() -> QWidget * override { return this; }

    void showRemaining(int seconds, int total);
    void setRunning(bool running);
    void setPaused(bool paused);

signals:
    void startRequested(int seconds);
    void pauseToggled();
    void resetRequested();

private slots:
    void onStartClicked();
    void onDemoClicked();

protected:
    void changeEvent(QEvent *event) override;

private:
    void retranslateUi();
    void setupConnections();

    class PomodoroPagePrivate;
    std::unique_ptr<PomodoroPagePrivate> d_ptr;
};

} // namespace Pomodoro
