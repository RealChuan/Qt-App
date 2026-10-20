#include "registry.hpp"

#include "command.hpp"
#include "page.hpp"

#include <QLoggingCategory>

Q_LOGGING_CATEGORY(lcShell, "qtapp.shell", QtInfoMsg)

namespace Shell {

Registry::Registry(QObject *parent)
    : QObject(parent)
{}

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

    const QList<IPage *> existing = pages();
    for (const IPage *candidate : existing) {
        if (candidate->id() == page->id()) {
            qCWarning(lcShell) << "duplicate page id, ignored:" << page->id();
            return;
        }
    }

    m_pages.append(page);
    emit pagesChanged();
}

void Registry::addCommand(ICommand *command)
{
    if (!command)
        return;

    const QList<ICommand *> existing = commands();
    for (const ICommand *candidate : existing) {
        if (candidate->id() == command->id()) {
            qCWarning(lcShell) << "duplicate command id, ignored:" << command->id();
            return;
        }
    }

    m_commands.append(command);
    emit commandsChanged();
}

auto Registry::pages() const -> QList<IPage *>
{
    return m_pages;
}

auto Registry::commands() const -> QList<ICommand *>
{
    return m_commands;
}

} // namespace Shell
