#include "snakegame.hpp"

#include <QPoint>
#include <QTest>

#include <algorithm>
#include <cmath>

using namespace Snake;

namespace {

constexpr int kColumns = 28;
constexpr int kRows = 20;

// 与 snakegame.cc 的落点权重同一个度量：归一化方格距离（切比雪夫）。
auto normalizedDistance(const QPoint &cell) -> double
{
    const double centerX = (kColumns - 1) / 2.0;
    const double centerY = (kRows - 1) / 2.0;
    const double dx = std::abs(cell.x() - centerX) / std::max(1.0, centerX);
    const double dy = std::abs(cell.y() - centerY) / std::max(1.0, centerY);
    return std::clamp(std::max(dx, dy), 0.0, 1.0);
}

auto meanFoodDistance(Difficulty difficulty, int samples) -> double
{
    SnakeGame game(kColumns, kRows);
    game.setDifficulty(difficulty);

    double sum = 0.0;
    for (int i = 0; i < samples; ++i) {
        game.start();
        sum += normalizedDistance(game.food());
    }
    return sum / samples;
}

} // namespace

class TestSnakeGame : public QObject
{
    Q_OBJECT

private slots:
    void startsWithThreeCells();
    void stepsIntervalByDifficulty_data();
    void stepsIntervalByDifficulty();
    void turnsIntoWall();
    void rejectsReverse();
    void queuesTurnsInOrder();
    void rejectsOppositeOfQueuedTurn();
    void pauseFreezesTheRound();
    void foodNeverLandsOnTheSnake();
    void foodFavoursTheCentreOnEasy();
};

void TestSnakeGame::startsWithThreeCells()
{
    SnakeGame game(kColumns, kRows);
    QCOMPARE(game.state(), GameState::Idle);

    game.start();
    QCOMPARE(game.state(), GameState::Running);
    QCOMPARE(int(game.body().size()), 3);
    QCOMPARE(game.body().front(), QPoint(14, 10));
    QCOMPARE(game.heading(), Direction::Right);
    QCOMPARE(game.score(), 0);
    QCOMPARE(game.foodsEaten(), 0);
    QCOMPARE(game.columns(), kColumns);
    QCOMPARE(game.rows(), kRows);
}

void TestSnakeGame::stepsIntervalByDifficulty_data()
{
    QTest::addColumn<int>("difficulty");
    QTest::addColumn<int>("intervalMs");

    QTest::newRow("easy") << static_cast<int>(Difficulty::Easy) << 240;
    QTest::newRow("normal") << static_cast<int>(Difficulty::Normal) << 190;
    QTest::newRow("hard") << static_cast<int>(Difficulty::Hard) << 150;
}

void TestSnakeGame::stepsIntervalByDifficulty()
{
    QFETCH(int, difficulty);
    QFETCH(int, intervalMs);

    SnakeGame game(kColumns, kRows);
    QCOMPARE(game.difficulty(), Difficulty::Normal); // 默认标准
    game.setDifficulty(static_cast<Difficulty>(difficulty));
    QCOMPARE(game.stepIntervalMs(), intervalMs);

    // 加速只在吃到食物后发生；开局无论如何都是该档的起步值。
    game.start();
    QCOMPARE(game.stepIntervalMs(), intervalMs);
}

void TestSnakeGame::turnsIntoWall()
{
    SnakeGame game(kColumns, kRows);
    game.start();

    for (int i = 0; i < kColumns - 1 - 14; ++i)
        game.step();
    QCOMPARE(game.state(), GameState::Running);
    QCOMPARE(game.body().front().x(), kColumns - 1);

    game.step(); // 再走一步就出界
    QCOMPARE(game.state(), GameState::Over);
}

void TestSnakeGame::rejectsReverse()
{
    SnakeGame game(kColumns, kRows);
    game.start();
    game.turn(Direction::Left);
    game.step();

    QCOMPARE(game.body().front(), QPoint(15, 10));
}

void TestSnakeGame::queuesTurnsInOrder()
{
    SnakeGame game(kColumns, kRows);
    game.start();

    game.turn(Direction::Up);
    game.turn(Direction::Left);

    game.step();
    QCOMPARE(game.body().front(), QPoint(14, 9));
    game.step();
    QCOMPARE(game.body().front(), QPoint(13, 9));
}

void TestSnakeGame::rejectsOppositeOfQueuedTurn()
{
    SnakeGame game(kColumns, kRows);
    game.start();

    game.turn(Direction::Up);
    game.turn(Direction::Down); // 相对队列末尾的"上"是反向

    game.step();
    game.step();
    QCOMPARE(game.body().front(), QPoint(14, 8));
}

void TestSnakeGame::pauseFreezesTheRound()
{
    SnakeGame game(kColumns, kRows);
    game.start();
    game.togglePause();
    QCOMPARE(game.state(), GameState::Paused);

    const QPoint head = game.body().front();
    game.turn(Direction::Up);
    game.step();
    QCOMPARE(game.body().front(), head);

    game.togglePause();
    QCOMPARE(game.state(), GameState::Running);
    game.step();
    QCOMPARE(game.body().front(), QPoint(15, 10));
}

void TestSnakeGame::foodNeverLandsOnTheSnake()
{
    constexpr int kSamples = 2000;
    SnakeGame game(kColumns, kRows);

    for (int i = 0; i < kSamples; ++i) {
        game.start();
        const QPoint food = game.food();

        QVERIFY(food.x() >= 0 && food.x() < kColumns);
        QVERIFY(food.y() >= 0 && food.y() < kRows);
        QVERIFY(std::find(game.body().cbegin(), game.body().cend(), food) == game.body().cend());
    }
}

void TestSnakeGame::foodFavoursTheCentreOnEasy()
{
    // 期望值来自解析计算（见设计文档）：轻松 0.60 / 标准 0.65 / 挑战 0.69。
    // 差距远大于抽样误差，所以断言留 0.02 的余量也不会抖。
    constexpr int kSamples = 5000;
    const double easy = meanFoodDistance(Difficulty::Easy, kSamples);
    const double normal = meanFoodDistance(Difficulty::Normal, kSamples);
    const double hard = meanFoodDistance(Difficulty::Hard, kSamples);

    qInfo(
        "%s",
        qPrintable(QStringLiteral("easy=%1 normal=%2 hard=%3")
                       .arg(easy, 0, 'f', 3)
                       .arg(normal, 0, 'f', 3)
                       .arg(hard, 0, 'f', 3)));

    QVERIFY(easy < normal - 0.02);
    QVERIFY(normal < hard - 0.02);
    QVERIFY(easy < 0.65);
    QVERIFY(hard > 0.65);
}

QTEST_GUILESS_MAIN(TestSnakeGame)

#include "tst_snakegame.moc"
