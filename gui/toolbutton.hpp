#ifndef TOOLBUTTON_HPP
#define TOOLBUTTON_HPP

#include "gui_global.hpp"

#include <QToolButton>

namespace GUI {

class GUI_EXPORT ToolButton : public QToolButton
{
public:
    explicit ToolButton(QWidget *parent = nullptr);
    ~ToolButton() override;

    void setNormalIcon(const QIcon &icon);
    void setHoverIcon(const QIcon &icon);
    void setActiveIcon(const QIcon &icon);

protected:
    auto eventFilter(QObject *watched, QEvent *event) -> bool override;

private:
    class ToolButtonPrivate;
    QScopedPointer<ToolButtonPrivate> d_ptr;
};

auto createToolButton(const QStringList &normalIconPaths,
                             const QStringList &hoverIconPaths,
                             const QStringList &activeIconPaths,
                             QWidget *parent = nullptr) -> ToolButton *;

} // namespace GUI

#endif // TOOLBUTTON_HPP
