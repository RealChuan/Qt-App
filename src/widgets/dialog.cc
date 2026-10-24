#include "dialog.hpp"

#include <utils/uihelpers.hpp>

#include <QEventLoop>
#include <QHBoxLayout>
#include <QtWidgets>

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
    setMinButtonVisible(false);
    setRestoreMaxButtonVisible(false);
    setResizable(false);
    setupConnections();
    resize(600, 370);
}

Dialog::~Dialog() = default;

auto Dialog::exec() -> int
{
#ifdef Q_OS_LINUX
    QMetaObject::invokeMethod(this, &Dialog::onMoveParentCenter, Qt::QueuedConnection);
#endif
    // 只保留一种窗口类型：Popup 与 Dialog 混用会让窗口变成"点外部就关"的菜单语义，
    // 也拿不到正常的模态激活。
    setWindowFlags(
        Qt::Dialog | Qt::FramelessWindowHint | Qt::WindowCloseButtonHint
        | Qt::MSWindowsFixedSizeDialogHint);
    setWindowModality(Qt::ApplicationModal);
    show();
    raise();
    activateWindow();

    d_ptr->flag = QDialog::Rejected;
    d_ptr->loop.exec();

    hide();
    return d_ptr->flag;
}

void Dialog::accept()
{
    d_ptr->flag = QDialog::Accepted;
    d_ptr->loop.quit();
}

void Dialog::reject()
{
    d_ptr->flag = QDialog::Rejected;
    d_ptr->loop.quit();
}

void Dialog::onClosed()
{
    reject();
}

void Dialog::closeEvent(QCloseEvent *event)
{
    QWidget::closeEvent(event);
    // 标题栏的关闭按钮会先发 aboutToClose 再 close()，这里重复调用只是把 flag
    // 再置一次，不产生副作用。
    reject();
}

void Dialog::onMoveParentCenter()
{
    if (QWidget *owner = parentWidget()) {
        const QRect target
            = QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, size(), owner->geometry());
        move(target.topLeft());
    } else {
        Utils::centerOnScreen(this);
    }
}

void Dialog::setupConnections()
{
    connect(this, &Dialog::accepted, this, &Dialog::accept);
    connect(this, &Dialog::rejected, this, &Dialog::reject);
    connect(this, &Dialog::aboutToClose, this, &Dialog::onClosed);
}

#else
void Dialog::setCentralWidget(QWidget *widget)
{
    if (!widget)
        return;

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
