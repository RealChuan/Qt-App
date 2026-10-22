#include "registry.hpp"

#include "command.hpp"
#include "page.hpp"

#include <QLoggingCategory>

#include <algorithm>

Q_LOGGING_CATEGORY(lcShell, "qtapp.shell", QtInfoMsg)

namespace Shell {

class Registry::RegistryPrivate
{
public:
    QList<IPage *> pages;
    QList<ICommand *> commands;
};

Registry::Registry(QObject *parent)
    : QObject(parent)
    , d_ptr(std::make_unique<RegistryPrivate>())
{}

Registry::~Registry() = default;

auto Registry::instance() -> Registry *
{
    // 故意不析构：插件在静态期与退出期都可能访问它，进程结束时交给系统回收最安全。
    static Registry *registry = new Registry;
    return registry;
}

void Registry::addPage(IPage *page)
{
    if (!page)
        return;

    const auto hasSameId = [page](const IPage *candidate) { return candidate->id() == page->id(); };
    if (std::ranges::any_of(d_ptr->pages, hasSameId)) {
        qCWarning(lcShell) << "duplicate page id, ignored:" << page->id();
        return;
    }

    d_ptr->pages.append(page);
    emit pagesChanged();
}

void Registry::addCommand(ICommand *command)
{
    if (!command)
        return;

    const auto hasSameId = [command](const ICommand *candidate) {
        return candidate->id() == command->id();
    };
    if (std::ranges::any_of(d_ptr->commands, hasSameId)) {
        qCWarning(lcShell) << "duplicate command id, ignored:" << command->id();
        return;
    }

    d_ptr->commands.append(command);
    emit commandsChanged();
}

auto Registry::pages() const -> QList<IPage *>
{
    return d_ptr->pages;
}

auto Registry::commands() const -> QList<ICommand *>
{
    return d_ptr->commands;
}

} // namespace Shell
