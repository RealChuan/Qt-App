#pragma once

#include "snakegame.hpp"

#include <shell/page.hpp>

#include <QWidget>

#include <memory>

using namespace Qt::StringLiterals;

namespace Snake {

// 一局游戏：状态与定时器都归页面持有，棋盘只负责画和收键。
//
// 离开页面刻意**不**暂停：蛇保持最后方向继续走，撞墙自然结束。
// 切走是用户自己的选择；想安全离开，先按 Space 暂停。所以这里没有 hideEvent。
class SnakePage final : public QWidget, public Shell::IPage
{
    Q_OBJECT

public:
    explicit SnakePage(QWidget *parent = nullptr);
    ~SnakePage() override;

    [[nodiscard]] auto id() const -> QString override { return u"snakeplugin.page"_s; }
    [[nodiscard]] auto title() const -> QString override { return tr("Snake"); }
    [[nodiscard]] auto subtitle() const -> QString override
    {
        return tr(
            "A whole round: fixed-step loop, keyboard focus and a best score kept in settings.");
    }
    [[nodiscard]] auto area() const -> Shell::PageArea override { return Shell::PageArea::Games; }
    [[nodiscard]] auto order() const -> int override { return Shell::PageOrder::kFirst; }
    [[nodiscard]] auto widget() -> QWidget * override { return this; }

    void setBestScore(int score);
    // 回填难度：插件按存档里的选择调用，会屏蔽下拉框的信号。
    void setDifficulty(Snake::Difficulty difficulty);
    // 命令入口：从头开一局。
    void startGame();

signals:
    // 只在真正结束时发一次（撞墙、自撞或棋盘塞满），插件据此刷新最高分。
    void gameFinished(int score);
    // 用户换难度；插件负责落盘并换回该档的最高分。
    void difficultyChanged(Snake::Difficulty difficulty);

private slots:
    void onFrameTimeout();
    void onPrimaryAction();
    void onPauseToggled();
    void onTurnRequested(Snake::Direction direction);
    void onDifficultyChanged(int index);

protected:
    void showEvent(QShowEvent *event) override;
    void changeEvent(QEvent *event) override;

private:
    void retranslateUi();
    void setupConnections();
    void updateBestLabel();
    [[nodiscard]] auto difficultyLabel(Snake::Difficulty difficulty) const -> QString;

    class SnakePagePrivate;
    std::unique_ptr<SnakePagePrivate> d_ptr;
};

} // namespace Snake
