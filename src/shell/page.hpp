#pragma once

#include "shell_global.hpp"

#include <QIcon>
#include <QString>

class QWidget;

namespace Shell {

// 侧边栏的三个分组。同一个插件可以往多个分组里放页面。
enum class PageArea { Workspace, Settings, Help };

// 同组内的排序权重：小的在前。档位之间留 100 的间隔，插件插队时不必改动别人。
namespace PageOrder {
constexpr int kFirst = 100;
constexpr int kSecond = 200;
constexpr int kThird = 300;
constexpr int kFourth = 400;
constexpr int kFifth = 500;
constexpr int kDefault = 1000;
} // namespace PageOrder

// 一个侧边栏页面。插件实现它，外壳负责按钮、堆栈和切换。
// widget() 返回的 QWidget 由外壳接管父对象，插件保持对 IPage 本身的所有权。
class SHELL_EXPORT IPage
{
public:
    virtual ~IPage() = default;

    // 全局唯一，建议 "<plugin-id>.<page>"。
    [[nodiscard]] virtual auto id() const -> QString = 0;
    // 侧边栏按钮文字，插件里用 tr() 提供。
    [[nodiscard]] virtual auto title() const -> QString = 0;
    // 页面标题下的一行说明，可为空；由外壳统一排版，插件不必自己画标题。
    [[nodiscard]] virtual auto subtitle() const -> QString { return {}; }
    [[nodiscard]] virtual auto icon() const -> QIcon { return {}; }
    [[nodiscard]] virtual auto area() const -> PageArea { return PageArea::Workspace; }
    // 同一分组内的排序，小的在前。
    [[nodiscard]] virtual auto order() const -> int { return PageOrder::kDefault; }
    [[nodiscard]] virtual auto widget() -> QWidget * = 0;
};

} // namespace Shell
