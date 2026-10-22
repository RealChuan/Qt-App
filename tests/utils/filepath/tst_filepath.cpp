#include <utils/filepath.hpp>

#include <QTemporaryDir>
#include <QTest>

using namespace Utils;

namespace {

// std::expected 在成功时调用 error() 会触发 MSVC STL 的调试断言，而 QVERIFY2 的
// 消息参数总会被求值，所以这里先短路，避免把成功路径也送进 error()。
template<typename T>
QString errorText(const Utils::Result<T> &result)
{
    return result ? QString() : result.error();
}

} // namespace

// FilePath 是整个项目所有文件操作的入口：插件扫描、设置、资源、临时文件都走它。
// 它自己实现了路径拼接/裁剪/比较，不经过 QDir，所以这些边界必须锁住。
class TestFilePath : public QObject
{
    Q_OBJECT

private slots:
    void fromStringNormalizesSeparators();
    void fromStringCollapsesDotSegments();
    void pathAppended();
    void pathAppendedKeepsEmptyArgument();
    void pathAppendedReplacesOnAbsoluteArgument();
    void pathComponents();
    void fileNameWithPathComponents();
    void fileNameParts();
    void parentDir_data();
    void parentDir();
    void isChildOf_data();
    void isChildOf();
    void isChildOfIgnoresCaseOnWindows();
    void relativeChildPath();
    void withSuffixAppends();
    void resourcePathRoundTrip();
    void equalityAndOrdering();
    void filePathsCommonPath();
    void filePathsCommonPathWithUnrelatedRelativePaths();
    void filePathsSettingsRoundTrip();
    void fileContentsRoundTrip();
    void copyAndRemove();
};

void TestFilePath::fromStringNormalizesSeparators()
{
    QCOMPARE(FilePath::fromString(QStringLiteral("a\\b\\c")).path(), QStringLiteral("a/b/c"));
    QCOMPARE(FilePath::fromString(QStringLiteral("a//b///c")).path(), QStringLiteral("a/b/c"));
}

void TestFilePath::fromStringCollapsesDotSegments()
{
    QCOMPARE(FilePath::fromString(QStringLiteral("a/b/../c")).path(), QStringLiteral("a/c"));
    QCOMPARE(FilePath::fromString(QStringLiteral("./a/b")).path(), QStringLiteral("a/b"));
    QCOMPARE(FilePath::fromString(QStringLiteral("a/./b")).path(), QStringLiteral("a/b"));
}

void TestFilePath::pathAppended()
{
    // 尾部斜杠不能导致出现双斜杠
    QCOMPARE(
        (FilePath(QStringLiteral("/a/b/")) / QStringLiteral("c")).path(), QStringLiteral("/a/b/c"));
    QCOMPARE(
        (FilePath(QStringLiteral("/a/b")) / QStringLiteral("c/d")).path(),
        QStringLiteral("/a/b/c/d"));
    // 传入反斜杠也按分隔符处理
    QCOMPARE(
        (FilePath(QStringLiteral("/a")) / QStringLiteral("b\\c")).path(), QStringLiteral("/a/b/c"));
    // 空路径作为基底：结果就是参数本身
    QCOMPARE((FilePath() / QStringLiteral("a/b")).path(), QStringLiteral("a/b"));
    // 参数里的 ".." 会被清理
    QCOMPARE(
        (FilePath(QStringLiteral("/a/b")) / QStringLiteral("../c")).path(), QStringLiteral("/a/c"));
}

void TestFilePath::pathAppendedKeepsEmptyArgument()
{
    const FilePath base(QStringLiteral("/a/b"));
    QCOMPARE((base / QString()).path(), base.path());
}

void TestFilePath::pathAppendedReplacesOnAbsoluteArgument()
{
    QCOMPARE(
        (FilePath(QStringLiteral("/a/b")) / QStringLiteral("/c/d")).path(), QStringLiteral("/c/d"));
}

void TestFilePath::pathComponents()
{
    const auto toStringList = [](const QList<QStringView> &views) {
        QStringList result;
        for (const QStringView &view : views)
            result.append(view.toString());
        return result;
    };

    QCOMPARE(
        toStringList(FilePath(QStringLiteral("/a/b/c")).pathComponents()),
        QStringList({QStringLiteral("a"), QStringLiteral("b"), QStringLiteral("c")}));
    // 空段（连续斜杠、根斜杠）不产生组件
    QCOMPARE(
        toStringList(FilePath(QStringLiteral("/a//b")).pathComponents()),
        QStringList({QStringLiteral("a"), QStringLiteral("b")}));
    QVERIFY(FilePath(QStringLiteral("/")).pathComponents().isEmpty());
    QVERIFY(FilePath().pathComponents().isEmpty());
}

