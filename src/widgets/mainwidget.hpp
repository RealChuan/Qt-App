#pragma once

#include "widgets_global.hpp"

#include <QMainWindow>

namespace Widgets {

#ifndef Q_OS_MACOS
class WIDGETS_EXPORT MainWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MainWidget(QWidget *parent = nullptr);
    ~MainWidget() override;

    void setRestoreMaxButtonVisible(bool visible);
    void setMinButtonVisible(bool visible);

    void setCentralWidget(QWidget *centralWidget);
    void setTitleBar(QWidget *titleBar);

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

    class MainWidgetPrivate;
    QScopedPointer<MainWidgetPrivate> d_ptr;
};
#else
class WIDGETS_EXPORT MainWidget : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWidget(QWidget *parent = nullptr)
        : QMainWindow(parent)
    {}

    void setTitleBar(QWidget *widget);
    int shadowPadding() { return 0; }

signals:
    void aboutToclose();
};
#endif

} // namespace Widgets
