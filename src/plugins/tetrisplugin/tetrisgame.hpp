#pragma once

#include <QPoint>
#include <QVector>

#include <array>
#include <memory>
#include <optional>

namespace Tetris {

enum class PieceType { I = 0, J, L, O, S, T, Z };

enum class GameState { Idle, Running, Paused, Over };

inline constexpr int kColumns = 10;
inline constexpr int kVisibleRows = 20;
inline constexpr int kPieceCount = 7;

// 出生框内的初始形状（rotation 0）。预览控件也用它，所以放在头文件。
struct PieceShape
{
    int boxSize = 0;
    std::array<QPoint, 4> cells{};
};

[[nodiscard]] auto shapeOf(PieceType type) -> PieceShape;

// 纯逻辑：不继承 QObject、不发信号、不认识控件。
// 旋转、踢墙、7-bag、锁定延迟、消行计分都在这一层，视图只读格子。
class TetrisGame
{
public:
    // seed 为 0 时取全局随机源；测试传固定值即可复现整局。
    explicit TetrisGame(quint32 seed = 0);
    ~TetrisGame();

    TetrisGame(const TetrisGame &) = delete;
    auto operator=(const TetrisGame &) -> TetrisGame & = delete;

    void start();
    void togglePause();
    // 推进 dtMs：重力、锁定延迟、消行动画都在这里，输入层只管调用。
    void advance(int dtMs);

    // 返回 false 表示被挡住，调用方据此决定是否重置连发状态。
    bool moveBy(int dx);
    bool rotate(bool isClockwise);
    void softDrop();
    void hardDrop();
    void hold();

    [[nodiscard]] auto state() const -> GameState;
    // 0 表示空，否则是 PieceType + 1。
    [[nodiscard]] auto settledCellAt(int x, int y) const -> int;
    [[nodiscard]] auto activeCells() const -> QVector<QPoint>;
    [[nodiscard]] auto ghostCells() const -> QVector<QPoint>;
    [[nodiscard]] auto activeType() const -> PieceType;
    [[nodiscard]] auto nextQueue(int count) const -> QVector<PieceType>;
    [[nodiscard]] auto heldType() const -> std::optional<PieceType>;
    [[nodiscard]] auto canHold() const -> bool;
    // 正在闪烁的行；空表示没有在消行。
    [[nodiscard]] auto clearingRows() const -> QVector<int>;
    // 0..1，供视图做闪烁渐变。
    [[nodiscard]] auto clearingProgress() const -> double;
    [[nodiscard]] auto score() const -> int;
    [[nodiscard]] auto lines() const -> int;
    [[nodiscard]] auto level() const -> int;

private:
    class TetrisGamePrivate;
    std::unique_ptr<TetrisGamePrivate> d_ptr;
};

} // namespace Tetris
