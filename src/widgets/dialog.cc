#include "dialog.hpp"

#include <utils/qtcassert.hpp>
#include <utils/uihelpers.hpp>

#include <QDebug>
#include <QEventLoop>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLoggingCategory>
#include <QPushButton>
#include <QStyle>

#ifdef Q_OS_MACOS
// 与 AppWindow 共用一个分类：按名字过滤一次就能同时关掉这两个类的告警。
Q_LOGGING_CATEGORY(lcDialog, "qtapp.widgets", QtInfoMsg)
#endif

namespace Widgets {

#ifndef Q_OS_MACOS
class Dialog::DialogPrivate
{
public:
    QEventLoop loop;
    int flag = QDialog::Rejected;
};

Dialog::Dialog(QWidget *parent)
    : AppWindow(parent)
    , d_ptr(std::make_unique<DialogPrivate>())
{
    // 窗口类型与模态只定一次：写在 exec() 里会让每次弹窗都重建一次原生窗口。
    setWindowFlags(
        Qt::Dialog | Qt::FramelessWindowHint | Qt::WindowCloseButtonHint
        | Qt::MSWindowsFixedSizeDialogHint);
    setWindowModality(Qt::ApplicationModal);

    setMinButtonVisible(false);
    setRestoreMaxButtonVisible(false);
    setResizable(false);
    // 对话框没有页面层，主体自己当卡片面，否则露出的画布色会和背后的主界面糊在一起。
    setBodyAsCard(true);
    resize(600, 370);
}

Dialog::~Dialog() = default;

auto Dialog::exec() -> int
{
#ifdef Q_OS_LINUX
    // 父窗口的几何在 show() 之前还没定下来，Wayland/X11 上要回到事件循环再居中。
    QMetaObject::invokeMethod(this, &Dialog::centerOnOwner, Qt::QueuedConnection);
#else
    centerOnOwner();
#endif
    Utils::restoreAndActivate(this);

    d_ptr->flag = QDialog::Rejected;
    d_ptr->loop.exec();

    hide();
    return d_ptr->flag;
}

void Dialog::accept()
{
    d_ptr->flag = QDialog::Accepted;
    d_ptr->loop.quit();
    emit accepted();
}

void Dialog::reject()
{
    d_ptr->flag = QDialog::Rejected;
    d_ptr->loop.quit();
    emit rejected();
}

void Dialog::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Escape:
        reject();
        return;
    case Qt::Key_Return:
    case Qt::Key_Enter:
        if (auto *button = defaultButton()) {
            button->click();
            return;
        }
        break;
    default:
        break;
    }

    AppWindow::keyPressEvent(event);
}

void Dialog::closeEvent(QCloseEvent *event)
{
    AppWindow::closeEvent(event);
    // 标题栏按钮、Alt+F4、程序化 close() 最终都落到这里。
    reject();
}

void Dialog::centerOnOwner()
{
    if (QWidget *owner = parentWidget()) {
        // 父窗口的坐标是屏幕坐标，Utils::centerInParent 是子控件语义，这里不能复用。
        const QRect target
            = QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, size(), owner->geometry());
        move(target.topLeft());
        return;
    }

    Utils::centerOnScreen(this);
}

auto Dialog::defaultButton() const -> QPushButton *
{
    const auto buttons = findChildren<QPushButton *>();
    for (QPushButton *button : buttons) {
        if (button->isDefault() && button->isVisible() && button->isEnabled())
            return button;
    }
    return nullptr;
}

#else
Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowFlags((windowFlags() | Qt::CustomizeWindowHint) & ~Qt::WindowMaximizeButtonHint);
}

Dialog::~Dialog() = default;

void Dialog::setMinButtonVisible(bool visible)
{
    // 系统装饰下按钮归窗口管理器，而且对话框本来就没有最小化按钮。
    if (visible) {
        qCWarning(lcDialog).noquote()
            << "Dialog::setMinButtonVisible(true): macOS dialogs have no minimize button, "
               "the window manager owns the window buttons";
    }
}

void Dialog::setRestoreMaxButtonVisible(bool visible)
{
    if (visible) {
        qCWarning(lcDialog).noquote()
            << "Dialog::setRestoreMaxButtonVisible(true): macOS dialogs have no maximize button, "
               "the window manager owns the window buttons";
    }
}

auto Dialog::shadowPadding() const -> int
{
    // 系统装饰下没有自绘阴影占位，调用方拿到的就该是 0。
    return 0;
}

void Dialog::setCentralWidget(QWidget *widget)
{
    // 内容控件是必填的：传空是调用方的错，报一声就退出，对话框本身不受影响。
    QTC_ASSERT(widget, return);

    // 重复调用换内容时只替换，不叠第二层布局。
    if (QLayout *existing = layout()) {
        if (auto *item = existing->takeAt(0)) {
            if (QWidget *previous = item->widget())
                previous->hide();
            delete item;
        }
        existing->addWidget(widget);
        return;
    }

    auto *box = new QHBoxLayout(this);
    box->setContentsMargins({});
    box->setSpacing(0);
    box->addWidget(widget);
}
#endif

} // namespace Widgets