void TestFilePath::fileNameWithPathComponents()
{
    const FilePath path(QStringLiteral("/a/b/c.txt"));
    QCOMPARE(path.fileNameWithPathComponents(1), QStringLiteral("c.txt"));
    QCOMPARE(path.fileNameWithPathComponents(2), QStringLiteral("b/c.txt"));
    QCOMPARE(path.fileNameWithPathComponents(3), QStringLiteral("a/b/c.txt"));
    // 越界与非正数都退化为纯文件名
    QCOMPARE(path.fileNameWithPathComponents(0), QStringLiteral("c.txt"));
    QCOMPARE(path.fileNameWithPathComponents(-1), QStringLiteral("c.txt"));
    QCOMPARE(path.fileNameWithPathComponents(9), QStringLiteral("c.txt"));
}

void TestFilePath::fileNameParts()
{
    const FilePath path(QStringLiteral("/a/b.tar.gz"));
    QCOMPARE(path.fileName(), QStringLiteral("b.tar.gz"));
    QCOMPARE(path.baseName(), QStringLiteral("b"));
    QCOMPARE(path.completeBaseName(), QStringLiteral("b.tar"));
    QCOMPARE(path.suffix(), QStringLiteral("tar.gz"));
    QCOMPARE(path.completeSuffix(), QStringLiteral("gz"));

    const FilePath noSuffix(QStringLiteral("/a/noext"));
    QCOMPARE(noSuffix.baseName(), QStringLiteral("noext"));
    QCOMPARE(noSuffix.completeBaseName(), QStringLiteral("noext"));
    QVERIFY(noSuffix.suffix().isEmpty());
    QVERIFY(noSuffix.completeSuffix().isEmpty());
}

void TestFilePath::parentDir_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("expected");

    QTest::newRow("nested") << QStringLiteral("/a/b") << QStringLiteral("/a");
    QTest::newRow("one level below root") << QStringLiteral("/a") << QStringLiteral("/");
    QTest::newRow("root") << QStringLiteral("/") << QStringLiteral("/");
    QTest::newRow("no separator") << QStringLiteral("abc") << QString();
    QTest::newRow("empty") << QString() << QString();
}

void TestFilePath::parentDir()
{
    QFETCH(QString, input);
    QFETCH(QString, expected);

    QCOMPARE(FilePath(input).parentDir().path(), expected);
}

void TestFilePath::isChildOf_data()
{
    QTest::addColumn<QString>("child");
    QTest::addColumn<QString>("parent");
    QTest::addColumn<bool>("expected");

    QTest::newRow("direct child") << QStringLiteral("/a/b/c") << QStringLiteral("/a/b") << true;
    QTest::newRow("grand child") << QStringLiteral("/a/b/c") << QStringLiteral("/a") << true;
    QTest::newRow("same path") << QStringLiteral("/a/b") << QStringLiteral("/a/b") << false;
    // 前缀相同但下一字符不是分隔符，不能算子路径
    QTest::newRow("sibling with shared prefix")
        << QStringLiteral("/a/bc") << QStringLiteral("/a/b") << false;
    QTest::newRow("parent with trailing slash")
        << QStringLiteral("/a/b/c") << QStringLiteral("/a/b/") << true;
    QTest::newRow("empty parent") << QStringLiteral("/a/b") << QString() << false;
}

void TestFilePath::isChildOf()
{
    QFETCH(QString, child);
    QFETCH(QString, parent);
    QFETCH(bool, expected);

    QCOMPARE(FilePath(child).isChildOf(FilePath(parent)), expected);
}

void TestFilePath::isChildOfIgnoresCaseOnWindows()
{
    const bool expected =
#ifdef Q_OS_WIN
        true;
#else
        false;
#endif
    QCOMPARE(FilePath(QStringLiteral("/a/B/c")).isChildOf(FilePath(QStringLiteral("/a/b"))), expected);
}

void TestFilePath::relativeChildPath()
{
    QCOMPARE(
        FilePath(QStringLiteral("/a/b/c")).relativeChildPath(FilePath(QStringLiteral("/a/b"))).path(),
        QStringLiteral("c"));
    QCOMPARE(
        FilePath(QStringLiteral("/a/b/c/d")).relativeChildPath(FilePath(QStringLiteral("/a"))).path(),
        QStringLiteral("b/c/d"));
    // 不是子路径时返回空，而不是抛异常
    QVERIFY(
        FilePath(QStringLiteral("/x/y")).relativeChildPath(FilePath(QStringLiteral("/a"))).isEmpty());
}

void TestFilePath::withSuffixAppends()
{
    // 语义是「追加」而不是「替换」，容易被误用，这里锁住
    QCOMPARE(
        FilePath(QStringLiteral("/a/b.txt")).withSuffix(QStringLiteral(".bak")).path(),
        QStringLiteral("/a/b.txt.bak"));
}

