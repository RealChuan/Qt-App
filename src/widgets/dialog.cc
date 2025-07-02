#include "dialog.hpp"

#include <utils/utils.h>

#include <QtWidgets>

namespace Widgets {

#ifndef Q_OS_MACOS
class Dialog::DialogPrivate
{
public:
    explicit DialogPrivate(Dialog *q)
        : q_ptr(q)
    {}

    Dialog *q_ptr;

    QEventLoop loop;
    Dialog::ExecFlag flag = Dialog::ExecFlag::Closed;
};

Dialog::Dialog(QWidget *parent)
    : MainWidget(parent)
    , d_ptr(new DialogPrivate(this))
{
    setMinButtonVisible(false);
    setRestoreMaxButtonVisible(false);
    setSizeGripVisible(false);
    buildConnect();
    resize(600, 370);
}

Dialog::~Dialog() = default;

auto Dialog::exec() -> int
{
#ifdef Q_OS_LINUX
    QMetaObject::invokeMethod(this, &Dialog::onMoveParentCenter, Qt::QueuedConnection);
#endif
    setWindowFlags(Qt::Dialog | Qt::Popup | Qt::FramelessWindowHint | Qt::WindowCloseButtonHint
                   | Qt::MSWindowsFixedSizeDialogHint);
    setWindowModality(Qt::ApplicationModal);
    show();
    raise();
    activateWindow();

    d_ptr->flag = Closed;
    d_ptr->loop.exec();

    hide();

    return d_ptr->flag;
}

void Dialog::accept()
{
    d_ptr->flag = Accepted;
    d_ptr->loop.quit();
}

void Dialog::reject()
{
    d_ptr->flag = Rejected;
    d_ptr->loop.quit();
}

void Dialog::onClosed()
{
    d_ptr->flag = Closed;
    d_ptr->loop.quit();
}

void Dialog::onMoveParentCenter()
{
    auto *parentWidget = this->parentWidget();
    if (parentWidget) {
        auto center = parentWidget->mapToGlobal(parentWidget->rect().center());
        auto rect = this->rect();
        auto x = center.x() - rect.width() / 2;
        auto y = center.y() - rect.height() / 2;
        move(x, y);
    } else {
        Utils::windowCenter(this);
    }
}

void Dialog::buildConnect()
{
    connect(this, &Dialog::accepted, this, &Dialog::accept);
    connect(this, &Dialog::rejected, this, &Dialog::reject);
    connect(this, &Dialog::aboutToclose, this, &Dialog::onClosed);
}

#else
void Dialog::setCentralWidget(QWidget *widget)
{
    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins({});
    layout->setSpacing(0);
    layout->addWidget(widget);
}
#endif

} // namespace Widgets
