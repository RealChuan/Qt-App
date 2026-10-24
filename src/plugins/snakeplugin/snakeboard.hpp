#pragma once

#include "snakegame.hpp"

#include <QWidget>

#include <memory>

namespace Snake {

// 纯视图：把 SnakeGame 的状态画成一块棋盘，并把按键转成信号。
// 配色全部取自应用调色板（AlternateBase 底 / Mid 描边 / Highlight 蛇身），
// 所以浅色、深色两套方案不用各写一遍。
class SnakeBoard final : public QWidget
{
    Q_OBJECT

public:
    explicit SnakeBoard(QWidget *parent = nullptr);
    ~SnakeBoard() override;

    // 只读引用：状态归 SnakePage 持有，棋盘不掺和规则。
    void setGame(const SnakeGame *game);

signals:
    void turnRequested(Snake::Direction direction);
    // Space / Enter：开始、暂停、继续、再来一局，由页面按状态决定。
    void primaryActionRequested();
    // P / Esc：只切换暂停。
    void pauseToggled();

protected:
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void changeEvent(QEvent *event) override;

private:
    void retranslateUi();

    class SnakeBoardPrivate;
    std::unique_ptr<SnakeBoardPrivate> d_ptr;
};

} // namespace Snake
