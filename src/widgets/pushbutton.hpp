#ifndef PUSHBUTTON_HPP
#define PUSHBUTTON_HPP

#include <QPushButton>

namespace Widgets {

class PushButton : public QPushButton
{
    Q_OBJECT
public:
    explicit PushButton(QWidget *parent = nullptr);
    ~PushButton() override;

    void setNormalIcon(const QIcon &icon);
    void setHoverIcon(const QIcon &icon);
    void setActiveIcon(const QIcon &icon);

private slots:
    void onToggled(bool checked);

protected:
    auto eventFilter(QObject *watched, QEvent *event) -> bool override;

private:
    void buildConnect();

    class PushButtonPrivate;
    QScopedPointer<PushButtonPrivate> d_ptr;
};

auto createPushButton(const QStringList &normalIconPaths,
                      const QStringList &hoverIconPaths,
                      const QStringList &activeIconPaths,
                      QWidget *parent = nullptr) -> PushButton *;

} // namespace Widgets

#endif // PUSHBUTTON_HPP
