#pragma once

#include "command.hpp"

#include <functional>
#include <memory>

namespace Shell {

// 最常见的一类命令：只触发一个动作。用 std::function 包装，插件就不必为每条
// 命令再写一个只有委托的类；需要动态标题/可用状态时仍然可以直接实现 ICommand。
class SHELL_EXPORT FunctionCommand final : public ICommand
{
public:
    FunctionCommand(QString id, QString title, std::function<void()> action);
    ~FunctionCommand() override;

    FunctionCommand(const FunctionCommand &) = delete;
    auto operator=(const FunctionCommand &) -> FunctionCommand & = delete;

    // 可选字段用链式调用补，构造点只留必填项
    auto withSubtitle(QString subtitle) -> FunctionCommand &;
    auto withShortcut(QKeySequence shortcut) -> FunctionCommand &;

    [[nodiscard]] auto id() const -> QString override;
    [[nodiscard]] auto title() const -> QString override;
    [[nodiscard]] auto subtitle() const -> QString override;
    [[nodiscard]] auto shortcut() const -> QKeySequence override;
    void execute() override;

private:
    class FunctionCommandPrivate;
    std::unique_ptr<FunctionCommandPrivate> d_ptr;
};

} // namespace Shell
