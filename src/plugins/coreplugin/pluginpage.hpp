#pragma once

#include <shell/page.hpp>

#include <QWidget>

#include <memory>

using namespace Qt::StringLiterals;

namespace ExtensionSystem {
class PluginSpec;
}

class QLineEdit;
class QSplitter;

namespace Core {

class PluginDetailsView;
class PluginView;

// 插件管理页：分组树 + 搜索 + 详情面板；启用/禁用需要重启后生效。
class PluginPage final : public QWidget, public Shell::IPage
{
    Q_OBJECT

public:
    explicit PluginPage(QWidget *parent = nullptr);
    ~PluginPage() override;

    [[nodiscard]] auto id() const -> QString override { return u"coreplugin.plugins"_s; }
    [[nodiscard]] auto title() const -> QString override { return tr("Plugins"); }
    [[nodiscard]] auto subtitle() const -> QString override
    {
        return tr("Enable or disable plugins and inspect what each one provides.");
    }
    [[nodiscard]] auto area() const -> Shell::PageArea override
    {
        return Shell::PageArea::Settings;
    }
    [[nodiscard]] auto order() const -> int override { return Shell::PageOrder::kFirst; }
    [[nodiscard]] auto widget() -> QWidget * override { return this; }

private slots:
    void onFilterChanged(const QString &text);
    void onCurrentPluginChanged(ExtensionSystem::PluginSpec *spec);
    void onPluginsChanged();

protected:
    void changeEvent(QEvent *event) override;

private:
    void retranslateUi();
    void setupConnections();

    class PluginPagePrivate;
    std::unique_ptr<PluginPagePrivate> d_ptr;
};

} // namespace Core
