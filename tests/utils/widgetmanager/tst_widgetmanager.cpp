#include <utils/theme.hpp>
#include <utils/widgetmanager.hpp>

#include <QSettings>
#include <QTemporaryDir>
#include <QTest>

using namespace Utils;

namespace {

int modeValue(ThemeMode mode)
{
    return static_cast<int>(mode);
}

} // namespace

// WidgetManager 的取值既写进用户的 QSettings，也会从磁盘读回来。
// 读回来时的默认值与非法值收敛必须稳定：配置文件被手工改坏过一次，
// 界面就会停在一个不存在的主题模式上。
class TestWidgetManager : public QObject
{
    Q_OBJECT

private slots:
    void defaults();
    void saveLoadRoundTrip();
    void invalidStoredModeFallsBackToSystem();
    void emptySettingsKeepDefaults();
};

void TestWidgetManager::defaults()
{
    WidgetManager manager;
    QCOMPARE(manager.showMask(), true);
    QCOMPARE(manager.blurBackground(), true);
    QCOMPARE(modeValue(manager.themeMode()), modeValue(ThemeMode::System));
}

void TestWidgetManager::saveLoadRoundTrip()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QSettings settings(dir.filePath(u"settings.ini"_s), QSettings::IniFormat);

    WidgetManager saved;
    saved.setShowMask(false);
    saved.setBlurBackground(false);
    saved.setThemeMode(ThemeMode::Dark);
    saved.saveSettings(settings);
    settings.sync();

    WidgetManager loaded;
    loaded.loadSettings(settings);
    QCOMPARE(loaded.showMask(), false);
    QCOMPARE(loaded.blurBackground(), false);
    QCOMPARE(modeValue(loaded.themeMode()), modeValue(ThemeMode::Dark));
}

void TestWidgetManager::invalidStoredModeFallsBackToSystem()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QSettings settings(dir.filePath(u"settings.ini"_s), QSettings::IniFormat);

    // 超出枚举范围的数字必须收敛到 System，而不是原样 static_cast
    settings.setValue(u"Widget/themeMode"_s, 42);
    settings.sync();

    WidgetManager loaded;
    loaded.loadSettings(settings);
    QCOMPARE(modeValue(loaded.themeMode()), modeValue(ThemeMode::System));
}

void TestWidgetManager::emptySettingsKeepDefaults()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QSettings settings(dir.filePath(u"settings.ini"_s), QSettings::IniFormat);

    WidgetManager loaded;
    loaded.loadSettings(settings);
    QCOMPARE(loaded.showMask(), true);
    QCOMPARE(loaded.blurBackground(), true);
    QCOMPARE(modeValue(loaded.themeMode()), modeValue(ThemeMode::System));
}

QTEST_GUILESS_MAIN(TestWidgetManager)

#include "tst_widgetmanager.moc"
