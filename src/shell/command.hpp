#pragma once

#include "shell_global.hpp"

#include <QKeySequence>
#include <QMetaType>
#include <QString>

namespace Shell {

// 命令面板（Ctrl+K）里的一条命令。插件把用户可触发的动作都注册成命令，
// 这样不需要为每个功能再造一个按钮。
class SHELL_EXPORT ICommand
{
public:
    virtual ~ICommand() = default;

    [[nodiscard]] virtual auto id() const -> QString = 0;
    [[nodiscard]] virtual auto title() const -> QString = 0;
    [[nodiscard]] virtual auto subtitle() const -> QString { return {}; }
    [[nodiscard]] virtual auto shortcut() const -> QKeySequence { return {}; }
    [[nodiscard]] virtual auto isEnabled() const -> bool { return true; }
    virtual void execute() = 0;
};

} // namespace Shell

// 命令对象由插件持有、注册表只借用指针；允许它进 QVariant 后，
// 命令面板不必再把指针强转成整数存进 UserRole。
Q_DECLARE_METATYPE(Shell::ICommand *)
