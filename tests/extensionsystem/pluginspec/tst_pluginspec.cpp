#include <extensionsystem/pluginspec.hpp>

#include <QTest>

using namespace ExtensionSystem;

// 版本比较决定「依赖是否满足」：写错一个比较，插件要么在版本足够时被判缺失，
// 要么在版本不足时被放行。解析只取前四段、非数字段按 0 处理，这些隐含规则需要锁住。
class TestPluginSpec : public QObject
{
    Q_OBJECT

private slots:
    void versionCompare_data();
    void versionCompare();
    void versionCompareIsAntisymmetric();
    void freshSpecIsInvalid();
    void dependencyEquality();
    void dependencyTypeDefaultsToRequired();
};

void TestPluginSpec::versionCompare_data()
{
    QTest::addColumn<QString>("lhs");
    QTest::addColumn<QString>("rhs");
    QTest::addColumn<int>("expected");

    QTest::newRow("patch 10 beats patch 9")
        << QStringLiteral("1.2.10") << QStringLiteral("1.2.9") << 1;
    QTest::newRow("patch 9 loses to patch 10")
        << QStringLiteral("1.2.9") << QStringLiteral("1.2.10") << -1;
    QTest::newRow("minor beats patch") << QStringLiteral("1.3.0") << QStringLiteral("1.2.99") << 1;
    QTest::newRow("major dominates") << QStringLiteral("2.0") << QStringLiteral("1.99.99") << 1;
    QTest::newRow("missing segments are zero")
        << QStringLiteral("1.2") << QStringLiteral("1.2.0") << 0;
    QTest::newRow("zero padded segments")
        << QStringLiteral("1.2.0.0") << QStringLiteral("1.2") << 0;
    QTest::newRow("longer version wins") << QStringLiteral("1.2.1") << QStringLiteral("1.2") << 1;
    QTest::newRow("numeric not lexicographic")
        << QStringLiteral("10.0") << QStringLiteral("9.0") << 1;
    QTest::newRow("leading zeros ignored") << QStringLiteral("1.02") << QStringLiteral("1.2") << 0;
    QTest::newRow("empty equals empty") << QString() << QString() << 0;
    QTest::newRow("empty equals zero") << QString() << QStringLiteral("0") << 0;
    QTest::newRow("empty loses to one") << QString() << QStringLiteral("1") << -1;
    // 预发布后缀不参与比较，只取前四段数字
    QTest::newRow("suffix ignored") << QStringLiteral("1.2.3-beta") << QStringLiteral("1.2.3") << 0;
    // 非数字段解析失败后按 0 处理
    QTest::newRow("garbage equals zero") << QStringLiteral("abc") << QStringLiteral("0") << 0;
    // 只有前四段有效，第五段不影响结果
    QTest::newRow("fifth segment ignored")
        << QStringLiteral("1.2.3.4.5") << QStringLiteral("1.2.3.4") << 0;
}

void TestPluginSpec::versionCompare()
{
    QFETCH(QString, lhs);
    QFETCH(QString, rhs);
    QFETCH(int, expected);

    QCOMPARE(PluginSpec::versionCompare(lhs, rhs), expected);
}

void TestPluginSpec::versionCompareIsAntisymmetric()
{
    const QStringList versions
        = {QStringLiteral("0"),
           QStringLiteral("1"),
           QStringLiteral("1.2"),
           QStringLiteral("1.2.0"),
           QStringLiteral("1.2.9"),
           QStringLiteral("1.2.10"),
           QStringLiteral("2.0.0.1")};

    for (const QString &lhs : versions) {
        for (const QString &rhs : versions) {
            const int forward = PluginSpec::versionCompare(lhs, rhs);
            const int backward = PluginSpec::versionCompare(rhs, lhs);
            // 只允许 -1 / 0 / 1，且两个方向必须相反
            QVERIFY2(qAbs(forward) <= 1, qPrintable(lhs + QLatin1Char(' ') + rhs));
            QCOMPARE(forward, -backward);
        }
    }
}

void TestPluginSpec::freshSpecIsInvalid()
{
    PluginSpec spec;

    QCOMPARE(spec.state(), PluginSpec::State::Invalid);
    QVERIFY(spec.stateText() != QString());
    QVERIFY(!spec.hasError());
    QVERIFY(spec.errorString().isEmpty());

    // 未被明确要求时默认启用，且没有依赖挡路
    QVERIFY(!spec.isRequired());
    QVERIFY(spec.isEnabledBySettings());
    QVERIFY(spec.isEffectivelyEnabled());

    QVERIFY(spec.id().isEmpty());
    QVERIFY(spec.dependencies().isEmpty());
    QVERIFY(spec.dependencySpecs().isEmpty());
    QVERIFY(spec.filePath().isEmpty());
    QCOMPARE(spec.plugin(), nullptr);
    QCOMPARE(spec.loadTimeMs(), qint64(0));
}

void TestPluginSpec::dependencyEquality()
{
    PluginDependency first;
    first.id = QStringLiteral("danmakuplugin");
    first.version = QStringLiteral("0.2.1");
    first.type = PluginDependency::Type::Optional;

    PluginDependency same = first;
    QVERIFY(first == same);

    same.version = QStringLiteral("0.2.2");
    QVERIFY(first != same);
}

void TestPluginSpec::dependencyTypeDefaultsToRequired()
{
    // 缺省不是 Optional：漏写 Type 必须是硬依赖，否则插件会静默降级
    PluginDependency dependency;
    QCOMPARE(dependency.type, PluginDependency::Type::Required);
}

QTEST_GUILESS_MAIN(TestPluginSpec)

#include "tst_pluginspec.moc"
