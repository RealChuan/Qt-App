#pragma once

#include "snakegame.hpp"

#include <extensionsystem/iplugin.hpp>

#include <memory>

namespace Snake {

class SnakePage;

// 完整小游戏示例：固定步进的游戏循环、键盘焦点归属、难度与最高分落插件自己的 QSettings。
// 暂停策略见 SnakePage：页面切走不停表，所以这里也不需要处理隐藏事件。
class SnakePlugin final : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "Youth.Qt.plugin" FILE "snakeplugin.json")

public:
    SnakePlugin();
    ~SnakePlugin() override;

    Utils::Result<> initialize(const ExtensionSystem::PluginContext &context) override;
    void extensionsInitialized() override;

    void play();

private slots:
    void onGameFinished(int score);
    void onDifficultyChanged(Snake::Difficulty difficulty);

private:
    class SnakePluginPrivate;
    std::unique_ptr<SnakePluginPrivate> d_ptr;
};

} // namespace Snake
