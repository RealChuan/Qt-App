#include "tetrisgame.hpp"

#include <QRandomGenerator>

#include <algorithm>
#include <array>
#include <cstddef>

namespace Tetris {

namespace {

constexpr int kLockDelayMs = 500;
constexpr int kMaxLockResets = 15;
constexpr int kClearDurationMs = 150;
constexpr int kBaseGravityMs = 800;
constexpr int kGravityStepMs = 60;
constexpr int kMinGravityMs = 60;
constexpr int kLinesPerLevel = 10;
constexpr int kMaxLevel = 20;
constexpr int kQueueLookahead = 5;
constexpr int kSpawnX = 3;

// 消 1/2/3/4 行的基础分，再乘当前等级。
constexpr std::array<int, 5> kLineScores{0, 100, 300, 500, 800};

struct Kick
{
    int dx;
    int dy;
};

// SRS 踢墙表。原表以 y 轴向上，这里全部取反以适配屏幕坐标（y 向下）。
// 索引 = 起始朝向 * 2 + (顺时针 ? 0 : 1)。
using KickSet = std::array<Kick, 5>;

constexpr std::array<KickSet, 8> kJlstzKicks{{
    {{{0, 0}, {-1, 0}, {-1, -1}, {0, 2}, {-1, 2}}},  // 0 -> 1
    {{{0, 0}, {1, 0}, {1, -1}, {0, 2}, {1, 2}}},     // 0 -> 3
    {{{0, 0}, {1, 0}, {1, 1}, {0, -2}, {1, -2}}},    // 1 -> 2
    {{{0, 0}, {1, 0}, {1, 1}, {0, -2}, {1, -2}}},    // 1 -> 0
    {{{0, 0}, {1, 0}, {1, -1}, {0, 2}, {1, 2}}},     // 2 -> 3
    {{{0, 0}, {-1, 0}, {-1, -1}, {0, 2}, {-1, 2}}},  // 2 -> 1
    {{{0, 0}, {-1, 0}, {-1, 1}, {0, -2}, {-1, -2}}}, // 3 -> 0
    {{{0, 0}, {-1, 0}, {-1, 1}, {0, -2}, {-1, -2}}}, // 3 -> 2
}};

constexpr std::array<KickSet, 8> kIKicks{{
    {{{0, 0}, {-2, 0}, {1, 0}, {-2, 1}, {1, -2}}}, // 0 -> 1
    {{{0, 0}, {-1, 0}, {2, 0}, {-1, -2}, {2, 1}}}, // 0 -> 3
    {{{0, 0}, {-1, 0}, {2, 0}, {-1, -2}, {2, 1}}}, // 1 -> 2
    {{{0, 0}, {2, 0}, {-1, 0}, {2, -1}, {-1, 2}}}, // 1 -> 0
    {{{0, 0}, {2, 0}, {-1, 0}, {2, -1}, {-1, 2}}}, // 2 -> 3
    {{{0, 0}, {1, 0}, {-2, 0}, {1, 2}, {-2, -1}}}, // 2 -> 1
    {{{0, 0}, {1, 0}, {-2, 0}, {1, 2}, {-2, -1}}}, // 3 -> 0
    {{{0, 0}, {-2, 0}, {1, 0}, {-2, 1}, {1, -2}}}, // 3 -> 2
}};

auto kicksFor(PieceType type, int fromRotation, bool isClockwise) -> const KickSet &
{
    const auto index = static_cast<std::size_t>(fromRotation * 2 + (isClockwise ? 0 : 1));
    return type == PieceType::I ? kIKicks.at(index) : kJlstzKicks.at(index);
}

constexpr auto indexOf(int x, int y) -> int
{
    return y * kColumns + x;
}

} // namespace

auto shapeOf(PieceType type) -> PieceShape
{
    switch (type) {
    case PieceType::I:
        return {4, {QPoint(0, 1), QPoint(1, 1), QPoint(2, 1), QPoint(3, 1)}};
    case PieceType::J:
        return {3, {QPoint(0, 0), QPoint(0, 1), QPoint(1, 1), QPoint(2, 1)}};
    case PieceType::L:
        return {3, {QPoint(2, 0), QPoint(0, 1), QPoint(1, 1), QPoint(2, 1)}};
    case PieceType::O:
        return {4, {QPoint(1, 0), QPoint(2, 0), QPoint(1, 1), QPoint(2, 1)}};
    case PieceType::S:
        return {3, {QPoint(1, 0), QPoint(2, 0), QPoint(0, 1), QPoint(1, 1)}};
    case PieceType::T:
        return {3, {QPoint(1, 0), QPoint(0, 1), QPoint(1, 1), QPoint(2, 1)}};
    case PieceType::Z:
        return {3, {QPoint(0, 0), QPoint(1, 0), QPoint(1, 1), QPoint(2, 1)}};
    }
    return {};
}

class TetrisGame::TetrisGamePrivate
{
public:
    explicit TetrisGamePrivate(quint32 seed)
        : rng(seed == 0 ? QRandomGenerator::global()->generate() : seed)
    {}

