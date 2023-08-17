#include "toolbutton.hpp"

#include <QEvent>

namespace GUI {

class ToolButton::ToolButtonPrivate
{
public:
    explicit ToolButtonPrivate(ToolButton *q)
        : q_ptr(q)
    {}

    ToolButton *q_ptr;

    QIcon icon;
    QIcon hoverIcon;
    QIcon activeIcon;
};

ToolButton::ToolButton(QWidget *parent)
    : QToolButton(parent)
    , d_ptr(new ToolButtonPrivate(this))
{
    installEventFilter(this);
}

ToolButton::~ToolButton() = default;

void ToolButton::setNormalIcon(const QIcon &icon)
{
    d_ptr->icon = icon;
    setIcon(icon);
}

void ToolButton::setHoverIcon(const QIcon &icon)
{
    d_ptr->hoverIcon = icon;
}

void ToolButton::setActiveIcon(const QIcon &icon)
{
    d_ptr->activeIcon = icon;
}

auto ToolButton::eventFilter(QObject *watched, QEvent *event) -> bool
{
    if (watched == this) {
        switch (event->type()) {
        case QEvent::Enter: setIcon(d_ptr->hoverIcon); break;
        case QEvent::MouseButtonPress: setIcon(d_ptr->activeIcon); break;
        case QEvent::Leave: setIcon(d_ptr->icon); break;
        default: break;
        }
    }

    return QToolButton::eventFilter(watched, event);
}

auto fromPaths(const QStringList &Paths) -> QIcon
{
    QIcon icon;
    for (const auto &path : qAsConst(Paths)) {
        icon.addFile(path);
    }
    return icon;
}

auto createToolButton(const QStringList &normalIconPaths,
                      const QStringList &hoverIconPaths,
                      const QStringList &activeIconPaths,
                      QWidget *parent) -> ToolButton *
{
    auto *btn = new ToolButton(parent);
    btn->setNormalIcon(fromPaths(normalIconPaths));
    btn->setHoverIcon(fromPaths(hoverIconPaths));
    btn->setActiveIcon(fromPaths(activeIconPaths));
    return btn;
}

} // namespace GUI
