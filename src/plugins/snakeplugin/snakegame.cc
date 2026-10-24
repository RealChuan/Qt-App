#include "snakegame.hpp"

#include <QRandomGenerator>
#include <QVector>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <utility>

namespace Snake {

namespace {

constexpr int kMinColumns = 8;
constexpr int kMinRows = 6;
constexpr int kInitialLength = 3;
constexpr int kScorePerFood = 10;
constexpr int kFoodsPerSpeedUp = 4;
constexpr int kSpeedUpMs = 10;
constexpr std::size_t kMaxPendingTurns = 2;

// 食物落点的空间偏置：bias > 0 偏棋盘中央，= 0 完全均匀，< 0 偏四周。
// 难度给基准，占用率（蛇长 / 总格数）把基准往均匀方向侵蚀——蛇越长，中腹本来
// 就被蛇身占住了，不该再往中间塞食物。
constexpr double kCenterBiasEasy = 1.6;
constexpr double kCenterBiasNormal = 0.8;
constexpr double kCenterBiasHard = 0.2;
constexpr double kBiasErosionPerOccupancy = 2.0;
constexpr double kMinFoodBias = -1.0;

struct SpeedProfile
{
    int startMs;
    int minMs;
};

// 三档的差异只在"起步有多宽松"和"最后有多快"：加速曲线本身是同一套。
auto speedProfileOf(Difficulty difficulty) -> SpeedProfile
{
    switch (difficulty) {
    case Difficulty::Easy:
        return {240, 130};
    case Difficulty::Normal:
        return {190, 100};
    case Difficulty::Hard:
        return {150, 70};
    }
    return {190, 100};
}

auto offsetOf(Direction direction) -> QPoint
{
    switch (direction) {
    case Direction::Up:
        return {0, -1};
    case Direction::Down:
        return {0, 1};
    case Direction::Left:
        return {-1, 0};
    case Direction::Right:
        return {1, 0};
    }
    return {};
}

auto isOpposite(Direction a, Direction b) -> bool
{
    return offsetOf(a) + offsetOf(b) == QPoint(0, 0);
}

auto centerBiasOf(Difficulty difficulty) -> double
{
    switch (difficulty) {
    case Difficulty::Easy:
        return kCenterBiasEasy;
    case Difficulty::Normal:
        return kCenterBiasNormal;
    case Difficulty::Hard:
        return kCenterBiasHard;
    }
    return kCenterBiasNormal;
}

// 归一化的方格距离（切比雪夫）：棋盘是方的，用"环"比用"圆"更贴合玩家直觉。
auto normalizedDistance(const QPoint &cell, int columns, int rows) -> double
{
    const double centerX = (columns - 1) / 2.0;
    const double centerY = (rows - 1) / 2.0;
    const double halfX = std::max(1.0, centerX);
    const double halfY = std::max(1.0, centerY);
    const double dx = std::abs(cell.x() - centerX) / halfX;
    const double dy = std::abs(cell.y() - centerY) / halfY;
    return std::clamp(std::max(dx, dy), 0.0, 1.0);
}

} // namespace

class SnakeGame::SnakeGamePrivate
{
public:
    SnakeGamePrivate(int columns, int rows)
        : columns(std::max(kMinColumns, columns))
        , rows(std::max(kMinRows, rows))
    {}

    void start();
    void step();
    void turn(Direction direction);
    void placeFood();
    [[nodiscard]] auto hitsBody(QPoint cell, bool isTailMovingAway) const -> bool;
    [[nodiscard]] auto stepIntervalMs() const -> int;

