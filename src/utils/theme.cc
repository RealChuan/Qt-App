#include "theme.hpp"

#include "singletonmanager.hpp"
#include "widgetmanager.hpp"

#include <QApplication>
#include <QColor>
#include <QEventLoop>
#include <QFile>
#include <QFont>
#include <QGuiApplication>
#include <QPalette>
#include <QStyleHints>

using namespace Qt::StringLiterals;

namespace Utils {

namespace {

// 换了主题后 Qt 会把每个控件按新的样式表重新 polish 一遍，
// Debug 构建下 200 多个控件约 2~3 秒，且只能发生在 GUI 线程。
// 因此这里不做线程化：线程只能省掉“读文件 + 替换令牌”的几毫秒。
// 想缩短耗时，唯一有效的手段是减少规则数（耗时随规则数线性增长）。
auto buildStyleSheet(const ThemeConfig &config, const QHash<QString, QString> &tokens) -> QString
{
    QStringList sheets;
    for (const QString &name : config.styleSheets) {
        QFile file(config.directory + name);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qWarning().noquote() << u"Cannot open the file: %1 (%2)"_s
                                        .arg(file.fileName(), file.errorString());
            continue;
        }

        QString css = QString::fromUtf8(file.readAll());
        for (auto it = tokens.cbegin(); it != tokens.cend(); ++it)
            css.replace(u"@@"_s + it.key() + u"@@"_s, it.value());
        sheets.append(css);
    }
    return sheets.join(u'\n');
}

auto colorOf(const QHash<QString, QString> &tokens, const QString &key) -> QColor
{
    return QColor::fromString(tokens.value(key));
}

// 让没有被 QSS 覆盖的部分（原生菜单、标准对话框等）也保持同一套配色。
auto buildPalette(const QHash<QString, QString> &tokens) -> QPalette
{
    QPalette palette;
    palette.setColor(QPalette::Window, colorOf(tokens, u"window"_s));
    palette.setColor(QPalette::WindowText, colorOf(tokens, u"text"_s));
    palette.setColor(QPalette::Base, colorOf(tokens, u"surface"_s));
    palette.setColor(QPalette::AlternateBase, colorOf(tokens, u"surfaceAlt"_s));
    palette.setColor(QPalette::Text, colorOf(tokens, u"text"_s));
    palette.setColor(QPalette::Button, colorOf(tokens, u"surface"_s));
    palette.setColor(QPalette::ButtonText, colorOf(tokens, u"text"_s));
    palette.setColor(QPalette::BrightText, colorOf(tokens, u"onAccent"_s));
    palette.setColor(QPalette::Highlight, colorOf(tokens, u"accent"_s));
    palette.setColor(QPalette::HighlightedText, colorOf(tokens, u"onAccent"_s));
    palette.setColor(QPalette::ToolTipBase, colorOf(tokens, u"tooltipBg"_s));
    palette.setColor(QPalette::ToolTipText, colorOf(tokens, u"tooltipText"_s));
    palette.setColor(QPalette::PlaceholderText, colorOf(tokens, u"textSubtle"_s));
    palette.setColor(QPalette::Mid, colorOf(tokens, u"border"_s));
    palette.setColor(QPalette::Midlight, colorOf(tokens, u"divider"_s));
    palette.setColor(QPalette::Dark, colorOf(tokens, u"borderStrong"_s));
    palette.setColor(QPalette::Light, colorOf(tokens, u"surface"_s));

    for (const QPalette::ColorRole role :
         {QPalette::WindowText, QPalette::Text, QPalette::ButtonText}) {
        palette.setColor(QPalette::Disabled, role, colorOf(tokens, u"textDisabled"_s));
    }
    palette.setColor(QPalette::Disabled, QPalette::Highlight, colorOf(tokens, u"surfaceSunken"_s));
    palette
        .setColor(QPalette::Disabled, QPalette::HighlightedText, colorOf(tokens, u"textDisabled"_s));
    return palette;
}

// 字体在 C++ 里设，而不是写进 QSS 的 font-family：
// QSS 里只能给单一族名，写死一个平台字体在别的平台会直接落到默认字体。
auto uiFont() -> QFont
{
    QFont font = QGuiApplication::font();
    font.setFamilies({
        u"Segoe UI Variable Text"_s,
        u"Segoe UI"_s,
        u"Inter"_s,
        u"SF Pro Text"_s,
        u"PingFang SC"_s,
        u"Microsoft YaHei UI"_s,
        u"Noto Sans CJK SC"_s,
        u"Noto Sans"_s,
    });
    // 与 base.css 的 QWidget 同字号，否则同窗里大小混排；14px 靠对比度而非放大。
    font.setPixelSize(14);
    return font;
}

} // namespace

