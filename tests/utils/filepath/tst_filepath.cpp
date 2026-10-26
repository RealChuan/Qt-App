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
    QCOMPARE(FilePath::fromString(u"a\\b\\c"_s).path(), u"a/b/c"_s);
    QCOMPARE(FilePath::fromString(u"a//b///c"_s).path(), u"a/b/c"_s);
}

void TestFilePath::fromStringCollapsesDotSegments()
{
    QCOMPARE(FilePath::fromString(u"a/b/../c"_s).path(), u"a/c"_s);
    QCOMPARE(FilePath::fromString(u"./a/b"_s).path(), u"a/b"_s);
    QCOMPARE(FilePath::fromString(u"a/./b"_s).path(), u"a/b"_s);
}

void TestFilePath::pathAppended()
{
    // 尾部斜杠不能导致出现双斜杠
    QCOMPARE((FilePath(u"/a/b/"_s) / u"c"_s).path(), u"/a/b/c"_s);
    QCOMPARE((FilePath(u"/a/b"_s) / u"c/d"_s).path(), u"/a/b/c/d"_s);
    // 传入反斜杠也按分隔符处理
    QCOMPARE((FilePath(u"/a"_s) / u"b\\c"_s).path(), u"/a/b/c"_s);
    // 空路径作为基底：结果就是参数本身
    QCOMPARE((FilePath() / u"a/b"_s).path(), u"a/b"_s);
    // 参数里的 ".." 会被清理
    QCOMPARE((FilePath(u"/a/b"_s) / u"../c"_s).path(), u"/a/c"_s);
}

void TestFilePath::pathAppendedKeepsEmptyArgument()
{
    const FilePath base(u"/a/b"_s);
    QCOMPARE((base / QString()).path(), base.path());
}

void TestFilePath::pathAppendedReplacesOnAbsoluteArgument()
{
    QCOMPARE((FilePath(u"/a/b"_s) / u"/c/d"_s).path(), u"/c/d"_s);
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
        toStringList(FilePath(u"/a/b/c"_s).pathComponents()), QStringList({u"a"_s, u"b"_s, u"c"_s}));
    // 空段（连续斜杠、根斜杠）不产生组件
    QCOMPARE(toStringList(FilePath(u"/a//b"_s).pathComponents()), QStringList({u"a"_s, u"b"_s}));
    QVERIFY(FilePath(u"/"_s).pathComponents().isEmpty());
    QVERIFY(FilePath().pathComponents().isEmpty());
}

void TestFilePath::fileNameWithPathComponents()
{
    const FilePath path(u"/a/b/c.txt"_s);
    QCOMPARE(path.fileNameWithPathComponents(1), u"c.txt"_s);
    QCOMPARE(path.fileNameWithPathComponents(2), u"b/c.txt"_s);
    QCOMPARE(path.fileNameWithPathComponents(3), u"a/b/c.txt"_s);
    // 越界与非正数都退化为纯文件名
    QCOMPARE(path.fileNameWithPathComponents(0), u"c.txt"_s);
    QCOMPARE(path.fileNameWithPathComponents(-1), u"c.txt"_s);
    QCOMPARE(path.fileNameWithPathComponents(9), u"c.txt"_s);
}

void TestFilePath::fileNameParts()
{
    const FilePath path(u"/a/b.tar.gz"_s);
    QCOMPARE(path.fileName(), u"b.tar.gz"_s);
    QCOMPARE(path.baseName(), u"b"_s);
    QCOMPARE(path.completeBaseName(), u"b.tar"_s);
    QCOMPARE(path.suffix(), u"tar.gz"_s);
    QCOMPARE(path.completeSuffix(), u"gz"_s);

    const FilePath noSuffix(u"/a/noext"_s);
    QCOMPARE(noSuffix.baseName(), u"noext"_s);
    QCOMPARE(noSuffix.completeBaseName(), u"noext"_s);
    QVERIFY(noSuffix.suffix().isEmpty());
    QVERIFY(noSuffix.completeSuffix().isEmpty());
}

