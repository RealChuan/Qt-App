#include <utils/uihelpers.hpp>

#include <QRectF>
#include <QTest>

using namespace Utils;

// fitGrid 是贪吃蛇与俄罗斯方块共用的棋盘几何：错一个像素不会崩，只会画歪，
// 所以这里把取 min、留白、退化输入三类边界锁住。
class TestBoardGrid : public QObject
{
    Q_OBJECT

private slots:
    void takesSmallerOfBothDirections();
    void subtractsPaddingBeforeFitting();
    void rejectsPaddingLargerThanArea();
    void rejectsEmptyGrid();
    void cellRectFollowsCellSize();
    void rowRectSpansGridWidth();
};

void TestBoardGrid::takesSmallerOfBothDirections()
{
    // 300/10=30，200/10=20：格子按更紧的那一边取 20，棋盘上下贴边、左右居中。
    const GridMetrics grid = fitGrid(QSize(300, 200), 10, 10);

    QCOMPARE(grid.cell, 20);
    QCOMPARE(grid.columns, 10);
    QCOMPARE(grid.rows, 10);
    QCOMPARE(grid.rect, QRect(50, 0, 200, 200));
}

void TestBoardGrid::subtractsPaddingBeforeFitting()
{
    // 留白 10：可用 300×240，300/15=20 而 240/10=24，仍然取 20；
    // 棋盘 300×200，在控件里居中的位置要把留白算进去。
    const GridMetrics grid = fitGrid(QSize(320, 260), 15, 10, 10);

    QCOMPARE(grid.cell, 20);
    QCOMPARE(grid.rect, QRect(10, 30, 300, 200));
}

void TestBoardGrid::rejectsPaddingLargerThanArea()
{
    // 20×20 的控件留 10 的白，可用尺寸变成 0：cell 为 0，调用方据此跳过绘制。
    const GridMetrics grid = fitGrid(QSize(20, 20), 10, 10, 10);

    QCOMPARE(grid.cell, 0);
    QVERIFY(grid.rect.isNull());
}

void TestBoardGrid::rejectsEmptyGrid()
{
    QCOMPARE(fitGrid(QSize(300, 200), 0, 10).cell, 0);
    QCOMPARE(fitGrid(QSize(300, 200), 10, 0).cell, 0);
}

void TestBoardGrid::cellRectFollowsCellSize()
{
    GridMetrics grid;
    grid.cell = 20;
    grid.columns = 10;
    grid.rows = 10;
    grid.rect = QRect(10, 30, 200, 200);

    QCOMPARE(grid.cellRect(QPoint(0, 0)), QRectF(10, 30, 20, 20));
    QCOMPARE(grid.cellRect(QPoint(2, 3)), QRectF(50, 90, 20, 20));
}

void TestBoardGrid::rowRectSpansGridWidth()
{
    GridMetrics grid;
    grid.cell = 20;
    grid.columns = 10;
    grid.rows = 10;
    grid.rect = QRect(10, 30, 200, 200);

    QCOMPARE(grid.rowRect(0), QRectF(10, 30, 200, 20));
    QCOMPARE(grid.rowRect(4), QRectF(10, 110, 200, 20));
}

QTEST_GUILESS_MAIN(TestBoardGrid)

#include "tst_boardgrid.moc"
