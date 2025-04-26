#pragma once

#include "commonwidget.hpp"

#include <QDialog>

namespace GUI {

#ifndef Q_OS_MACOS
class GUI_EXPORT Dialog : public CommonWidget
{
    Q_OBJECT
public:
    enum ExecFlag : int { Accepted, Rejected, Closed };

    explicit Dialog(QWidget *parent = nullptr);
    ~Dialog() override;

    auto exec() -> int;

signals:
    void accepted();
    void rejected();

public slots:
    void accept();
    void reject();

private slots:
    void onClosed();

private:
    void buildConnect();

    class DialogPrivate;
    QScopedPointer<DialogPrivate> d_ptr;
};
#else
class GUI_EXPORT Dialog : public QDialog
{
    Q_OBJECT
public:
    explicit Dialog(QWidget *parent = nullptr)
        : QDialog(parent)
    {
        setWindowFlags((windowFlags() | Qt::CustomizeWindowHint) & ~Qt::WindowMaximizeButtonHint);
    }

    void setMinButtonVisible(bool) {}
    void setRestoreMaxButtonVisible(bool) {}

    void setIcon(const QIcon &icon) { setWindowIcon(icon); }
    void setTitle(const QString &title) { setWindowTitle(title); }
    void setCentralWidget(QWidget *widget);
    int shadowPadding() { return 0; }
};
#endif

} // namespace GUI
