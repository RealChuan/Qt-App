#include "snakeboard.hpp"

#include <utils/uihelpers.hpp>

#include <QEvent>
#include <QGuiApplication>
#include <QKeyEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPalette>

#include <cmath>
#include <cstddef>
#include <deque>

namespace Snake {

namespace {

constexpr int kInnerPadding = 10;
// 格子小于这个尺寸时网格线只会糊成一片，干脆不画。
constexpr int kMinCellForGrid = 14;
constexpr qreal kCellInset = 1.5;
constexpr qreal kHeadInset = 0.5;
constexpr qreal kCellRadiusRatio = 0.22;
constexpr qreal kFoodRadiusRatio = 0.30;
// QColor::setAlphaF 是 float 接口，透明度常量直接用 float。
constexpr float kTailAlpha = 0.45F;
constexpr float kGridAlpha = 0.16F;

auto complementOf(const QColor &color) -> QColor
{
    float hue = color.hsvHueF();
    if (hue < 0.0F)
        hue = 0.58F; // 强调色是灰色时的冷蓝
    return QColor::fromHsvF(std::fmod(hue + 0.5F, 1.0F), 0.70F, 0.95F);
}

} // namespace

class SnakeBoard::SnakeBoardPrivate
{
public:
    explicit SnakeBoardPrivate(SnakeBoard *q)
        : q_ptr(q)
    {}

    [[nodiscard]] auto metrics() const -> Utils::GridMetrics;

    void paintGrid(QPainter &painter, const Utils::GridMetrics &grid, const QPalette &palette) const;
    void paintFood(QPainter &painter, const Utils::GridMetrics &grid, const QPalette &palette) const;
    void paintSnake(QPainter &painter, const Utils::GridMetrics &grid, const QPalette &palette) const;

    SnakeBoard *q_ptr = nullptr;
    const SnakeGame *game = nullptr;
    QString idleText;
    QString overTitle;
    QString pausedTitle;
    QString playAgainHint;
    QString resumeHint;
};

auto SnakeBoard::SnakeBoardPrivate::metrics() const -> Utils::GridMetrics
{
    if (!game)
        return {};

    return Utils::fitGrid(q_ptr->size(), game->columns(), game->rows(), kInnerPadding);
}

void SnakeBoard::SnakeBoardPrivate::paintGrid(
    QPainter &painter, const Utils::GridMetrics &grid, const QPalette &palette) const
{
    if (grid.cell < kMinCellForGrid)
        return;

    QColor line = palette.color(QPalette::PlaceholderText);
    line.setAlphaF(kGridAlpha);
    // 落在 +0.5 上：1px 线压在整像素边界会被抗锯齿摊成两根灰线。
    painter.setPen(QPen(line, 1.0));

    for (int x = 1; x < grid.columns; ++x) {
        const qreal px = grid.rect.left() + x * grid.cell + 0.5;
        painter.drawLine(QPointF(px, grid.rect.top()), QPointF(px, grid.rect.bottom()));
    }
    for (int y = 1; y < grid.rows; ++y) {
        const qreal py = grid.rect.top() + y * grid.cell + 0.5;
        painter.drawLine(QPointF(grid.rect.left(), py), QPointF(grid.rect.right(), py));
    }
}

void SnakeBoard::SnakeBoardPrivate::paintFood(
    QPainter &painter, const Utils::GridMetrics &grid, const QPalette &palette) const
{
    const QRectF target = grid.cellRect(game->food());
    const qreal radius = grid.cell * kFoodRadiusRatio;

    painter.setPen(Qt::NoPen);
    painter.setBrush(complementOf(palette.color(QPalette::Highlight)));
    painter.drawEllipse(target.center(), radius, radius);
}

void SnakeBoard::SnakeBoardPrivate::paintSnake(
    QPainter &painter, const Utils::GridMetrics &grid, const QPalette &palette) const
{
    const std::deque<QPoint> &body = game->body();
    const int count = static_cast<int>(body.size());
    if (count == 0)
        return;

    const QColor base = palette.color(QPalette::Highlight);

    for (int i = 0; i < count; ++i) {
        // 从头到尾淡出，尾端一眼可辨，不用再画眼睛之类的装饰。
        const float t = count > 1 ? static_cast<float>(i) / static_cast<float>(count - 1) : 0.0F;
        QColor color = base;
        color.setAlphaF(1.0F - (1.0F - kTailAlpha) * t);

        // 头比身体多出半像素：两块挨着时能看出哪端是头。
        const qreal inset = i == 0 ? kHeadInset : kCellInset;
        Utils::paintCell(
            painter,
            grid.cellRect(body.at(static_cast<std::size_t>(i))),
            color,
            {inset, kCellRadiusRatio});
    }
}

SnakeBoard::SnakeBoard(QWidget *parent)
    : QWidget(parent)
    , d_ptr(std::make_unique<SnakeBoardPrivate>(this))
{
    // 键盘是唯一的输入来源，没有焦点就等于玩不了。
    setFocusPolicy(Qt::StrongFocus);
    setMinimumSize(360, 260);
    retranslateUi();
}

SnakeBoard::~SnakeBoard() = default;

void SnakeBoard::setGame(const SnakeGame *game)
{
    d_ptr->game = game;
    update();
}

void SnakeBoard::retranslateUi()
{
    d_ptr->idleText = tr("Press Space to start");
    d_ptr->overTitle = tr("Game Over");
    d_ptr->pausedTitle = tr("Paused");
    d_ptr->playAgainHint = tr("Space to play again");
    d_ptr->resumeHint = tr("Space to resume");
    update();
}

void SnakeBoard::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    const QPalette palette = QGuiApplication::palette();
    const QPainterPath shape = Utils::paintBoardFrame(painter, QRectF(rect()), palette);

    if (!d_ptr->game)
        return;

    painter.save();
    painter.setClipPath(shape);

    const Utils::GridMetrics grid = d_ptr->metrics();
    if (grid.cell > 0) {
        d_ptr->paintGrid(painter, grid, palette);
        d_ptr->paintFood(painter, grid, palette);
        d_ptr->paintSnake(painter, grid, palette);
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

void SnakeBoard::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Up:
    case Qt::Key_W:
        emit turnRequested(Direction::Up);
        return;
    case Qt::Key_Down:
    case Qt::Key_S:
        emit turnRequested(Direction::Down);
        return;
    case Qt::Key_Left:
    case Qt::Key_A:
        emit turnRequested(Direction::Left);
        return;
    case Qt::Key_Right:
    case Qt::Key_D:
        emit turnRequested(Direction::Right);
        return;
    case Qt::Key_Space:
    case Qt::Key_Return:
    case Qt::Key_Enter:
        // 长按 Space 会在暂停与继续之间来回跳，这里只认第一次。
        if (!event->isAutoRepeat())
            emit primaryActionRequested();
        return;
    case Qt::Key_P:
    case Qt::Key_Escape:
        if (!event->isAutoRepeat())
            emit pauseToggled();
        return;
    default:
        break;
    }

    QWidget::keyPressEvent(event);
}

void SnakeBoard::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
        retranslateUi();
    QWidget::changeEvent(event);
}

} // namespace Snake