    int columns = 0;
    int rows = 0;
    std::deque<QPoint> body;
    std::deque<Direction> pendingTurns;
    QPoint food;
    Direction heading = Direction::Right;
    GameState state = GameState::Idle;
    Difficulty difficulty = Difficulty::Normal;
    int score = 0;
    int foodsEaten = 0;
};

void SnakeGame::SnakeGamePrivate::start()
{
    body.clear();
    pendingTurns.clear();

    // 头在右、身在左，开局直接朝右走，不会第一 tick 就撞墙。
    const int row = rows / 2;
    const int headX = std::max(kInitialLength - 1, columns / 2);
    for (int i = 0; i < kInitialLength; ++i)
        body.push_back(QPoint(headX - i, row));

    heading = Direction::Right;
    score = 0;
    foodsEaten = 0;
    state = GameState::Running;
    placeFood();
}

void SnakeGame::SnakeGamePrivate::turn(Direction direction)
{
    if (state != GameState::Running)
        return;

    // 拿队列里最后一个方向判断，而不是当前朝向：一个 tick 内连按两次
    // "上、左"时，第二次必须相对"上"判断，否则会漏掉 180° 拦截。
    const Direction last = pendingTurns.empty() ? heading : pendingTurns.back();
    if (direction == last || isOpposite(last, direction))
        return;
    if (pendingTurns.size() >= kMaxPendingTurns)
        return;

    pendingTurns.push_back(direction);
}

void SnakeGame::SnakeGamePrivate::step()
{
    if (state != GameState::Running)
        return;

    if (!pendingTurns.empty()) {
        heading = pendingTurns.front();
        pendingTurns.pop_front();
    }

    const QPoint head = body.front() + offsetOf(heading);
    const bool isOutside = head.x() < 0 || head.y() < 0 || head.x() >= columns || head.y() >= rows;
    if (isOutside) {
        state = GameState::Over;
        return;
    }

    const bool isGrowing = head == food;
    if (hitsBody(head, !isGrowing)) {
        state = GameState::Over;
        return;
    }

    if (!isGrowing)
        body.pop_back();
    body.push_front(head);

    if (!isGrowing)
        return;

    score += kScorePerFood;
    ++foodsEaten;
    placeFood(); // 棋盘被塞满时在这里把状态改成 Over
}

void SnakeGame::SnakeGamePrivate::placeFood()
{
    QVector<QPoint> freeCells;
    freeCells.reserve(columns * rows);
    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x < columns; ++x) {
            const QPoint cell(x, y);
            if (std::find(body.cbegin(), body.cend(), cell) == body.cend())
                freeCells.append(cell);
        }
    }

    if (freeCells.isEmpty()) {
        // 蛇填满棋盘，没有空位可放食物：这一局没有下一步了。
        state = GameState::Over;
        return;
    }

    const double occupancy = static_cast<double>(body.size()) / static_cast<double>(columns * rows);
    const double bias
        = std::max(kMinFoodBias, centerBiasOf(difficulty) - kBiasErosionPerOccupancy * occupancy);

    // 权重取 exp(-bias * t)：不会在任何格子上归零，角落始终留一点机会。
    double totalWeight = 0.0;
    for (const QPoint &cell : std::as_const(freeCells))
        totalWeight += std::exp(-bias * normalizedDistance(cell, columns, rows));

    double ticket = QRandomGenerator::global()->generateDouble() * totalWeight;
    for (const QPoint &cell : std::as_const(freeCells)) {
        ticket -= std::exp(-bias * normalizedDistance(cell, columns, rows));
        if (ticket <= 0.0) {
            food = cell;
            return;
        }
    }

    // 浮点累加的兜底：票数落到末尾之外时取最后一个空格。
    food = freeCells.last();
}

auto SnakeGame::SnakeGamePrivate::hitsBody(QPoint cell, bool isTailMovingAway) const -> bool
{
    // 不吃食物时尾巴会让出格子，所以撞尾判断要排除最后一节，
    // 否则"头追着尾巴走"会被误判成自杀。
    const auto end = isTailMovingAway ? body.cend() - 1 : body.cend();
    return std::find(body.cbegin(), end, cell) != end;
}

auto SnakeGame::SnakeGamePrivate::stepIntervalMs() const -> int
{
    const SpeedProfile profile = speedProfileOf(difficulty);
    const int level = foodsEaten / kFoodsPerSpeedUp;
    return std::max(profile.minMs, profile.startMs - level * kSpeedUpMs);
}

SnakeGame::SnakeGame(int columns, int rows)
    : d_ptr(std::make_unique<SnakeGamePrivate>(columns, rows))
{}

SnakeGame::~SnakeGame() = default;

void SnakeGame::setDifficulty(Difficulty difficulty)
{
    d_ptr->difficulty = difficulty;
}

void SnakeGame::start()
{
    d_ptr->start();
}

void SnakeGame::togglePause()
{
    if (d_ptr->state == GameState::Running)
        d_ptr->state = GameState::Paused;
    else if (d_ptr->state == GameState::Paused)
        d_ptr->state = GameState::Running;
}

void SnakeGame::step()
{
    d_ptr->step();
}

void SnakeGame::turn(Direction direction)
{
    d_ptr->turn(direction);
}

auto SnakeGame::difficulty() const -> Difficulty
{
    return d_ptr->difficulty;
}

auto SnakeGame::state() const -> GameState
{
    return d_ptr->state;
}

auto SnakeGame::score() const -> int
{
    return d_ptr->score;
}

auto SnakeGame::foodsEaten() const -> int
{
    return d_ptr->foodsEaten;
}

auto SnakeGame::columns() const -> int
{
    return d_ptr->columns;
}

auto SnakeGame::rows() const -> int
{
    return d_ptr->rows;
}

auto SnakeGame::body() const -> const std::deque<QPoint> &
{
    return d_ptr->body;
}

auto SnakeGame::food() const -> QPoint
{
    return d_ptr->food;
}

auto SnakeGame::heading() const -> Direction
{
    return d_ptr->heading;
}

auto SnakeGame::stepIntervalMs() const -> int
{
    return d_ptr->stepIntervalMs();
}

} // namespace Snake