    void start();
    void advance(int dtMs);
    bool moveBy(int dx);
    bool rotate(bool isClockwise);
    void softDrop();
    void hardDrop();
    void hold();

    [[nodiscard]] auto isPlayable() const -> bool;
    [[nodiscard]] auto cellsOf(PieceType type, int rotation, QPoint origin) const
        -> std::array<QPoint, 4>;
    [[nodiscard]] auto collides(PieceType type, int rotation, QPoint origin) const -> bool;
    [[nodiscard]] auto canFall() const -> bool;
    [[nodiscard]] auto ghostOrigin() const -> QPoint;
    [[nodiscard]] auto gravityIntervalMs() const -> int;

    void resetLockDelay();
    void lockPiece();
    void applyClear();
    void removeRows(const QVector<int> &rows);
    void spawnFromQueue();
    void refillBag();
    [[nodiscard]] auto isRowFull(int y) const -> bool;

    QRandomGenerator rng;
    std::array<quint8, kColumns * kVisibleRows> board{};
    QVector<PieceType> queue;

    PieceType active = PieceType::I;
    int rotation = 0;
    QPoint origin{kSpawnX, 0};
    std::optional<PieceType> held;
    bool hasHeldThisPiece = false;

    GameState state = GameState::Idle;
    int score = 0;
    int lines = 0;
    int level = 1;

    int gravityMs = 0;
    int lockMs = 0;
    int lockResets = 0;

