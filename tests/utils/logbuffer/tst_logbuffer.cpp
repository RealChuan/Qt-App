#include <utils/logbuffer.hpp>

#include <QElapsedTimer>
#include <QTest>

#include <chrono>
#include <thread>
#include <vector>

using namespace AsyncLog;
using namespace std::chrono_literals;

namespace {

LogRecord makeRecord(QtMsgType type, const QString &message)
{
    LogRecord record;
    record.timestampNs = 1;
    record.type = type;
    record.message = message;
    record.threadId = 0;
    return record;
}

} // namespace

// LogBuffer 是日志后端唯一的多线程交界：生产端多线程 push，后端单线程 popBatch。
// 饱和降级策略（丢 DEBUG/INFO、保 WARNING 及以上）一旦写错，线上会静默丢日志，
// 所以这里把队列语义与丢弃计数都锁住。
class TestLogBuffer : public QObject
{
    Q_OBJECT

private slots:
    void severityOrdersByImportance();
    void popBatchReturnsRecordsInOrder();
    void popBatchTimesOutWhenEmpty();
    void closeDrainsRemainingRecords();
    void pushAfterCloseIsIgnored();
    void reopenAcceptsRecordsAgain();
    void overflowDropsDebugButKeepsWarning();
    void takeDroppedResetsCounter();
    void reopenResetsDroppedCounter();
    void concurrentProducersLoseNothing();
};

void TestLogBuffer::severityOrdersByImportance()
{
    // QtMsgType 的数值顺序不是严重性顺序（QtInfoMsg == 4 > QtFatalMsg == 3），
    // 映射表一旦照抄枚举值，级别过滤就会反过来。
    QVERIFY(severity(QtDebugMsg) < severity(QtInfoMsg));
    QVERIFY(severity(QtInfoMsg) < severity(QtWarningMsg));
    QVERIFY(severity(QtWarningMsg) < severity(QtCriticalMsg));
    QVERIFY(severity(QtCriticalMsg) < severity(QtFatalMsg));
}

void TestLogBuffer::popBatchReturnsRecordsInOrder()
{
    LogBuffer buffer;
    buffer.push(makeRecord(QtDebugMsg, QStringLiteral("first")));
    buffer.push(makeRecord(QtInfoMsg, QStringLiteral("second")));
    buffer.push(makeRecord(QtWarningMsg, QStringLiteral("third")));

    std::vector<LogRecord> batch;
    QVERIFY(buffer.popBatch(batch, 100ms));

    QCOMPARE(int(batch.size()), 3);
    QCOMPARE(batch.at(0).message, QStringLiteral("first"));
    QCOMPARE(batch.at(1).message, QStringLiteral("second"));
    QCOMPARE(batch.at(2).message, QStringLiteral("third"));
    QCOMPARE(batch.at(2).type, QtWarningMsg);
}

void TestLogBuffer::popBatchTimesOutWhenEmpty()
{
    LogBuffer buffer;
    std::vector<LogRecord> batch;

    // 未 close 时超时返回 true（后端应继续等待），而不是把空批当成退出信号
    QVERIFY(buffer.popBatch(batch, 5ms));
    QVERIFY(batch.empty());
}

void TestLogBuffer::closeDrainsRemainingRecords()
{
    LogBuffer buffer;
    buffer.push(makeRecord(QtInfoMsg, QStringLiteral("a")));
    buffer.push(makeRecord(QtInfoMsg, QStringLiteral("b")));
    buffer.close();

    std::vector<LogRecord> batch;
    QVERIFY(buffer.popBatch(batch, 100ms));
    QCOMPARE(int(batch.size()), 2);

    batch.clear();
    // close 且排空之后才返回 false，后端据此退出
    QVERIFY(!buffer.popBatch(batch, 5ms));
    QVERIFY(batch.empty());
}

