#include <widgets/messagebox.hpp>

#include <QAbstractButton>
#include <QPushButton>
#include <QTest>
#include <QTimer>

using namespace Widgets;

namespace {

// Qt 在 Windows 上给标准按钮加助记符（"&Yes"），macOS 不加；比较前统一去掉。
auto plainText(const QAbstractButton *button) -> QString
{
    QString text = button->text();
    text.remove(QLatin1Char('&'));
    return text;
}

// 只问"有哪些按钮"，不问顺序：QMessageBox::buttons() 的顺序由按钮角色和平台决定
// （Windows 上 Yes|Cancel 返回 [Cancel, Yes]），顺序不在它的契约里。
auto visibleButtonTexts(const MessageBox &box) -> QStringList
{
    QStringList texts;
    for (const QAbstractButton *button : box.buttons()) {
        if (button->isVisibleTo(&box))
            texts.append(plainText(button));
    }
    texts.sort();
    return texts;
}

auto buttonFor(const MessageBox &box, const QString &text) -> QPushButton *
{
    for (QAbstractButton *button : box.buttons()) {
        if (plainText(button) == text)
            return qobject_cast<QPushButton *>(button);
    }
    return nullptr;
}

// exec() 是阻塞调用：先用单发定时器把按钮点掉，再兜一个超时，避免测试挂死。
void clickLater(MessageBox &box, const QString &text)
{
    QTimer::singleShot(0, &box, [&box, text] {
        if (QPushButton *button = buttonFor(box, text))
            button->click();
    });
    QTimer::singleShot(5000, &box, &MessageBox::reject);
}

void pressLater(MessageBox &box, Qt::Key key)
{
    QTimer::singleShot(0, &box, [&box, key] { QTest::keyClick(&box, key); });
    QTimer::singleShot(5000, &box, &MessageBox::reject);
}

} // namespace

// Widgets::MessageBox 的接口和语义都对着 QMessageBox 抄（macOS 上它本身就是 QMessageBox），
// 所以这里只用两边都有的公开接口断言：buttons() / button() / exec()。这样同一份用例在
// 三个平台断言的是同一件事，而不会去碰自绘分支的私有细节（objectName、创建顺序）。
class TestMessageBox : public QObject
{
    Q_OBJECT

private slots:
    void buildsButtonsFromStandardButtons();
    void fallsBackToCloseWhenNothingIsSupported();
    void enterClicksFirstConfirmButton();
    void explicitDefaultButtonWinsOnEnter();
    void execReturnsClickedButton();
    void escapeMapsToCancel();
};

void TestMessageBox::buildsButtonsFromStandardButtons()
{
    MessageBox box;
    box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);

    QCOMPARE(visibleButtonTexts(box), QStringList({QStringLiteral("No"), QStringLiteral("Yes")}));
    QVERIFY(buttonFor(box, QStringLiteral("Yes")));
    QVERIFY(box.button(QMessageBox::No) == buttonFor(box, QStringLiteral("No")));
}

void TestMessageBox::fallsBackToCloseWhenNothingIsSupported()
{
#ifdef Q_OS_MACOS
    // 这两个分支的差异在能力集，不在语义：自绘分支只实现 Ok/Yes/No/Cancel/Close，
    // macOS 借来的 QMessageBox 支持 Save，所以"不支持就兜底"在这里没有对应行为。
    QSKIP("QMessageBox supports Save; the fallback rule belongs to the self-drawn branch");
#else
    MessageBox box;
    // Save 没有自绘实现：告警后忽略，但必须留下一个能关窗的按钮。
    box.setStandardButtons(QMessageBox::Save);

    QCOMPARE(visibleButtonTexts(box), QStringList({QStringLiteral("Close")}));
#endif
}

void TestMessageBox::enterClicksFirstConfirmButton()
{
    MessageBox box;
    box.setStandardButtons(QMessageBox::No | QMessageBox::Yes);

    // 不指定默认按钮时，回车必须命中第一个确认类按钮。QMessageBox 靠 QDialog 的
    // autoDefault 完成，自绘分支靠 applyDefaultButton() 给 Yes 标 setDefault。
    pressLater(box, Qt::Key_Return);

    QCOMPARE(box.exec(), int(QMessageBox::Yes));
}

void TestMessageBox::explicitDefaultButtonWinsOnEnter()
{
    MessageBox box;
    box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    box.setDefaultButton(QMessageBox::No);

    pressLater(box, Qt::Key_Return);

    QCOMPARE(box.exec(), int(QMessageBox::No));
}

void TestMessageBox::execReturnsClickedButton()
{
    MessageBox box;
    box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    clickLater(box, QStringLiteral("No"));

    QCOMPARE(box.exec(), int(QMessageBox::No));
}

void TestMessageBox::escapeMapsToCancel()
{
    MessageBox box;
    box.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);

    pressLater(box, Qt::Key_Escape);

    QCOMPARE(box.exec(), int(QMessageBox::Cancel));
}

QTEST_MAIN(TestMessageBox)

#include "tst_messagebox.moc"
