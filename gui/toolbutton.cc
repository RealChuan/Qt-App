#include "toolbutton.hpp"

#include <QEvent>

namespace GUI {

class ToolButton::ToolButtonPrivate
{
public:
    ToolButtonPrivate(ToolButton *q)
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

ToolButton::~ToolButton() {}

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

bool ToolButton::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == this) {
        switch (event->type()) {
        case QEvent::Enter: setIcon(d_ptr->hoverIcon); break;
        case QEvent::MouseButtonPress: setIcon(d_ptr->activeIcon); break;
        case QEvent::QEvent::Leave: setIcon(d_ptr->icon); break;
        default: break;
        }
    }

    return QToolButton::eventFilter(watched, event);
}

QIcon fromPaths(const QStringList &Paths)
{
    QIcon icon;
    for (const auto &path : qAsConst(Paths)) {
        icon.addFile(path);
    }
    return icon;
}

ToolButton *createToolButton(const QStringList &normalIconPaths,
                             const QStringList &hoverIconPaths,
                             const QStringList &activeIconPaths,
                             QWidget *parent)
{
    auto btn = new ToolButton(parent);
    btn->setNormalIcon(fromPaths(normalIconPaths));
    btn->setHoverIcon(fromPaths(hoverIconPaths));
    btn->setActiveIcon(fromPaths(activeIconPaths));
    return btn;
}

} // namespace GUI
