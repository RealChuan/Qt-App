#pragma once

#include <QWidget>

#include <memory>

namespace ExtensionSystem {
class PluginSpec;
}

namespace Core {

// 插件详情：元数据、依赖、描述；插件加载失败时额外显示错误详情。
class PluginDetailsView final : public QWidget
{
    Q_OBJECT

public:
    explicit PluginDetailsView(QWidget *parent = nullptr);
    ~PluginDetailsView() override;

    void setSpec(ExtensionSystem::PluginSpec *spec);

protected:
    void changeEvent(QEvent *event) override;

private:
    void retranslateUi();
    void updateContents();
    void setFieldLabel(QWidget *field, const QString &text);

    class PluginDetailsViewPrivate;
    std::unique_ptr<PluginDetailsViewPrivate> d_ptr;
};

} // namespace Core
