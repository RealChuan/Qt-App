#pragma once

#include <QPoint>

#include <deque>
#include <memory>

namespace Snake {

enum class Direction { Up, Down, Left, Right };

enum class GameState { Idle, Running, Paused, Over };

// 难度只决定起步速度与加速下限，起步/下限的对应关系见 snakegame.cc。
enum class Difficulty { Easy, Normal, Hard };

// 纯逻辑：不继承 QObject、不发信号、不认识控件，棋盘是逻辑坐标。
// 数据与算法收进 SnakeGamePrivate，头文件只留接口；因此这一层可以脱离 GUI
// 直接单测，渲染层自己把格子换算成像素。
class SnakeGame
{
public:
    SnakeGame(int columns, int rows);
    ~SnakeGame();

    SnakeGame(const SnakeGame &) = delete;
    auto operator=(const SnakeGame &) -> SnakeGame & = delete;

    // 只在开局前设置：页面在跑动和暂停期间会把难度选择禁掉。
    void setDifficulty(Difficulty difficulty);
    [[nodiscard]] auto difficulty() const -> Difficulty;

    // 重置到开局状态并开始跑。
    void start();
    void togglePause();
    // 前进一格。撞墙、撞到自己、棋盘塞满都会把状态置为 Over。
    void step();

    // 转向进队列：一 tick 内连按不会互相覆盖，也不允许 180° 掉头。
    void turn(Direction direction);

    [[nodiscard]] auto state() const -> GameState;
    [[nodiscard]] auto score() const -> int;
    [[nodiscard]] auto foodsEaten() const -> int;
    [[nodiscard]] auto columns() const -> int;
    [[nodiscard]] auto rows() const -> int;
    // 蛇身：front() 是头，back() 是尾。
    [[nodiscard]] auto body() const -> const std::deque<QPoint> &;
    [[nodiscard]] auto food() const -> QPoint;
    [[nodiscard]] auto heading() const -> Direction;
    // 当前每 tick 的毫秒数：吃食物会变快，到该难度的下限为止。
    [[nodiscard]] auto stepIntervalMs() const -> int;

private:
    class SnakeGamePrivate;
    std::unique_ptr<SnakeGamePrivate> d_ptr;
};

} // namespace Snake
