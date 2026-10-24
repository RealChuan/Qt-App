#pragma once

#include <extensionsystem/iplugin.hpp>

#include <memory>

namespace Tetris {

class TetrisPage;

// 第二个游戏插件：模型带 SRS 踢墙与 7-bag，页面跑 60 Hz 帧循环并自己实现 DAS/ARR。
// 与贪吃蛇一致：切走页面不停表，只持久化最高分。
class TetrisPlugin final : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "Youth.Qt.plugin" FILE "tetrisplugin.json")

public:
    TetrisPlugin();
    ~TetrisPlugin() override;

    Utils::Result<> initialize(const ExtensionSystem::PluginContext &context) override;
    void extensionsInitialized() override;

    void play();

private slots:
    void onGameFinished(int score);

private:
    class TetrisPluginPrivate;
    std::unique_ptr<TetrisPluginPrivate> d_ptr;
};

} // namespace Tetris
