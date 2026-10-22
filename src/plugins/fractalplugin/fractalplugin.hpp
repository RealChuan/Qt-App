#pragma once

#include <extensionsystem/iplugin.hpp>

#include <memory>

namespace Fractal {

class FractalPage;

// 演示"页面自己搞后台计算"：页面内部用 QtConcurrent 渲染，插件只负责注册。
class FractalPlugin final : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "Youth.Qt.plugin" FILE "fractalplugin.json")

public:
    FractalPlugin();
    ~FractalPlugin() override;

    Utils::Result<> initialize(const ExtensionSystem::PluginContext &context) override;
    void extensionsInitialized() override;

    void resetView();

private:
    class FractalPluginPrivate;
    std::unique_ptr<FractalPluginPrivate> d_ptr;
};

} // namespace Fractal
