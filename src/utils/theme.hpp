#pragma once

#include "utils_global.hpp"

#include <QHash>
#include <QObject>
#include <QString>
#include <QStringList>

#include <memory>

using namespace Qt::StringLiterals;

namespace Utils {

// 主题模式。System 表示跟随操作系统的深色/浅色设置。
enum class ThemeMode { System = 0, Light = 1, Dark = 2 };

// 读取系统当前是否为深色。Qt 6.5 起由平台主题提供该信息。
[[nodiscard]] UTILS_EXPORT auto systemThemeIsDark() -> bool;

// 样式表来源：CSS 里的令牌占位符由 <light|dark>.theme 提供的值替换。
//
// 可移植性：把 qss 目录整份复制到别的 Qt 程序，配置一下这两个字段即可，
// 自己的样式表接在 base.css 后面，同样可以使用这些令牌：
//     Utils::theme()->setConfig({":/qss/", {"base.css", "app.css"}});
//     Utils::theme()->apply();
struct ThemeConfig
{
    // 令牌文件与样式表所在目录，必须以 '/' 结尾。
    QString directory = u":/qss/"_s;
    // 依次拼接，越靠后的规则优先级越高。
    QStringList styleSheets{u"base.css"_s, u"shell.css"_s, u"pages.css"_s};
};

// 为什么需要这一层：QSS 一旦声明了颜色就会覆盖 QPalette，
// 系统切换深浅色时不会自动跟进。这里按当前方案重新生成一次样式表。
class UTILS_EXPORT Theme : public QObject
{
    Q_OBJECT
public:
    explicit Theme(QObject *parent = nullptr);
    ~Theme() override;

    void setConfig(const ThemeConfig &config);
    [[nodiscard]] auto config() const -> ThemeConfig;

    // 按当前模式重新生成并应用样式表、调色板与基础字体。可重复调用。
    void apply();

    void setMode(ThemeMode mode);
    [[nodiscard]] auto mode() const -> ThemeMode;
    [[nodiscard]] auto isDark() const -> bool;

signals:
    void themeApplied(bool dark);

private slots:
    // 系统的深浅色变化：只在“跟随系统”时生效。
    void onSystemColorSchemeChanged();

private:
    void applyPalette(const QHash<QString, QString> &tokens);
    void applyFont();
    void watchSystemScheme();

    class ThemePrivate;
    std::unique_ptr<ThemePrivate> d_ptr;
};

} // namespace Utils
