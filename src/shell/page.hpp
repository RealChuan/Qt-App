#pragma once

#include "shell_global.hpp"

#include <QIcon>
#include <QString>

class QWidget;

namespace Shell {

// 侧边栏的三个分组。同一个插件可以往多个分组里放页面。
enum class PageArea { Workspace, Settings, Help };

// 一个侧边栏页面。插件实现它，外壳负责按钮、堆栈和切换。
// widget() 返回的 QWidget 由外壳接管父对象，插件保持对 IPage 本身的所有权。
class SHELL_EXPORT IPage
{
public:
    virtual ~IPage() = default;

    // 全局唯一，建议 "<plugin-id>.<page>"。
    virtual auto id() const -> QString = 0;
    // 侧边栏按钮文字，插件里用 tr() 提供。
    virtual auto title() const -> QString = 0;
    virtual auto icon() const -> QIcon { return {}; }
    virtual auto area() const -> PageArea { return PageArea::Workspace; }
    // 同一分组内的排序，小的在前。
    virtual auto order() const -> int { return 100; }
    virtual auto widget() -> QWidget * = 0;
};

} // namespace Shell
