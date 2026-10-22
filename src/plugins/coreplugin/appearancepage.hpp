#pragma once

#include <shell/page.hpp>

#include <QWidget>

#include <memory>

namespace Core {

// 外观与语言设置页。类名与页面 id / 标题保持一致（coreplugin.appearance）。
class AppearancePage final : public QWidget, public Shell::IPage
{
    Q_OBJECT

public:
    explicit AppearancePage(QWidget *parent = nullptr);
    ~AppearancePage() override;

    [[nodiscard]] auto id() const -> QString override
    {
        return QStringLiteral("coreplugin.appearance");
    }
    [[nodiscard]] auto title() const -> QString override { return tr("Appearance"); }
    [[nodiscard]] auto subtitle() const -> QString override
    {
        return tr("Theme, language and window effects.");
    }
    [[nodiscard]] auto area() const -> Shell::PageArea override
    {
        return Shell::PageArea::Settings;
    }
    [[nodiscard]] auto order() const -> int override { return Shell::PageOrder::kThird; }
    [[nodiscard]] auto widget() -> QWidget * override { return this; }

private slots:
    void showTestDialog();
    void onThemeChanged();
    // 标题栏也能切主题，这里把下拉框同步回去。
    void onThemeApplied(bool dark);
    void onLanguageChanged();
    void onMaskToggled(Qt::CheckState state);
    void onBlurToggled(Qt::CheckState state);

protected:
    void changeEvent(QEvent *event) override;

private:
    void retranslateUi();
    void setupConnections();

    class AppearancePagePrivate;
    std::unique_ptr<AppearancePagePrivate> d_ptr;
};

} // namespace Core
