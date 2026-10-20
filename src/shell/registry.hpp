#pragma once

#include "shell_global.hpp"

#include <QList>
#include <QObject>

namespace Shell {

class ICommand;
class IPage;

// 页面与命令的登记处。插件在 extensionsInitialized() 里注册，
// 外壳监听变更信号增量刷新界面。
// 注意：不提供 remove：插件运行期不卸载，生命周期与进程一致。
class SHELL_EXPORT Registry : public QObject
{
    Q_OBJECT

public:
    [[nodiscard]] static auto instance() -> Registry *;

    void addPage(IPage *page);
    void addCommand(ICommand *command);

    [[nodiscard]] auto pages() const -> QList<IPage *>;
    [[nodiscard]] auto commands() const -> QList<ICommand *>;

signals:
    void pagesChanged();
    void commandsChanged();

private:
    explicit Registry(QObject *parent = nullptr);

    QList<IPage *> m_pages;
    QList<ICommand *> m_commands;
};

} // namespace Shell
