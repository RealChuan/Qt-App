#pragma once

#include "extensionsystem_global.h"

#include <utils/filepath.h>

#include <QAnyStringView>
#include <QString>

class QSettings;

namespace ExtensionSystem {

class PluginSpec;

// 每个插件在 initialize() 里拿到的上下文。
// 插件不应该自己构造它，也不应该把它缓存超过本插件生命周期。
class EXTENSIONSYSTEM_EXPORT PluginContext
{
public:
    PluginContext() = default;
    PluginContext(const PluginSpec *spec, QSettings *settings, Utils::FilePath resourceRoot);

    [[nodiscard]] auto spec() const -> const PluginSpec &;
    [[nodiscard]] auto id() const -> QString;

    // 该插件专属的 QSettings（已按插件 id 分组），可直接读写。
    [[nodiscard]] auto settings() const -> QSettings *;

    // <安装目录>/resources/<relative>
    [[nodiscard]] auto resourcePath(QAnyStringView relative) const -> Utils::FilePath;

private:
    const PluginSpec *m_spec = nullptr;
    QSettings *m_settings = nullptr;
    Utils::FilePath m_resourceRoot;
};

} // namespace ExtensionSystem
