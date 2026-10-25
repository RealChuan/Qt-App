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

constexpr qreal kBlockInset = 1.0;
constexpr qreal kBlockRadiusRatio = 0.18;
// QColor::setAlphaF 是 float 接口，透明度常量直接用 float。
constexpr float kGhostAlpha = 0.45F;
constexpr float kFlashAlpha = 0.70F;
// 幽灵是描边不是实心块，内缩比实心块多半个像素，两者叠在一起才不糊边。
constexpr qreal kGhostInset = 1.5;
constexpr qreal kGhostPenWidth = 1.5;

} // namespace

class TetrisBoard::TetrisBoardPrivate
{
public:
    explicit TetrisBoardPrivate(TetrisBoard *q)
        : q_ptr(q)
    {}

    [[nodiscard]] auto metrics() const -> Utils::GridMetrics;

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

auto TetrisBoard::TetrisBoardPrivate::metrics() const -> Utils::GridMetrics
{
    // 控件尺寸本来就是格子的整数倍，这里的 cell 恒等于 TetrisBoard::kCellSize。
    return Utils::fitGrid(q_ptr->size(), kColumns, kVisibleRows);
}

void TetrisBoard::TetrisBoardPrivate::paintBlock(
    QPainter &painter, const QRectF &area, const QColor &color) const
{
    Utils::paintCell(painter, area, color, {kBlockInset, kBlockRadiusRatio});
}

void TetrisBoard::TetrisBoardPrivate::paintGhost(
    QPainter &painter, const QRectF &area, const QColor &color) const
{
    QColor line = color;
    line.setAlphaF(kGhostAlpha);
    const qreal radius = std::max(Utils::kCellMinRadius, area.width() * kBlockRadiusRatio);

    painter.setPen(QPen(line, kGhostPenWidth));
    painter.setBrush(Qt::NoBrush);
    painter.drawRoundedRect(
        area.adjusted(kGhostInset, kGhostInset, -kGhostInset, -kGhostInset), radius, radius);
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
    // 用 Dark（borderStrong）而不是 Mid：井的边界要和卡片底色拉开，否则一片糊。
    const QPainterPath shape
        = Utils::paintBoardFrame(painter, QRectF(rect()), palette, QPalette::Dark);

    if (!d_ptr->game)
        return;

    painter.save();
    painter.setClipPath(shape);

    const Utils::GridMetrics grid = d_ptr->metrics();
    if (grid.cell > 0) {
        for (int y = 0; y < kVisibleRows; ++y) {
            for (int x = 0; x < kColumns; ++x) {
                const int value = d_ptr->game->settledCellAt(x, y);
                if (value == 0)
                    continue;
                const auto type = static_cast<PieceType>(value - 1);
                d_ptr->paintBlock(painter, grid.cellRect(QPoint(x, y)), pieceColor(type));
            }
        }

        // 消行闪烁：盖一层随进度淡出的亮色，行本身还没被抹掉。
        const QVector<int> rows = d_ptr->game->clearingRows();
        if (!rows.isEmpty()) {
            QColor flash = palette.color(QPalette::Window);
            flash.setAlphaF(
                kFlashAlpha * (1.0F - static_cast<float>(d_ptr->game->clearingProgress())));
            painter.setPen(Qt::NoPen);
            painter.setBrush(flash);
            for (const int row : rows) {
                painter.drawRect(grid.rowRect(row));
            }
        }

        const QColor activeColor = pieceColor(d_ptr->game->activeType());
        for (const QPoint &ghost : d_ptr->game->ghostCells()) {
            d_ptr->paintGhost(painter, grid.cellRect(ghost), activeColor);
        }
        for (const QPoint &active : d_ptr->game->activeCells()) {
            d_ptr->paintBlock(painter, grid.cellRect(active), activeColor);
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
        Utils::paintIdleText(painter, QRectF(rect()), palette, d_ptr->idleText);
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