void TestFilePath::resourcePathRoundTrip()
{
    const FilePath resource(QStringLiteral(":/qss/base.css"));
    QVERIFY(resource.isResourceFile());
    QVERIFY(resource.isAbsolutePath());
    QCOMPARE(resource.toUrl(), QUrl(QStringLiteral("qrc:/qss/base.css")));

    const FilePath local = FilePath::fromUrl(QUrl::fromLocalFile(QStringLiteral("/a/b c.css")));
    QCOMPARE(local.path(), QStringLiteral("/a/b c.css"));
    QVERIFY(!local.isResourceFile());
}

void TestFilePath::equalityAndOrdering()
{
    const FilePath first(QStringLiteral("/a/b"));
    const FilePath same(QStringLiteral("/a/b"));
    const FilePath other(QStringLiteral("/a/c"));

    QVERIFY(first == same);
    QVERIFY(first != other);
    QVERIFY(first < other);

    FilePaths paths = FilePaths::fromStrings(
        {QStringLiteral("/b"), QStringLiteral("/a"), QStringLiteral("/c")});
    paths.sort();
    QCOMPARE(
        paths.toFsPathStrings(),
        QStringList({QStringLiteral("/a"), QStringLiteral("/b"), QStringLiteral("/c")}));
}

void TestFilePath::filePathsCommonPath()
{
    const FilePaths paths = FilePaths::fromStrings(
        {QStringLiteral("/a/b/c"), QStringLiteral("/a/b/d"), QStringLiteral("/a/e")});
    QCOMPARE(paths.commonPath().path(), QStringLiteral("/a"));

    const FilePaths single = FilePaths::fromStrings({QStringLiteral("/a/b")});
    QCOMPARE(single.commonPath().path(), QStringLiteral("/a/b"));
}

void TestFilePath::filePathsCommonPathWithUnrelatedRelativePaths()
{
    // 回归：相对路径且首段不同时曾退化成死循环（parentDir() 返回空路径后不再收敛）
    const FilePaths paths = FilePaths::fromStrings({QStringLiteral("abc"), QStringLiteral("abd")});
    QVERIFY(paths.commonPath().isEmpty());
}

void TestFilePath::filePathsSettingsRoundTrip()
{
    const FilePaths paths = FilePaths::fromStrings({QStringLiteral("/a/b"), QStringLiteral("/c/d")});
    const FilePaths restored = FilePaths::fromSettings(paths.toSettings());
    QCOMPARE(restored, paths);
}

void TestFilePath::fileContentsRoundTrip()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const FilePath file = FilePath::fromString(dir.filePath(QStringLiteral("sample.txt")));
    QVERIFY(!file.exists());

    const QByteArray payload = QByteArrayLiteral("hello\nworld\n");
    const auto written = file.writeFileContents(payload);
    QVERIFY2(written.has_value(), qPrintable(errorText(written)));
    QCOMPARE(*written, qint64(payload.size()));

    QVERIFY(file.exists());
    QVERIFY(file.isFile());
    QCOMPARE(file.fileSize(), qint64(payload.size()));

    const auto contents = file.fileContents();
    QVERIFY2(contents.has_value(), qPrintable(errorText(contents)));
    QCOMPARE(*contents, payload);

    // 带偏移与上限的读取按预期截断
    const auto partial = file.fileContents(5, 6);
    QVERIFY2(partial.has_value(), qPrintable(errorText(partial)));
    QCOMPARE(*partial, QByteArrayLiteral("world"));

    const auto removed = file.removeFile();
    QVERIFY2(removed.has_value(), qPrintable(errorText(removed)));
    QVERIFY(!file.exists());
}

void TestFilePath::copyAndRemove()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const FilePath source = FilePath::fromString(dir.filePath(QStringLiteral("a.txt")));
    const auto seeded = source.writeFileContents(QByteArrayLiteral("payload"));
    QVERIFY2(seeded.has_value(), qPrintable(errorText(seeded)));

    const FilePath targetDir = FilePath::fromString(dir.filePath(QStringLiteral("sub")));
    const auto created = targetDir.ensureWritableDir();
    QVERIFY2(created.has_value(), qPrintable(errorText(created)));

    // 目标是目录时，文件按原文件名落进去
    const auto copied = source.copyFile(targetDir);
    QVERIFY2(copied.has_value(), qPrintable(errorText(copied)));

    const FilePath target = targetDir / QStringLiteral("a.txt");
    QVERIFY(target.exists());
    const auto targetContents = target.fileContents();
    QVERIFY2(targetContents.has_value(), qPrintable(errorText(targetContents)));
    QCOMPARE(*targetContents, QByteArrayLiteral("payload"));

    const auto removed = targetDir.removeRecursively();
    QVERIFY2(removed.has_value(), qPrintable(errorText(removed)));
    QVERIFY(!targetDir.exists());
}

QTEST_GUILESS_MAIN(TestFilePath)

#include "tst_filepath.moc"