class Theme::ThemePrivate
{
public:
    ThemeConfig config;
    ThemeMode mode = ThemeMode::System;
    bool dark = false;
    bool watched = false;
    bool applied = false;
    QString styleSheet;
};

auto systemThemeIsDark() -> bool
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    if (auto *hints = QGuiApplication::styleHints())
        return hints->colorScheme() == Qt::ColorScheme::Dark;
#endif
    return false;
}

Theme::Theme(QObject *parent)
    : QObject(parent)
    , d_ptr(std::make_unique<ThemePrivate>())
{}

Theme::~Theme() = default;

auto Theme::config() const -> ThemeConfig
{
    return d_ptr->config;
}

void Theme::setConfig(const ThemeConfig &config)
{
    d_ptr->config = config;
}

auto Theme::mode() const -> ThemeMode
{
    return d_ptr->mode;
}

auto Theme::isDark() const -> bool
{
    return d_ptr->dark;
}

void Theme::setMode(ThemeMode mode)
{
    if (widgetManager()->themeMode() != mode)
        widgetManager()->setThemeMode(mode);
    apply();
}

void Theme::apply()
{
    if (!qApp)
        return;

    const ThemeMode mode = widgetManager()->themeMode();
    const bool dark = mode == ThemeMode::Dark || (mode == ThemeMode::System && systemThemeIsDark());

    // 令牌按 <light|dark>.theme 的键值对读入，键名全局唯一，[colors] / [icons]
    // 两个分组只是给人看的。不用 QSettings 是因为它读不了 qrc 资源。
    QHash<QString, QString> tokens;
    {
        QFile file(d_ptr->config.directory + (dark ? u"dark.theme"_s : u"light.theme"_s));
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qWarning().noquote() << u"Cannot open the file: %1 (%2)"_s
                                        .arg(file.fileName(), file.errorString());
            return;
        }

        while (!file.atEnd()) {
            const QString line = QString::fromUtf8(file.readLine()).trimmed();
            if (line.isEmpty() || line.startsWith(u';') || line.startsWith(u'#'))
                continue;
            if (line.startsWith(u'['))
                continue;

            const qsizetype separator = line.indexOf(u'=');
            if (separator <= 0)
                continue;
            tokens.insert(line.left(separator).trimmed(), line.mid(separator + 1).trimmed());
        }
    }

    const QString sheet = buildStyleSheet(d_ptr->config, tokens);
    if (sheet.isEmpty()) {
        qWarning("No stylesheet could be loaded, the theme is not applied");
        return;
    }
    if (sheet.contains(u"@@"_s))
        qWarning("Stylesheet still contains unresolved @@token@@ placeholders");

    // 同一种方案重复 apply() 时没必要再让 Qt 全量 polish 一遍。
    // 注意“样式表没变”不等于“状态没变”：System 与 Light 在浅色系统下生成
    // 的样式表完全相同，但模式变了，UI 必须收到通知。
    const bool needRepolish = !d_ptr->applied || sheet != d_ptr->styleSheet;
    if (needRepolish) {
        // 先把等待光标画出来，再做那次耗时几百毫秒到数秒的 polish。
        const bool isSwitch = d_ptr->applied;
        if (isSwitch) {
            QApplication::setOverrideCursor(Qt::WaitCursor);
            qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
        }

        qApp->setStyleSheet(sheet);
        qApp->setPalette(buildPalette(tokens));
        applyFont();

        if (isSwitch)
            QApplication::restoreOverrideCursor();

        d_ptr->styleSheet = sheet;
        d_ptr->applied = true;
    }

    const bool isStateChanged = mode != d_ptr->mode || dark != d_ptr->dark;
    d_ptr->mode = mode;
    d_ptr->dark = dark;

    watchSystemScheme();

    if (needRepolish || isStateChanged)
        emit themeApplied(dark);
}

void Theme::applyFont()
{
    // 字体和主题无关，但 QApplication::setFont 同样会让整棵控件树重新 layout。
    const QFont font = uiFont();
    if (qApp->font() != font)
        qApp->setFont(font);
}

void Theme::watchSystemScheme()
{
    if (d_ptr->watched)
        return;
    d_ptr->watched = true;
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    if (auto *hints = QGuiApplication::styleHints())
        connect(hints, &QStyleHints::colorSchemeChanged, this, &Theme::onSystemColorSchemeChanged);
#endif
}

void Theme::onSystemColorSchemeChanged()
{
    if (widgetManager()->themeMode() == ThemeMode::System)
        apply();
}

} // namespace Utils
