#pragma once

#include "shell_global.hpp"

#include <QAnyStringView>

class QSettings;
class QWidget;

namespace Shell {

// 外壳提供给插件的能力。coreplugin 实现它并调用 setHost() 注册。
class SHELL_EXPORT Host
{
public:
    virtual ~Host() = default;

    [[nodiscard]] virtual auto mainWindow() -> QWidget * = 0;
    virtual void showPage(QAnyStringView pageId) = 0;
    virtual void showStatusMessage(QAnyStringView text, int timeoutMs = 3000) = 0;
    virtual auto askForRestart(QAnyStringView reason) -> bool = 0;
};

SHELL_EXPORT void setHost(Host *host);
[[nodiscard]] SHELL_EXPORT auto host() -> Host *;

// 应用级设置（转发 PluginManager::settings()）。
[[nodiscard]] SHELL_EXPORT auto settings() -> QSettings *;

} // namespace Shell
