#include "tetrispreview.hpp"

#include "tetriscolors.hpp"

#include <QGuiApplication>
#include <QPainter>
#include <QPalette>

#include <algorithm>

namespace Tetris {

namespace {

constexpr int kCellSize = 16;
constexpr int kBoxCells = 4;
constexpr int kGap = 6;
constexpr qreal kBlockRadius = 2.0;
constexpr qreal kBlockInset = 0.5;

} // namespace

class TetrisPreview::TetrisPreviewPrivate
{
public:
    TetrisPreviewPrivate(TetrisPreview *q, int count)
        : q_ptr(q)
        , slotCount(count)
    {}

    void updateGeometry();

    TetrisPreview *q_ptr = nullptr;
    QVector<PieceType> pieces;
    // 名字不能叫 slots：Qt 把它定义成了宏。
    int slotCount = 1;
};

void TetrisPreview::TetrisPreviewPrivate::updateGeometry()
{
    const int box = kBoxCells * kCellSize;
    q_ptr->setFixedSize(box, slotCount * box + (slotCount - 1) * kGap);
}

TetrisPreview::TetrisPreview(QWidget *parent)
    : QWidget(parent)
    , d_ptr(std::make_unique<TetrisPreviewPrivate>(this, 1))
{
    d_ptr->updateGeometry();
}

TetrisPreview::~TetrisPreview() = default;

void TetrisPreview::setPieces(const QVector<PieceType> &pieces)
{
    if (d_ptr->pieces == pieces)
        return;

    d_ptr->pieces = pieces;
    update();
}

void TetrisPreview::setSlots(int count)
{
    const int clamped = std::max(1, count);
    if (d_ptr->slotCount == clamped)
        return;

    d_ptr->slotCount = clamped;
    d_ptr->updateGeometry();
    update();
}

void TetrisPreview::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    const QPalette palette = QGuiApplication::palette();
    const int box = kBoxCells * kCellSize;

    // 空格位画一条虚线框，告诉玩家这里会有东西。
    QColor slotLine = palette.color(QPalette::PlaceholderText);
    slotLine.setAlphaF(0.35F);

    const int count = std::min(static_cast<int>(d_ptr->pieces.size()), d_ptr->slotCount);
    for (int slot = 0; slot < d_ptr->slotCount; ++slot) {
        const QRectF area(0, slot * (box + kGap), box, box);

        if (slot >= count) {
            painter.setPen(QPen(slotLine, 1.0, Qt::DashLine));
            painter.setBrush(Qt::NoBrush);
            painter.drawRoundedRect(area.adjusted(0.5, 0.5, -0.5, -0.5), 4.0, 4.0);
            continue;
        }

        const PieceType type = d_ptr->pieces.at(slot);
        const PieceShape shape = shapeOf(type);
        const int offset = ((kBoxCells - shape.boxSize) * kCellSize) / 2;
        const QColor color = pieceColor(type);

        painter.setPen(Qt::NoPen);
        painter.setBrush(color);
        for (const QPoint &cell : shape.cells) {
            const QRectF target(
                offset + cell.x() * kCellSize + kBlockInset,
                area.top() + offset + cell.y() * kCellSize + kBlockInset,
                kCellSize - 2 * kBlockInset,
                kCellSize - 2 * kBlockInset);
            painter.drawRoundedRect(target, kBlockRadius, kBlockRadius);
        }
    }
}

} // namespace Tetris
