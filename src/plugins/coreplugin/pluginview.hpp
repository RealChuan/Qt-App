#pragma once

#include <QModelIndex>
#include <QWidget>

#include <memory>

namespace ExtensionSystem {
class PluginSpec;
}

namespace Core {

// 插件列表：按 Category 分组（必需插件单独成组），支持搜索过滤与启用勾选。
// 结构对应 Qt Creator 的 PluginView，只保留本仓库 PluginSpec 具备的字段。
class PluginView final : public QWidget
{
    Q_OBJECT

public:
    explicit PluginView(QWidget *parent = nullptr);
    ~PluginView() override;

    [[nodiscard]] auto currentPlugin() const -> ExtensionSystem::PluginSpec *;
    void setFilter(const QString &text);
    void rebuild();

signals:
    void currentPluginChanged(ExtensionSystem::PluginSpec *spec);
    void pluginsChanged();

private slots:
    void onCurrentChanged(const QModelIndex &current);
    void onDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    void notifyPluginsChanged();

protected:
    void changeEvent(QEvent *event) override;

private:
    void retranslateUi();
    void setupConnections();

    class PluginViewPrivate;
    std::unique_ptr<PluginViewPrivate> d_ptr;
};

} // namespace Core
