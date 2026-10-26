#include <widgets/modaloverlay.hpp>

#include <QGraphicsEffect>
#include <QTest>
#include <QWidget>

using namespace Qt::StringLiterals;
using namespace Widgets;

namespace {

auto maskOf(const QWidget &target) -> QWidget *
{
    return target.findChild<QWidget *>(u"ModalMask"_s);
}

} // namespace

// 遮挡层要能盖住任何控件，而不是只会盖主窗口，所以这里用普通 QWidget 当目标。
class TestModalOverlay : public QObject
{
    Q_OBJECT

private slots:
    void maskStartsHidden();
    void maskFillsTarget();
    void maskFollowsResize();
    void maskKeepsInsetInsideSmallTarget();
    void blurIsOffUntilRequested();
    void toleratesNullTarget();
};

void TestModalOverlay::maskStartsHidden()
{
    QWidget target;
    ModalOverlay overlay(&target);

    QWidget *mask = maskOf(target);
    QVERIFY(mask);
    QVERIFY(mask->isHidden());
    QVERIFY(!overlay.maskIsVisible());
}

void TestModalOverlay::maskFillsTarget()
{
    QWidget target;
    target.resize(200, 100);

    ModalOverlay overlay(&target);
    overlay.setMaskVisible(true);

    QVERIFY(overlay.maskIsVisible());
    QCOMPARE(maskOf(target)->geometry(), QRect(0, 0, 200, 100));
}

void TestModalOverlay::maskFollowsResize()
{
    QWidget target;
    // 从未 show 过的控件会把 resize 事件攒到显示时才发，事件过滤器收不到；
    // WA_DontShowOnScreen 让它成为真窗口但不真的弹出来。
    target.setAttribute(Qt::WA_DontShowOnScreen, true);
    target.show();
    target.resize(200, 100);

    ModalOverlay overlay(&target);
    overlay.setInset(10);
    overlay.setMaskVisible(true);
    QCOMPARE(maskOf(target)->geometry(), QRect(10, 10, 180, 80));

    // 最大化/还原会改目标尺寸，遮罩不跟着走就会错位。
    target.resize(300, 200);
    QCOMPARE(maskOf(target)->geometry(), QRect(10, 10, 280, 180));
}

void TestModalOverlay::maskKeepsInsetInsideSmallTarget()
{
    QWidget target;
    target.resize(120, 60);

    ModalOverlay overlay(&target);
    overlay.setInset(100); // 远大于目标尺寸
    overlay.setMaskVisible(true);

    // 内缩被夹到短边的一半：遮罩可能退化成一条线，但不会出现负尺寸。
    const QRect geometry = maskOf(target)->geometry();
    QCOMPARE(geometry.topLeft(), QPoint(30, 30));
    QVERIFY(geometry.width() >= 0);
    QVERIFY(geometry.height() >= 0);
}

void TestModalOverlay::blurIsOffUntilRequested()
{
    QWidget target;
    QWidget content;

    ModalOverlay overlay(&target);
    overlay.setBlurTarget(&content);
    QVERIFY(content.graphicsEffect());
    QVERIFY(!content.graphicsEffect()->isEnabled());

    overlay.setBlurActive(true);
    QVERIFY(overlay.blurIsActive());
    QVERIFY(content.graphicsEffect()->isEnabled());

    overlay.setBlurActive(false);
    QVERIFY(!content.graphicsEffect()->isEnabled());
}

void TestModalOverlay::toleratesNullTarget()
{
    // 传空指针是调用方的错：构造时 QTC_ASSERT 会喊一声，之后所有调用退化成空操作，
    // 但不能崩——遮挡层只是装饰，不该为它终止整个程序。
    ModalOverlay overlay(nullptr);
    overlay.setMaskVisible(true);
    overlay.setBlurActive(true);

    QVERIFY(!overlay.maskIsVisible());
    QVERIFY(!overlay.blurIsActive());
}

QTEST_MAIN(TestModalOverlay)

#include "tst_modaloverlay.moc"
