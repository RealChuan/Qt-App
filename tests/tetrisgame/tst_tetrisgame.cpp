#include "tetrisgame.hpp"

#include <QPoint>
#include <QSet>
#include <QTest>

#include <algorithm>

using namespace Tetris;

namespace {

constexpr quint32 kSeed = 20261024;

auto sortedCells(const QVector<QPoint> &cells) -> QVector<QPoint>
{
    QVector<QPoint> copy = cells;
    std::sort(copy.begin(), copy.end(), [](const QPoint &a, const QPoint &b) {
        return a.y() != b.y() ? a.y() < b.y() : a.x() < b.x();
    });
    return copy;
}

auto settledCount(const TetrisGame &game) -> int
{
    int count = 0;
    for (int y = 0; y < kVisibleRows; ++y) {
        for (int x = 0; x < kColumns; ++x) {
            if (game.settledCellAt(x, y) != 0)
                ++count;
        }
    }
    return count;
}

auto hasFullRow(const TetrisGame &game) -> bool
{
    for (int y = 0; y < kVisibleRows; ++y) {
        int filled = 0;
        for (int x = 0; x < kColumns; ++x) {
            if (game.settledCellAt(x, y) != 0)
                ++filled;
        }
        if (filled == kColumns)
            return true;
    }
    return false;
}

void slideTo(TetrisGame &game, int targetLeft)
{
    QVector<QPoint> cells = game.activeCells();
    if (cells.isEmpty())
        return;

    int left = cells.first().x();
    for (const QPoint &cell : cells)
        left = std::min(left, cell.x());

    while (left < targetLeft && game.moveBy(1))
        ++left;
    while (left > targetLeft && game.moveBy(-1))
        --left;
}

auto columnHeight(const TetrisGame &game, int x) -> int
{
    for (int y = 0; y < kVisibleRows; ++y) {
        if (game.settledCellAt(x, y) != 0)
            return kVisibleRows - y;
    }
    return 0;
}

} // namespace

class TestTetrisGame : public QObject
{
    Q_OBJECT

private slots:
    void startsEmpty();
    void rot4TimesReturnsHome_data();
    void rot4TimesReturnsHome();
    void wallKickKeepsPieceInside();
    void hardDropLandsAndScores();
    void holdSwapsOncePerPiece();
    void gravityMovesOneRowPerInterval();
    void locksAfterDelay();
    void longPlayClearsRows();
};

void TestTetrisGame::startsEmpty()
{
    TetrisGame game(kSeed);
    QCOMPARE(game.state(), GameState::Idle);

    game.start();
    QCOMPARE(game.state(), GameState::Running);
    QCOMPARE(game.score(), 0);
    QCOMPARE(game.lines(), 0);
    QCOMPARE(game.level(), 1);
    QCOMPARE(settledCount(game), 0);
    QCOMPARE(game.activeCells().size(), 4);
    QCOMPARE(game.nextQueue(5).size(), 5);
    QVERIFY(!game.heldType().has_value());
}

void TestTetrisGame::rot4TimesReturnsHome_data()
{
    QTest::addColumn<int>("type");
    for (int type = 0; type < kPieceCount; ++type)
        QTest::newRow(qPrintable(QString::number(type))) << type;
}

void TestTetrisGame::rot4TimesReturnsHome()
{
    QFETCH(int, type);

    TetrisGame game(kSeed);
    game.start();

    // 换到指定方块再测：hold 一次即把当前块塞进暂存，随后队列顶上的块顶上。
    while (static_cast<int>(game.activeType()) != type) {
        game.hold();
        if (static_cast<int>(game.activeType()) == type)
            break;
        game.hardDrop();
        for (int t = 0; t < 12; ++t)
            game.advance(16);
    }
    QCOMPARE(static_cast<int>(game.activeType()), type);

    const QVector<QPoint> before = sortedCells(game.activeCells());
    if (game.activeType() == PieceType::O) {
        QVERIFY(!game.rotate(true)); // O 不参与旋转
        return;
    }

    for (int i = 0; i < 4; ++i)
        QVERIFY(game.rotate(true));

    QCOMPARE(sortedCells(game.activeCells()), before);
}

