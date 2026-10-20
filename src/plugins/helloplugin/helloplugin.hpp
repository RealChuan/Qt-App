#pragma once

#include <extensionsystem/iplugin.hpp>

#include <memory>

class QSettings;

namespace Hello {

class HelloWidget;
class IncrementCommand;

// 最小可用插件模板：
//   1. 头文件里声明插件类 + Q_PLUGIN_METADATA（IID 必须与 extensionsystem/iplugin.hpp 一致）
//   2. initialize() 里只做"取上下文 + 建对象"，不碰界面
//   3. extensionsInitialized() 里把页面和命令注册进 Shell::Registry
class HelloPlugin final : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "Youth.Qt.plugin" FILE "helloplugin.json")

public:
    HelloPlugin();
    ~HelloPlugin() override;

    Utils::Result<> initialize(const ExtensionSystem::PluginContext &context) override;
    void extensionsInitialized() override;

    void increment();

private:
    QSettings *m_settings = nullptr;
    HelloWidget *m_widget = nullptr;
    std::unique_ptr<IncrementCommand> m_command;
};

} // namespace Hello