void TestLogBuffer::pushAfterCloseIsIgnored()
{
    LogBuffer buffer;
    buffer.close();
    buffer.push(makeRecord(QtCriticalMsg, QStringLiteral("too late")));

    std::vector<LogRecord> batch;
    QVERIFY(!buffer.popBatch(batch, 5ms));
    QVERIFY(batch.empty());
}

void TestLogBuffer::reopenAcceptsRecordsAgain()
{
    LogBuffer buffer;
    buffer.close();
    buffer.push(makeRecord(QtInfoMsg, QStringLiteral("dropped by close")));

    buffer.reopen();
    buffer.push(makeRecord(QtInfoMsg, QStringLiteral("after reopen")));

    std::vector<LogRecord> batch;
    QVERIFY(buffer.popBatch(batch, 100ms));
    QCOMPARE(int(batch.size()), 1);
    QCOMPARE(batch.front().message, QStringLiteral("after reopen"));
}

void TestLogBuffer::overflowDropsDebugButKeepsWarning()
{
    // 高水位设成 1 字节 + 1ms 等待：立刻进入降级期，不拖慢测试
    LogBuffer buffer(1, 1ms);

    buffer.push(makeRecord(QtDebugMsg, QStringLiteral("dropped")));
    QCOMPARE(buffer.takeDropped(), std::size_t(1));

    // 错误级绝不丢：宁可短暂超出高水位
    buffer.push(makeRecord(QtWarningMsg, QStringLiteral("kept")));

    std::vector<LogRecord> batch;
    QVERIFY(buffer.popBatch(batch, 100ms));
    QCOMPARE(int(batch.size()), 1);
    QCOMPARE(batch.front().message, QStringLiteral("kept"));

    // 只丢了一条 DEBUG，WARNING 没有被计入丢弃
    QCOMPARE(buffer.takeDropped(), std::size_t(0));
}

void TestLogBuffer::takeDroppedResetsCounter()
{
    LogBuffer buffer(1, 1ms);
    buffer.push(makeRecord(QtInfoMsg, QStringLiteral("dropped 1")));
    buffer.push(makeRecord(QtInfoMsg, QStringLiteral("dropped 2")));

    QCOMPARE(buffer.takeDropped(), std::size_t(2));
    // 取走即清零，避免后端重复补报
    QCOMPARE(buffer.takeDropped(), std::size_t(0));
}

void TestLogBuffer::reopenResetsDroppedCounter()
{
    LogBuffer buffer(1, 1ms);
    buffer.push(makeRecord(QtDebugMsg, QStringLiteral("dropped")));
    QCOMPARE(buffer.takeDropped(), std::size_t(1));

    buffer.reopen();
    QCOMPARE(buffer.takeDropped(), std::size_t(0));
}

void TestLogBuffer::concurrentProducersLoseNothing()
{
    constexpr int kProducers = 4;
    constexpr int kPerProducer = 250;
    constexpr int kTotal = kProducers * kPerProducer;

    // 高水位给足，测的是并发正确性而不是降级
    LogBuffer buffer(8 * 1024 * 1024, 200ms);

    std::vector<std::thread> producers;
    producers.reserve(kProducers);
    for (int producer = 0; producer < kProducers; ++producer) {
        producers.emplace_back([&buffer, producer] {
            for (int index = 0; index < kPerProducer; ++index) {
                buffer.push(makeRecord(QtInfoMsg, QStringLiteral("%1-%2").arg(producer).arg(index)));
            }
        });
    }

    std::vector<LogRecord> batch;
    int consumed = 0;
    QElapsedTimer timer;
    timer.start();
    while (consumed < kTotal && timer.elapsed() < 10000) {
        if (!buffer.popBatch(batch, 50ms))
            break;
        consumed += int(batch.size());
        batch.clear();
    }

    for (std::thread &producer : producers)
        producer.join();

    QCOMPARE(consumed, kTotal);
    QCOMPARE(buffer.takeDropped(), std::size_t(0));
}

QTEST_GUILESS_MAIN(TestLogBuffer)

#include "tst_logbuffer.moc"
