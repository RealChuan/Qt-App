#include "languagemanager.hpp"
#include "appinfo.hpp"

#include <QCoreApplication>
#include <QDebug>
#include <QLibraryInfo>
#include <QLocale>
#include <QSettings>
#include <QTranslator>

#include <array>
#include <memory>
#include <optional>
#include <utility>

using namespace Qt::StringLiterals;

namespace Utils {

namespace {

[[nodiscard]] auto translationSuffix(LanguageManager::Language language) -> QString
{
    switch (language) {
    case LanguageManager::Chinese:
        return u"zh_CN"_s;
    case LanguageManager::English:
        break;
    }
    return u"en"_s;
}

[[nodiscard]] auto translationsPath() -> QString
{
#ifdef Q_OS_MACOS
    // macOS 下 qm 随 App Bundle 分发，位于 Contents/Resources/translations。
    return QCoreApplication::applicationDirPath() + u"/../Resources/translations"_s;
#else
    return QCoreApplication::applicationDirPath() + u"/translations"_s;
#endif
}

// 配置中存的是枚举整数，手工改坏或版本回退时可能是越界值。
[[nodiscard]] constexpr auto languageFromInt(int value) -> std::optional<LanguageManager::Language>
{
    const auto language = static_cast<LanguageManager::Language>(value);
    switch (language) {
    case LanguageManager::Chinese:
    case LanguageManager::English:
        return language;
    }
    return std::nullopt;
}

[[nodiscard]] auto systemLanguage() -> LanguageManager::Language
{
    // 默认语言跟随系统 locale，未覆盖的语言回落到源语言英文。
    return QLocale::system().language() == QLocale::Chinese ? LanguageManager::Chinese
                                                            : LanguageManager::English;
}

} // namespace

class LanguageManager::LanguageManagerPrivate
{
public:
    LanguageManager::Language currentLanguage = LanguageManager::Chinese;
    std::unique_ptr<QTranslator> appTranslator;
    std::unique_ptr<QTranslator> qtTranslator;
};

LanguageManager::LanguageManager(QObject *parent)
    : QObject(parent)
    , d_ptr(std::make_unique<LanguageManagerPrivate>())
{}

LanguageManager::~LanguageManager() = default;

auto LanguageManager::currentLanguage() const -> Language
{
    return d_ptr->currentLanguage;
}

void LanguageManager::loadLanguage(Language language)
{
    d_ptr->currentLanguage = language;
    loadLanguage();
}

void LanguageManager::loadLanguage()
{
    // 移除已安装的翻译器，避免重复切换时叠加多个 QTranslator。
    if (d_ptr->appTranslator) {
        QCoreApplication::removeTranslator(d_ptr->appTranslator.get());
    }
    if (d_ptr->qtTranslator) {
        QCoreApplication::removeTranslator(d_ptr->qtTranslator.get());
    }

    d_ptr->appTranslator = std::make_unique<QTranslator>();
    d_ptr->qtTranslator = std::make_unique<QTranslator>();

    // 英文是源语言，Qt 与应用程序的原始字符串均为英文，无需安装翻译器。
    if (d_ptr->currentLanguage == English) {
        return;
    }

    const auto suffix = translationSuffix(d_ptr->currentLanguage);
    const auto directory = translationsPath();

    // 先安装 Qt 的翻译，再安装应用自身的翻译：后安装的翻译器优先级更高。
    const std::array qtDirectories{directory, QLibraryInfo::path(QLibraryInfo::TranslationsPath)};
    bool qtLoaded = false;
    for (const auto &qtDirectory : qtDirectories) {
        if (qtDirectory.isEmpty()) {
            continue;
        }
        // Qt 6 自带翻译以 qtbase_ 为前缀，与 Qt 5 的 qt_ 命名不同。
        if (d_ptr->qtTranslator->load(u"%1/qtbase_%2.qm"_s.arg(qtDirectory, suffix))) {
            QCoreApplication::installTranslator(d_ptr->qtTranslator.get());
            qtLoaded = true;
            break;
        }
    }
    if (!qtLoaded) {
        qWarning() << "LanguageManager: failed to load Qt translation for" << suffix;
    }

    const auto appFile
        = u"%1/%2_%3.qm"_s.arg(directory, QString::fromLatin1(Utils::appName).toLower(), suffix);
    if (d_ptr->appTranslator->load(appFile)) {
        QCoreApplication::installTranslator(d_ptr->appTranslator.get());
    } else {
        qWarning() << "LanguageManager: failed to load" << appFile;
    }
}

void LanguageManager::saveSettings(QSettings &settings) const
{
    settings.beginGroup(u"Language"_s);
    settings.setValue(u"Language"_s, std::to_underlying(d_ptr->currentLanguage));
    settings.endGroup();
}

void LanguageManager::loadSettings(QSettings &settings)
{
    const auto fallback = systemLanguage();

    settings.beginGroup(u"Language"_s);
    const int stored = settings.value(u"Language"_s, std::to_underlying(fallback)).toInt();
    settings.endGroup();

    d_ptr->currentLanguage = languageFromInt(stored).value_or(fallback);
}

} // namespace Utils
