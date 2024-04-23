#ifndef COMMONWIDGET_HPP
#define COMMONWIDGET_HPP

#include "gui_global.hpp"

#include <QMainWindow>
#include <QWidget>

namespace GUI {

#ifndef Q_OS_MACOS
class GUI_EXPORT CommonWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CommonWidget(QWidget *parent = nullptr);
    ~CommonWidget() override;

    void setRestoreMaxButtonVisible(bool visible);
    void setMinButtonVisible(bool visible);

    void setTitle(const QString &title);
    void setIcon(const QIcon &icon);

    void setCentralWidget(QWidget *centralWidget);
    void setTitleBar(QWidget *titleBar);

    void setShadowPadding(int shadowPadding = 10);
    auto shadowPadding() -> int;

    void setSizeGripVisible(bool visible);

signals:
    void aboutToclose();

private slots:
    void onShowMaximized();
    void onShowNormal();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void changeEvent(QEvent *event) override;

private:
    void buildConnnect();
    void setTr();

    class CommonWidgetPrivate;
    QScopedPointer<CommonWidgetPrivate> d_ptr;
};
#else
class GUI_EXPORT CommonWidget : public QMainWindow
{
    Q_OBJECT
public:
    explicit CommonWidget(QWidget *parent = nullptr)
        : QMainWindow(parent)
    {}

    void setIcon(const QIcon &icon) { setWindowIcon(icon); }
    void setTitle(const QString &title) { setWindowTitle(title); }
    void setTitleBar(QWidget *widget);
    int shadowPadding() { return 0; }

signals:
    void aboutToclose();
};
#endif

} // namespace GUI

#endif // COMMONWIDGET_HPP
