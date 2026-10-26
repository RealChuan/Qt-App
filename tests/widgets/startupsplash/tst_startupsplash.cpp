#include <widgets/startupsplash.hpp>

#include <QLabel>
#include <QProgressBar>
#include <QTest>

using namespace Qt::StringLiterals;
using namespace Widgets;

// 启动画面在插件加载期间不会收到事件，所以它不能自带任何定时器：
// 进度只在调用方主动推进时才变化，这条约束靠下面的用例守住。
class TestStartupSplash : public QObject
{
    Q_OBJECT

private slots:
    void hidesProgressUntilFirstUpdate();
    void clampsProgressToRange();
    void showsStatusText();
};

void TestStartupSplash::hidesProgressUntilFirstUpdate()
{
    StartupSplash splash;
    auto *progress = splash.findChild<QProgressBar *>(u"SplashProgress"_s);
    QVERIFY(progress);

    // 启动流程不推进度，进度条就不该占位置。
    QVERIFY(progress->isHidden());

    splash.setProgress(40);
    QVERIFY(!progress->isHidden());
    QCOMPARE(progress->value(), 40);
}

void TestStartupSplash::clampsProgressToRange()
{
    StartupSplash splash;
    auto *progress = splash.findChild<QProgressBar *>(u"SplashProgress"_s);
    QVERIFY(progress);

    splash.setProgress(-5);
    QCOMPARE(progress->value(), 0);

    splash.setProgress(150);
    QCOMPARE(progress->value(), 100);
}

void TestStartupSplash::showsStatusText()
{
    StartupSplash splash;
    auto *label = splash.findChild<QLabel *>(u"SplashText"_s);
    QVERIFY(label);
    QVERIFY(!label->text().isEmpty());
}

QTEST_MAIN(TestStartupSplash)

#include "tst_startupsplash.moc"
