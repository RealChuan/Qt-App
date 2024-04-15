#include "pushbutton.hpp"

#include <QEvent>

namespace GUI {

class PushButton::PushButtonPrivate
{
public:
    explicit PushButtonPrivate(PushButton *q)
        : q_ptr(q)
    {}

    PushButton *q_ptr;

    QIcon icon;
    QIcon hoverIcon;
    QIcon activeIcon;
};

PushButton::PushButton(QWidget *parent)
    : QPushButton(parent)
    , d_ptr(new PushButtonPrivate(this))
{
    buildConnect();
    installEventFilter(this);
}

PushButton::~PushButton() {}

void PushButton::setNormalIcon(const QIcon &icon)
{
    d_ptr->icon = icon;
    setIcon(icon);
}

void PushButton::setHoverIcon(const QIcon &icon)
{
    d_ptr->hoverIcon = icon;
}

void PushButton::setActiveIcon(const QIcon &icon)
{
    d_ptr->activeIcon = icon;
}

void PushButton::onToggled(bool checked)
{
    setIcon(checked ? d_ptr->activeIcon : d_ptr->icon);
}

bool PushButton::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == this) {
        switch (event->type()) {
        case QEvent::Enter: setIcon(d_ptr->hoverIcon); break;
        case QEvent::MouseButtonPress: setIcon(d_ptr->activeIcon); break;
        case QEvent::Leave: setIcon(d_ptr->icon); break;
        default: break;
        }
    }

    return QPushButton::eventFilter(watched, event);
}

void PushButton::buildConnect()
{
    connect(this, &PushButton::toggled, this, &PushButton::onToggled);
}

auto createPushButton(const QStringList &normalIconPaths,
                      const QStringList &hoverIconPaths,
                      const QStringList &activeIconPaths,
                      QWidget *parent) -> PushButton *
{
    auto fromPaths = [&](const QStringList &Paths) -> QIcon {
        QIcon icon;
        for (const auto &path : std::as_const(Paths)) {
            icon.addFile(path);
        }
        return icon;
    };

    auto *btn = new PushButton(parent);
    btn->setNormalIcon(fromPaths(normalIconPaths));
    btn->setHoverIcon(fromPaths(hoverIconPaths));
    btn->setActiveIcon(fromPaths(activeIconPaths));
    return btn;
}

} // namespace GUI