void TestTetrisGame::wallKickKeepsPieceInside()
{
    TetrisGame game(kSeed);
    game.start();

    while (game.moveBy(-1)) {
    }

    QVERIFY(game.rotate(true));
    for (const QPoint &cell : game.activeCells()) {
        QVERIFY(cell.x() >= 0);
        QVERIFY(cell.x() < kColumns);
    }
}

void TestTetrisGame::hardDropLandsAndScores()
{
    TetrisGame game(kSeed);
    game.start();

    game.hardDrop();
    QVERIFY(game.score() > 0);
    QCOMPARE(settledCount(game), 4);

    int lowest = -1;
    for (int y = 0; y < kVisibleRows; ++y) {
        for (int x = 0; x < kColumns; ++x) {
            if (game.settledCellAt(x, y) != 0)
                lowest = std::max(lowest, y);
        }
    }
    QCOMPARE(lowest, kVisibleRows - 1);
    QCOMPARE(game.activeCells().size(), 4); // 已经换上新块
}

void TestTetrisGame::holdSwapsOncePerPiece()
{
    TetrisGame game(kSeed);
    game.start();

    const PieceType first = game.activeType();
    game.hold();

    QVERIFY(game.heldType().has_value());
    QCOMPARE(*game.heldType(), first);
    QVERIFY(!game.canHold());

    const PieceType afterHold = game.activeType();
    game.hold(); // 同一块只能暂存一次
    QCOMPARE(game.activeType(), afterHold);

    game.hardDrop();
    QVERIFY(game.canHold()); // 落定后解除
}

void TestTetrisGame::gravityMovesOneRowPerInterval()
{
    TetrisGame game(kSeed);
    game.start();

    const int before = game.activeCells().first().y();
    game.advance(800); // 1 级 800 ms 一格
    QCOMPARE(game.activeCells().first().y(), before + 1);
}

void TestTetrisGame::locksAfterDelay()
{
    TetrisGame game(kSeed);
    game.start();

    // 软降到贴地：活动格与幽灵格重合。
    while (sortedCells(game.activeCells()) != sortedCells(game.ghostCells()))
        game.softDrop();

    game.advance(400); // 锁定延迟 500 ms，还没到
    QCOMPARE(settledCount(game), 0);

    game.advance(120);
    QCOMPARE(settledCount(game), 4);
}

void TestTetrisGame::longPlayClearsRows()
{
    TetrisGame game(kSeed);
    game.start();

    // 简易堆叠：每次把方块放到"其覆盖列最矮"的位置，保证能攒出完整的行。
    for (int i = 0; i < 160 && game.state() == GameState::Running; ++i) {
        const QVector<QPoint> cells = game.activeCells();
        int left = cells.first().x();
        int right = cells.first().x();
        for (const QPoint &cell : cells) {
            left = std::min(left, cell.x());
            right = std::max(right, cell.x());
        }
        const int width = right - left + 1;

        int bestLeft = left;
        int bestCost = std::numeric_limits<int>::max();
        for (int candidate = 0; candidate + width <= kColumns; ++candidate) {
            int cost = 0;
            for (int x = candidate; x < candidate + width; ++x)
                cost = std::max(cost, columnHeight(game, x));
            if (cost < bestCost) {
                bestCost = cost;
                bestLeft = candidate;
            }
        }

        slideTo(game, bestLeft);
        game.hardDrop();

        for (int t = 0; t < 20; ++t)
            game.advance(16);

        QVERIFY(!hasFullRow(game)); // 消行必须真的把行拿掉
    }

    QVERIFY(game.lines() > 0);
    QVERIFY(game.score() > 0);
    QVERIFY(game.level() >= 1);
}

QTEST_GUILESS_MAIN(TestTetrisGame)

#include "tst_tetrisgame.moc"
