#pragma once

#include <shell/page.hpp>

#include <QWidget>

#include <memory>

namespace Core {

// 插件管理页：列出所有插件、状态与加载耗时，允许启用/禁用（需要重启）。
class PluginPage final : public QWidget, public Shell::IPage
{
    Q_OBJECT

public:
    explicit PluginPage(QWidget *parent = nullptr);
    ~PluginPage() override;

    auto id() const -> QString override { return QStringLiteral("coreplugin.plugins"); }
    auto title() const -> QString override { return tr("Plugins"); }
    auto area() const -> Shell::PageArea override { return Shell::PageArea::Settings; }
    auto order() const -> int override { return 10; }
    auto widget() -> QWidget * override { return this; }

private:
    class PluginPagePrivate;
    std::unique_ptr<PluginPagePrivate> d_ptr;
};

} // namespace Core
