#pragma once

#include "tetrisgame.hpp"

#include <QWidget>

#include <memory>

namespace Tetris {

// 纯视图：画 10×20 的井、已落方块、幽灵、消行闪烁，并把按键原样转给页面。
// 连发节奏归页面管，这里只负责"键按下了 / 键松开了"。
class TetrisBoard final : public QWidget
{
    Q_OBJECT

public:
    // 井的像素尺寸只在这里定义一次：控件多宽井就多宽，页面按它排卡片。
    static constexpr int kCellSize = 20;
    static constexpr int kPixelWidth = kColumns * kCellSize;
    static constexpr int kPixelHeight = kVisibleRows * kCellSize;

    explicit TetrisBoard(QWidget *parent = nullptr);
    ~TetrisBoard() override;

    void setGame(const TetrisGame *game);

signals:
    void keyPressed(int key);
    void keyReleased(int key);

protected:
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void changeEvent(QEvent *event) override;

private:
    void retranslateUi();

    class TetrisBoardPrivate;
    std::unique_ptr<TetrisBoardPrivate> d_ptr;
};

} // namespace Tetris
