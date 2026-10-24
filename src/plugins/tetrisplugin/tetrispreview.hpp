#pragma once

#include "tetrisgame.hpp"

#include <QWidget>

#include <memory>

namespace Tetris {

// 小方块预览：Next 队列与 Hold 各一个实例，只画 4×4 的格框，不掺和规则。
class TetrisPreview final : public QWidget
{
    Q_OBJECT

public:
    explicit TetrisPreview(QWidget *parent = nullptr);
    ~TetrisPreview() override;

    // 空列表表示占位（暂存为空），高度由 setSlots() 决定，避免版式跳动。
    void setPieces(const QVector<PieceType> &pieces);
    void setSlots(int count);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    class TetrisPreviewPrivate;
    std::unique_ptr<TetrisPreviewPrivate> d_ptr;
};

} // namespace Tetris
