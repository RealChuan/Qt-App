#include "tetrisboard.hpp"

#include "tetriscolors.hpp"

#include <utils/uihelpers.hpp>

#include <QEvent>
#include <QGuiApplication>
#include <QKeyEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPalette>

#include <algorithm>

namespace Tetris {

namespace {

constexpr int kBorderWidth = 1;
constexpr qreal kBoardRadius = 10.0;
constexpr qreal kBlockInset = 1.0;
constexpr qreal kBlockRadiusRatio = 0.18;
constexpr qreal kGhostAlpha = 0.45;
constexpr qreal kFlashAlpha = 0.70;
// 开局提示是"该按键了"的号召，比正文大一档，颜色也用正文色而不是占位色。
constexpr int kIdlePixelSize = 20;

} // namespace

class TetrisBoard::TetrisBoardPrivate
{
public:
    explicit TetrisBoardPrivate(TetrisBoard *q)
        : q_ptr(q)
    {}

    [[nodiscard]] auto cellSize() const -> int;
    [[nodiscard]] auto gridRect(int cell) const -> QRect;
    [[nodiscard]] auto cellRect(const QRect &grid, int cell, const QPoint &position) const
        -> QRectF;

    void paintBlock(QPainter &painter, const QRectF &area, const QColor &color) const;
    void paintGhost(QPainter &painter, const QRectF &area, const QColor &color) const;

    TetrisBoard *q_ptr = nullptr;
    const TetrisGame *game = nullptr;

    QString idleText;
    QString overTitle;
    QString pausedTitle;
    QString playAgainHint;
    QString resumeHint;
};

auto TetrisBoard::TetrisBoardPrivate::cellSize() const -> int
{
    return TetrisBoard::kCellSize;
}

auto TetrisBoard::TetrisBoardPrivate::gridRect(int cell) const -> QRect
{
    const int width = cell * kColumns;
    const int height = cell * kVisibleRows;
    return QRect((q_ptr->width() - width) / 2, (q_ptr->height() - height) / 2, width, height);
}

auto TetrisBoard::TetrisBoardPrivate::cellRect(
    const QRect &grid, int cell, const QPoint &position) const -> QRectF
{
    return QRectF(grid.left() + position.x() * cell, grid.top() + position.y() * cell, cell, cell);
}

void TetrisBoard::TetrisBoardPrivate::paintBlock(
    QPainter &painter, const QRectF &area, const QColor &color) const
{
    const qreal radius = std::max(2.0, area.width() * kBlockRadiusRatio);

    painter.setPen(Qt::NoPen);
    painter.setBrush(color);
    painter.drawRoundedRect(
        area.adjusted(kBlockInset, kBlockInset, -kBlockInset, -kBlockInset), radius, radius);
}

void TetrisBoard::TetrisBoardPrivate::paintGhost(
    QPainter &painter, const QRectF &area, const QColor &color) const
{
    QColor line = color;
    line.setAlphaF(kGhostAlpha);
    const qreal radius = std::max(2.0, area.width() * kBlockRadiusRatio);

    painter.setPen(QPen(line, 1.5));
    painter.setBrush(Qt::NoBrush);
    painter.drawRoundedRect(area.adjusted(1.5, 1.5, -1.5, -1.5), radius, radius);
}

TetrisBoard::TetrisBoard(QWidget *parent)
    : QWidget(parent)
    , d_ptr(std::make_unique<TetrisBoardPrivate>(this))
{
    // 键盘是唯一的输入来源；没有焦点就等于玩不了。
    setFocusPolicy(Qt::StrongFocus);
    // 固定成 10×20 格大小：多余的宽度只会变成"看起来能走、其实撞墙"的空白。
    setFixedSize(kPixelWidth, kPixelHeight);
    retranslateUi();
}

TetrisBoard::~TetrisBoard() = default;

void TetrisBoard::setGame(const TetrisGame *game)
{
    d_ptr->game = game;
    update();
}

void TetrisBoard::retranslateUi()
{
    d_ptr->idleText = tr("Press Enter to start");
    d_ptr->overTitle = tr("Game Over");
    d_ptr->pausedTitle = tr("Paused");
    d_ptr->playAgainHint = tr("Enter to play again");
    d_ptr->resumeHint = tr("P or Esc to resume");
    update();
}

void TetrisBoard::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    const QPalette palette = QGuiApplication::palette();
    const qreal inset = kBorderWidth / 2.0;
    const QRectF outer = QRectF(rect()).adjusted(inset, inset, -inset, -inset);

