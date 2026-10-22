#include "functioncommand.hpp"

#include <utility>

namespace Shell {

class FunctionCommand::FunctionCommandPrivate
{
public:
    QString id;
    QString title;
    QString subtitle;
    QKeySequence shortcut;
    std::function<void()> action;
};

FunctionCommand::FunctionCommand(QString id, QString title, std::function<void()> action)
    : d_ptr(
          std::make_unique<FunctionCommandPrivate>(FunctionCommandPrivate{
              .id = std::move(id), .title = std::move(title), .action = std::move(action)}))
{}

FunctionCommand::~FunctionCommand() = default;

auto FunctionCommand::withSubtitle(QString subtitle) -> FunctionCommand &
{
    d_ptr->subtitle = std::move(subtitle);
    return *this;
}

auto FunctionCommand::withShortcut(QKeySequence shortcut) -> FunctionCommand &
{
    d_ptr->shortcut = std::move(shortcut);
    return *this;
}

auto FunctionCommand::id() const -> QString
{
    return d_ptr->id;
}

auto FunctionCommand::title() const -> QString
{
    return d_ptr->title;
}

auto FunctionCommand::subtitle() const -> QString
{
    return d_ptr->subtitle;
}

auto FunctionCommand::shortcut() const -> QKeySequence
{
    return d_ptr->shortcut;
}

void FunctionCommand::execute()
{
    if (d_ptr->action)
        d_ptr->action();
}

} // namespace Shell
