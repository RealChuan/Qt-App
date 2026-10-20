#pragma once

#include "mainwidget.hpp"

#include <QDialog>

#include <memory>

namespace Widgets {

#ifndef Q_OS_MACOS
class WIDGETS_EXPORT Dialog : public MainWidget
{
    Q_OBJECT
public:
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
    void onMoveParentCenter();

private:
    void buildConnect();

    class DialogPrivate;
    std::unique_ptr<DialogPrivate> d_ptr;
};
#else
class WIDGETS_EXPORT Dialog : public QDialog
{
    Q_OBJECT
public:
    explicit Dialog(QWidget *parent = nullptr)
        : QDialog(parent)
    { setWindowFlags((windowFlags() | Qt::CustomizeWindowHint) & ~Qt::WindowMaximizeButtonHint); }

    void setMinButtonVisible(bool) {}
    void setRestoreMaxButtonVisible(bool) {}

    void setCentralWidget(QWidget *widget);
    [[nodiscard]] auto shadowPadding() const -> int { return 0; }
};
#endif

} // namespace Widgets
