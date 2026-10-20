#pragma once

#include "shell_global.hpp"

#include <QKeySequence>
#include <QString>

namespace Shell {

// 命令面板（Ctrl+K）里的一条命令。插件把用户可触发的动作都注册成命令，
// 这样不需要为每个功能再造一个按钮。
class SHELL_EXPORT ICommand
{
public:
    virtual ~ICommand() = default;

    virtual auto id() const -> QString = 0;
    virtual auto title() const -> QString = 0;
    virtual auto subtitle() const -> QString { return {}; }
    virtual auto shortcut() const -> QKeySequence { return {}; }
    virtual auto isEnabled() const -> bool { return true; }
    virtual void execute() = 0;
};

} // namespace Shell
