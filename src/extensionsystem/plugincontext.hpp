#pragma once

#include "extensionsystem_global.hpp"

#include <utils/filepath.hpp>

#include <QAnyStringView>
#include <QString>

#include <memory>

class QSettings;

namespace ExtensionSystem {

class PluginSpec;

// 每个插件在 initialize() 里拿到的上下文。
// 插件不应该自己构造它，也不应该把它缓存超过本插件生命周期。
class EXTENSIONSYSTEM_EXPORT PluginContext
{
public:
    PluginContext();
    PluginContext(const PluginSpec *spec, QSettings *settings, Utils::FilePath resourceRoot);
    ~PluginContext();

    Q_DISABLE_COPY_MOVE(PluginContext)

    [[nodiscard]] auto spec() const -> const PluginSpec &;
    [[nodiscard]] auto id() const -> QString;

    // 该插件专属的 QSettings（已按插件 id 分组），可直接读写。
    [[nodiscard]] auto settings() const -> QSettings *;

    // <安装目录>/resources/<relative>
    [[nodiscard]] auto resourcePath(QAnyStringView relative) const -> Utils::FilePath;

private:
    class PluginContextPrivate;
    std::unique_ptr<PluginContextPrivate> d_ptr;
};

} // namespace ExtensionSystem