    QVector<int> clearingRows;
    int clearMs = 0;
};

void TetrisGame::TetrisGamePrivate::start()
{
    board.fill(0);
    queue.clear();
    held.reset();
    hasHeldThisPiece = false;

    score = 0;
    lines = 0;
    level = 1;
    gravityMs = 0;
    lockMs = 0;
    lockResets = 0;
    clearingRows.clear();
    clearMs = 0;

    state = GameState::Running;
    spawnFromQueue();
}

void TetrisGame::TetrisGamePrivate::advance(int dtMs)
{
    if (state != GameState::Running)
        return;

    if (!clearingRows.isEmpty()) {
        clearMs += dtMs;
        if (clearMs >= kClearDurationMs)
            applyClear();
        return;
    }

    gravityMs += dtMs;
    if (gravityMs >= gravityIntervalMs()) {
        gravityMs = 0;
        if (!collides(active, rotation, origin + QPoint(0, 1)))
            origin.ry() += 1;
    }

    // 落地后的宽限：站着不动才累计，一旦能继续下落就清掉。
    if (canFall()) {
        lockMs = 0;
        return;
    }

    lockMs += dtMs;
    if (lockMs >= kLockDelayMs)
        lockPiece();
}

auto TetrisGame::TetrisGamePrivate::isPlayable() const -> bool
{
    return state == GameState::Running && clearingRows.isEmpty();
}

bool TetrisGame::TetrisGamePrivate::moveBy(int dx)
{
    if (!isPlayable())
        return false;

    const QPoint candidate = origin + QPoint(dx, 0);
    if (collides(active, rotation, candidate))
        return false;

    origin = candidate;
    resetLockDelay();
    return true;
}

bool TetrisGame::TetrisGamePrivate::rotate(bool isClockwise)
{
    // O 转不出新形状，转它只会白白刷新锁定计时。
    if (!isPlayable() || active == PieceType::O)
        return false;

    const int target = (rotation + (isClockwise ? 1 : 3)) % 4;
    for (const Kick &kick : kicksFor(active, rotation, isClockwise)) {
        const QPoint candidate = origin + QPoint(kick.dx, kick.dy);
        if (collides(active, target, candidate))
            continue;

        rotation = target;
        origin = candidate;
        resetLockDelay();
        return true;
    }
    return false;
}

void TetrisGame::TetrisGamePrivate::softDrop()
{
    if (!isPlayable() || collides(active, rotation, origin + QPoint(0, 1)))
        return;

    origin.ry() += 1;
    ++score;
    gravityMs = 0;
}

void TetrisGame::TetrisGamePrivate::hardDrop()
{
    if (!isPlayable())
        return;

    while (!collides(active, rotation, origin + QPoint(0, 1))) {
        origin.ry() += 1;
        score += 2;
    }
    lockPiece();
}

void TetrisGame::TetrisGamePrivate::hold()
{
    if (!isPlayable() || hasHeldThisPiece)
        return;

    const PieceType previous = active;
    if (held.has_value()) {
        active = *held;
        held = previous;
        rotation = 0;
        origin = QPoint(kSpawnX, 0);
        gravityMs = 0;
        lockMs = 0;
        lockResets = 0;
        if (collides(active, rotation, origin))
            state = GameState::Over;
    } else {
        held = previous;
        spawnFromQueue();
    }

    hasHeldThisPiece = true;
}

auto TetrisGame::TetrisGamePrivate::cellsOf(PieceType type, int rotation, QPoint origin) const
    -> std::array<QPoint, 4>
{
    PieceShape shape = shapeOf(type);
    std::array<QPoint, 4> cells = shape.cells;

    // O 的四个格子关于中心对称，转与不转一样，所以直接跳过旋转。
    const int turns = type == PieceType::O ? 0 : rotation;
    for (int i = 0; i < turns; ++i) {
        for (QPoint &cell : cells)
            cell = QPoint(shape.boxSize - 1 - cell.y(), cell.x());
    }

    for (QPoint &cell : cells)
        cell += origin;
    return cells;
}

auto TetrisGame::TetrisGamePrivate::collides(PieceType type, int rotation, QPoint origin) const
    -> bool
{
    for (const QPoint &cell : cellsOf(type, rotation, origin)) {
        if (cell.x() < 0 || cell.x() >= kColumns || cell.y() >= kVisibleRows)
            return true;
        // y < 0 是出生框上方的空气，允许踢墙时短暂越界。
        if (cell.y() >= 0 && board.at(static_cast<std::size_t>(indexOf(cell.x(), cell.y()))) != 0)
            return true;
    }
    return false;
}

auto TetrisGame::TetrisGamePrivate::canFall() const -> bool
{
    return !collides(active, rotation, origin + QPoint(0, 1));
}

auto TetrisGame::TetrisGamePrivate::ghostOrigin() const -> QPoint
{
    QPoint result = origin;
    while (!collides(active, rotation, result + QPoint(0, 1)))
        result.ry() += 1;
    return result;
}

auto TetrisGame::TetrisGamePrivate::gravityIntervalMs() const -> int
{
    return std::max(kMinGravityMs, kBaseGravityMs - (level - 1) * kGravityStepMs);
}

void TetrisGame::TetrisGamePrivate::resetLockDelay()
{
    if (canFall() || lockResets >= kMaxLockResets)
        return;

    lockMs = 0;
    ++lockResets;
}

void TetrisGame::TetrisGamePrivate::lockPiece()
{
    for (const QPoint &cell : cellsOf(active, rotation, origin)) {
        if (cell.y() >= 0)
            board.at(static_cast<std::size_t>(indexOf(cell.x(), cell.y()))) = static_cast<quint8>(
                static_cast<int>(active) + 1);
    }

    QVector<int> fullRows;
    for (int y = 0; y < kVisibleRows; ++y) {
        if (isRowFull(y))
            fullRows.append(y);
    }

    lockMs = 0;
    lockResets = 0;

    if (fullRows.isEmpty()) {
        spawnFromQueue();
        return;
    }

    // 先停一拍让视图闪烁，真正的消除在 applyClear() 里。
    clearingRows = fullRows;
    clearMs = 0;
}

void TetrisGame::TetrisGamePrivate::applyClear()
{
    const int count = static_cast<int>(clearingRows.size());
    const auto index = static_cast<std::size_t>(std::min(count, 4));
    score += kLineScores.at(index) * level;

    lines += count;
    level = std::min(kMaxLevel, 1 + lines / kLinesPerLevel);

    removeRows(clearingRows);
    clearingRows.clear();
    clearMs = 0;

    spawnFromQueue();
}

void TetrisGame::TetrisGamePrivate::removeRows(const QVector<int> &rows)
{
    int writeY = kVisibleRows - 1;
    for (int readY = kVisibleRows - 1; readY >= 0; --readY) {
        if (rows.contains(readY))
            continue;

        if (writeY != readY) {
            for (int x = 0; x < kColumns; ++x)
                board.at(static_cast<std::size_t>(indexOf(x, writeY))) = board.at(
                    static_cast<std::size_t>(indexOf(x, readY)));
        }
        --writeY;
    }

    // 顶部空出来的行必须显式抹掉，否则会被上一次的内容沿用。
    for (int y = writeY; y >= 0; --y) {
        for (int x = 0; x < kColumns; ++x)
            board.at(static_cast<std::size_t>(indexOf(x, y))) = 0;
    }
}

void TetrisGame::TetrisGamePrivate::spawnFromQueue()
{
    while (queue.size() < kQueueLookahead)
        refillBag();

    active = queue.takeFirst();
    rotation = 0;
    origin = QPoint(kSpawnX, 0);
    gravityMs = 0;
    lockMs = 0;
    lockResets = 0;
    // 新的一块又可以用一次暂存；hold() 在生成之后再把它置回 true。
    hasHeldThisPiece = false;

    if (collides(active, rotation, origin))
        state = GameState::Over;
}

void TetrisGame::TetrisGamePrivate::refillBag()
{
    std::array<PieceType, kPieceCount>
        bag{PieceType::I,
            PieceType::J,
            PieceType::L,
            PieceType::O,
            PieceType::S,
            PieceType::T,
            PieceType::Z};

    for (int i = kPieceCount - 1; i > 0; --i) {
        const int j = rng.bounded(i + 1);
        std::swap(bag.at(static_cast<std::size_t>(i)), bag.at(static_cast<std::size_t>(j)));
    }

    for (const PieceType type : bag)
        queue.append(type);
}

auto TetrisGame::TetrisGamePrivate::isRowFull(int y) const -> bool
{
    for (int x = 0; x < kColumns; ++x) {
        if (board.at(static_cast<std::size_t>(indexOf(x, y))) == 0)
            return false;
    }
    return true;
}

TetrisGame::TetrisGame(quint32 seed)
    : d_ptr(std::make_unique<TetrisGamePrivate>(seed))
{}

TetrisGame::~TetrisGame() = default;

void TetrisGame::start()
{
    d_ptr->start();
}

void TetrisGame::togglePause()
{
    if (d_ptr->state == GameState::Running)
        d_ptr->state = GameState::Paused;
    else if (d_ptr->state == GameState::Paused)
        d_ptr->state = GameState::Running;
}

void TetrisGame::advance(int dtMs)
{
    d_ptr->advance(dtMs);
}

bool TetrisGame::moveBy(int dx)
{
    return d_ptr->moveBy(dx);
}

bool TetrisGame::rotate(bool isClockwise)
{
    return d_ptr->rotate(isClockwise);
}

void TetrisGame::softDrop()
{
    d_ptr->softDrop();
}

void TetrisGame::hardDrop()
{
    d_ptr->hardDrop();
}

void TetrisGame::hold()
{
    d_ptr->hold();
}

auto TetrisGame::state() const -> GameState
{
    return d_ptr->state;
}

auto TetrisGame::settledCellAt(int x, int y) const -> int
{
    if (x < 0 || x >= kColumns || y < 0 || y >= kVisibleRows)
        return 0;
    return d_ptr->board.at(static_cast<std::size_t>(indexOf(x, y)));
}

auto TetrisGame::activeCells() const -> QVector<QPoint>
{
    QVector<QPoint> cells;
    for (const QPoint &cell : d_ptr->cellsOf(d_ptr->active, d_ptr->rotation, d_ptr->origin)) {
        if (cell.y() >= 0)
            cells.append(cell);
    }
    return cells;
}

auto TetrisGame::ghostCells() const -> QVector<QPoint>
{
    QVector<QPoint> cells;
    const QPoint ghost = d_ptr->ghostOrigin();
    for (const QPoint &cell : d_ptr->cellsOf(d_ptr->active, d_ptr->rotation, ghost)) {
        if (cell.y() >= 0)
            cells.append(cell);
    }
    return cells;
}

auto TetrisGame::activeType() const -> PieceType
{
    return d_ptr->active;
}

auto TetrisGame::nextQueue(int count) const -> QVector<PieceType>
{
    const int take = std::clamp(count, 0, static_cast<int>(d_ptr->queue.size()));
    return d_ptr->queue.mid(0, take);
}

auto TetrisGame::heldType() const -> std::optional<PieceType>
{
    return d_ptr->held;
}

auto TetrisGame::canHold() const -> bool
{
    return d_ptr->isPlayable() && !d_ptr->hasHeldThisPiece;
}

auto TetrisGame::clearingRows() const -> QVector<int>
{
    return d_ptr->clearingRows;
}

auto TetrisGame::clearingProgress() const -> double
{
    if (d_ptr->clearingRows.isEmpty())
        return 0.0;
    return std::clamp(static_cast<double>(d_ptr->clearMs) / kClearDurationMs, 0.0, 1.0);
}

auto TetrisGame::score() const -> int
{
    return d_ptr->score;
}

auto TetrisGame::lines() const -> int
{
    return d_ptr->lines;
}

auto TetrisGame::level() const -> int
{
    return d_ptr->level;
}

} // namespace Tetris