    QPainterPath shape;
    shape.addRoundedRect(outer, kBoardRadius, kBoardRadius);

    // 用 Dark（borderStrong）而不是 Mid：井的边界要和卡片底色拉开，否则一片糊。
    painter.setPen(QPen(palette.color(QPalette::Dark), kBorderWidth));
    painter.setBrush(palette.color(QPalette::AlternateBase));
    painter.drawPath(shape);

    if (!d_ptr->game)
        return;

    painter.save();
    painter.setClipPath(shape);

    const int cell = d_ptr->cellSize();
    if (cell > 0) {
        const QRect grid = d_ptr->gridRect(cell);

        for (int y = 0; y < kVisibleRows; ++y) {
            for (int x = 0; x < kColumns; ++x) {
                const int value = d_ptr->game->settledCellAt(x, y);
                if (value == 0)
                    continue;
                const auto type = static_cast<PieceType>(value - 1);
                d_ptr->paintBlock(
                    painter, d_ptr->cellRect(grid, cell, QPoint(x, y)), pieceColor(type));
            }
        }

        // 消行闪烁：盖一层随进度淡出的亮色，行本身还没被抹掉。
        const QVector<int> rows = d_ptr->game->clearingRows();
        if (!rows.isEmpty()) {
            QColor flash = palette.color(QPalette::Window);
            flash.setAlphaF(kFlashAlpha * (1.0 - d_ptr->game->clearingProgress()));
            painter.setPen(Qt::NoPen);
            painter.setBrush(flash);
            for (const int row : rows) {
                painter.drawRect(QRectF(grid.left(), grid.top() + row * cell, grid.width(), cell));
            }
        }

        const QColor activeColor = pieceColor(d_ptr->game->activeType());
        for (const QPoint &ghost : d_ptr->game->ghostCells()) {
            d_ptr->paintGhost(painter, d_ptr->cellRect(grid, cell, ghost), activeColor);
        }
        for (const QPoint &active : d_ptr->game->activeCells()) {
            d_ptr->paintBlock(painter, d_ptr->cellRect(grid, cell, active), activeColor);
        }
    }

    const GameState state = d_ptr->game->state();
    if (state == GameState::Paused) {
        Utils::paintOverlay(
            painter, QRectF(rect()), palette, d_ptr->pausedTitle, {}, d_ptr->resumeHint);
    } else if (state == GameState::Over) {
        Utils::paintOverlay(
            painter,
            QRectF(rect()),
            palette,
            d_ptr->overTitle,
            QString::number(d_ptr->game->score()),
            d_ptr->playAgainHint);
    } else if (state == GameState::Idle) {
        QFont font = painter.font();
        font.setPixelSize(kIdlePixelSize);
        font.setWeight(QFont::DemiBold);
        painter.setFont(font);
        painter.setPen(palette.color(QPalette::Text));
        painter.drawText(rect(), Qt::AlignCenter, d_ptr->idleText);
    }

    painter.restore();
}

void TetrisBoard::keyPressEvent(QKeyEvent *event)
{
    // 自动重复交给页面的 DAS/ARR，Qt 那套太慢也不可控。
    if (!event->isAutoRepeat()) {
        emit keyPressed(event->key());
        return;
    }
    event->accept();
}

void TetrisBoard::keyReleaseEvent(QKeyEvent *event)
{
    if (!event->isAutoRepeat()) {
        emit keyReleased(event->key());
        return;
    }
    event->accept();
}

void TetrisBoard::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
        retranslateUi();
    QWidget::changeEvent(event);
}

} // namespace Tetris
