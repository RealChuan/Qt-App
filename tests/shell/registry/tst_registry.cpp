#include <shell/command.hpp>
#include <shell/page.hpp>
#include <shell/registry.hpp>

#include <QSignalSpy>
#include <QTest>

using namespace Qt::StringLiterals;

namespace {

class TestPage : public Shell::IPage
{
public:
    explicit TestPage(QString id)
        : m_id(std::move(id))
    {}

    auto id() const -> QString override { return m_id; }
    auto title() const -> QString override { return u"Test page"_s; }
    auto widget() -> QWidget * override { return nullptr; }

private:
    QString m_id;
};

class TestCommand : public Shell::ICommand
{
public:
    explicit TestCommand(QString id)
        : m_id(std::move(id))
    {}

    auto id() const -> QString override { return m_id; }
    auto title() const -> QString override { return u"Test command"_s; }
    void execute() override {}

private:
    QString m_id;
};

// Registry 是进程级单例且不支持移除，注册进去的指针必须活到进程结束：
// 后续用例的「id 重复」判定会遍历并解引用它们。所以这里刻意用 new 泄漏，
// 不能用栈对象。
auto registry() -> Shell::Registry *
{
    return Shell::Registry::instance();
}

} // namespace

// 插件在 extensionsInitialized() 里往 Registry 注册页面与命令。
// id 重复必须被挡住：外壳按 id 建按钮和快捷键，重复会让两个插件抢同一个入口。
class TestRegistry : public QObject
{
    Q_OBJECT

private slots:
    void addNullIsIgnored();
    void addPageEmitsPagesChanged();
    void duplicatePageIdIsRejected();
    void pagesKeepRegistrationOrder();
    void addCommandEmitsCommandsChanged();
    void duplicateCommandIdIsRejected();
};

void TestRegistry::addNullIsIgnored()
{
    const int pagesBefore = int(registry()->pages().size());
    const int commandsBefore = int(registry()->commands().size());

    QSignalSpy pagesSpy(registry(), &Shell::Registry::pagesChanged);
    QSignalSpy commandsSpy(registry(), &Shell::Registry::commandsChanged);

    registry()->addPage(nullptr);
    registry()->addCommand(nullptr);

    QCOMPARE(int(registry()->pages().size()), pagesBefore);
    QCOMPARE(int(registry()->commands().size()), commandsBefore);
    QCOMPARE(pagesSpy.count(), 0);
    QCOMPARE(commandsSpy.count(), 0);
}

void TestRegistry::addPageEmitsPagesChanged()
{
    const int before = int(registry()->pages().size());
    QSignalSpy spy(registry(), &Shell::Registry::pagesChanged);

    auto *page = new TestPage(u"test.page.signal"_s);
    registry()->addPage(page);

    QCOMPARE(int(registry()->pages().size()), before + 1);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(registry()->pages().last(), static_cast<Shell::IPage *>(page));
}

void TestRegistry::duplicatePageIdIsRejected()
{
    auto *first = new TestPage(u"test.page.duplicate"_s);
    auto *second = new TestPage(u"test.page.duplicate"_s);

    registry()->addPage(first);
    const int before = int(registry()->pages().size());
    QSignalSpy spy(registry(), &Shell::Registry::pagesChanged);

    registry()->addPage(second);

    // 第二个同 id 页面既不进表，也不触发刷新信号
    QCOMPARE(int(registry()->pages().size()), before);
    QCOMPARE(spy.count(), 0);
    QVERIFY(!registry()->pages().contains(second));
}

void TestRegistry::pagesKeepRegistrationOrder()
{
    const int before = int(registry()->pages().size());

    auto *first = new TestPage(u"test.page.order.1"_s);
    auto *second = new TestPage(u"test.page.order.2"_s);
    registry()->addPage(first);
    registry()->addPage(second);

    const QList<Shell::IPage *> pages = registry()->pages();
    QCOMPARE(int(pages.size()), before + 2);
    QCOMPARE(pages.at(pages.size() - 2), static_cast<Shell::IPage *>(first));
    QCOMPARE(pages.at(pages.size() - 1), static_cast<Shell::IPage *>(second));
}

void TestRegistry::addCommandEmitsCommandsChanged()
{
    const int before = int(registry()->commands().size());
    QSignalSpy spy(registry(), &Shell::Registry::commandsChanged);

    auto *command = new TestCommand(u"test.command.signal"_s);
    registry()->addCommand(command);

    QCOMPARE(int(registry()->commands().size()), before + 1);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(registry()->commands().last(), static_cast<Shell::ICommand *>(command));
}

void TestRegistry::duplicateCommandIdIsRejected()
{
    auto *first = new TestCommand(u"test.command.duplicate"_s);
    auto *second = new TestCommand(u"test.command.duplicate"_s);

    registry()->addCommand(first);
    const int before = int(registry()->commands().size());
    QSignalSpy spy(registry(), &Shell::Registry::commandsChanged);

    registry()->addCommand(second);

    QCOMPARE(int(registry()->commands().size()), before);
    QCOMPARE(spy.count(), 0);
    QVERIFY(!registry()->commands().contains(second));
}

QTEST_MAIN(TestRegistry)

#include "tst_registry.moc"
