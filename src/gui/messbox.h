#ifndef MESSAGEBOX_H
#define MESSAGEBOX_H

#include "dialog.hpp"

namespace GUI {

class GUI_EXPORT MessBox : public Dialog
{
    Q_OBJECT
public:
    enum MessButton { YESButton, NOButton, CloseButton, YesAndNoButton };

    explicit MessBox(QWidget *parent = nullptr);
    ~MessBox() override;

    static auto Info(QWidget *parent, const QString &msg, MessButton button = CloseButton) -> int;
    static auto Warning(QWidget *parent, const QString &msg, MessButton button) -> int;

    void setIcon(const QIcon &icon);
    void setMessage(const QString &msg);

    void setYesAndNoButtonVisible(bool visible);
    void setCloseButtonVisible(bool visible);

private:
    void buildConnect();

    class MessBoxPrivate;
    QScopedPointer<MessBoxPrivate> d_ptr;
};

} // namespace GUI

#endif // MESSAGEBOX_H