void TestFilePath::parentDir_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("expected");

    QTest::newRow("nested") << u"/a/b"_s << u"/a"_s;
    QTest::newRow("one level below root") << u"/a"_s << u"/"_s;
    QTest::newRow("root") << u"/"_s << u"/"_s;
    QTest::newRow("no separator") << u"abc"_s << QString();
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

    QTest::newRow("direct child") << u"/a/b/c"_s << u"/a/b"_s << true;
    QTest::newRow("grand child") << u"/a/b/c"_s << u"/a"_s << true;
    QTest::newRow("same path") << u"/a/b"_s << u"/a/b"_s << false;
    // 前缀相同但下一字符不是分隔符，不能算子路径
    QTest::newRow("sibling with shared prefix") << u"/a/bc"_s << u"/a/b"_s << false;
    QTest::newRow("parent with trailing slash") << u"/a/b/c"_s << u"/a/b/"_s << true;
    QTest::newRow("empty parent") << u"/a/b"_s << QString() << false;
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
    QCOMPARE(FilePath(u"/a/B/c"_s).isChildOf(FilePath(u"/a/b"_s)), expected);
}

void TestFilePath::relativeChildPath()
{
    QCOMPARE(FilePath(u"/a/b/c"_s).relativeChildPath(FilePath(u"/a/b"_s)).path(), u"c"_s);
    QCOMPARE(FilePath(u"/a/b/c/d"_s).relativeChildPath(FilePath(u"/a"_s)).path(), u"b/c/d"_s);
    // 不是子路径时返回空，而不是抛异常
    QVERIFY(FilePath(u"/x/y"_s).relativeChildPath(FilePath(u"/a"_s)).isEmpty());
}

void TestFilePath::withSuffixAppends()
{
    // 语义是「追加」而不是「替换」，容易被误用，这里锁住
    QCOMPARE(FilePath(u"/a/b.txt"_s).withSuffix(u".bak"_s).path(), u"/a/b.txt.bak"_s);
}

void TestFilePath::resourcePathRoundTrip()
{
    const FilePath resource(u":/qss/base.css"_s);
    QVERIFY(resource.isResourceFile());
    QVERIFY(resource.isAbsolutePath());
    QCOMPARE(resource.toUrl(), QUrl(u"qrc:/qss/base.css"_s));

    const FilePath local = FilePath::fromUrl(QUrl::fromLocalFile(u"/a/b c.css"_s));
    QCOMPARE(local.path(), u"/a/b c.css"_s);
    QVERIFY(!local.isResourceFile());
}

void TestFilePath::equalityAndOrdering()
{
    const FilePath first(u"/a/b"_s);
    const FilePath same(u"/a/b"_s);
    const FilePath other(u"/a/c"_s);

    QVERIFY(first == same);
    QVERIFY(first != other);
    QVERIFY(first < other);

    FilePaths paths = FilePaths::fromStrings({u"/b"_s, u"/a"_s, u"/c"_s});
    paths.sort();
    QCOMPARE(paths.toFsPathStrings(), QStringList({u"/a"_s, u"/b"_s, u"/c"_s}));
}

void TestFilePath::filePathsCommonPath()
{
    const FilePaths paths = FilePaths::fromStrings({u"/a/b/c"_s, u"/a/b/d"_s, u"/a/e"_s});
    QCOMPARE(paths.commonPath().path(), u"/a"_s);

    const FilePaths single = FilePaths::fromStrings({u"/a/b"_s});
    QCOMPARE(single.commonPath().path(), u"/a/b"_s);
}

void TestFilePath::filePathsCommonPathWithUnrelatedRelativePaths()
{
    // 回归：相对路径且首段不同时曾退化成死循环（parentDir() 返回空路径后不再收敛）
    const FilePaths paths = FilePaths::fromStrings({u"abc"_s, u"abd"_s});
    QVERIFY(paths.commonPath().isEmpty());
}

void TestFilePath::filePathsSettingsRoundTrip()
{
    const FilePaths paths = FilePaths::fromStrings({u"/a/b"_s, u"/c/d"_s});
    const FilePaths restored = FilePaths::fromSettings(paths.toSettings());
    QCOMPARE(restored, paths);
}

void TestFilePath::fileContentsRoundTrip()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const FilePath file = FilePath::fromString(dir.filePath(u"sample.txt"_s));
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

    const FilePath source = FilePath::fromString(dir.filePath(u"a.txt"_s));
    const auto seeded = source.writeFileContents(QByteArrayLiteral("payload"));
    QVERIFY2(seeded.has_value(), qPrintable(errorText(seeded)));

    const FilePath targetDir = FilePath::fromString(dir.filePath(u"sub"_s));
    const auto created = targetDir.ensureWritableDir();
    QVERIFY2(created.has_value(), qPrintable(errorText(created)));

    // 目标是目录时，文件按原文件名落进去
    const auto copied = source.copyFile(targetDir);
    QVERIFY2(copied.has_value(), qPrintable(errorText(copied)));

    const FilePath target = targetDir / u"a.txt"_s;
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
