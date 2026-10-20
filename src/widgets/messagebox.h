#pragma once

#include "dialog.hpp"

#include <memory>

namespace Widgets {

class WIDGETS_EXPORT MessageBox : public Dialog
{
    Q_OBJECT
public:
    enum class StandardButton : int {
        Yes = 1 << 0,
        No = 1 << 1,
        Close = 1 << 2,
    };
    Q_DECLARE_FLAGS(StandardButtons, StandardButton)
    Q_FLAG(StandardButtons)

    explicit MessageBox(QWidget *parent = nullptr);
    ~MessageBox() override;

    void setIcon(const QIcon &icon);
    void setMessage(const QString &msg);
    void setButtonVisible(StandardButtons buttons);

    static auto Info(QWidget *parent,
                     const QString &msg,
                     StandardButtons buttons = StandardButton::Close) -> int;
    static auto Warning(QWidget *parent, const QString &msg, StandardButtons buttons) -> int;

    [[nodiscard]] static auto checkButton(StandardButtons buttons, StandardButton button) -> bool;

private:
    void buildConnect();

    class MessBoxPrivate;
    std::unique_ptr<MessBoxPrivate> d_ptr;
};

} // namespace Widgets

Q_DECLARE_OPERATORS_FOR_FLAGS(Widgets::MessageBox::StandardButtons)
