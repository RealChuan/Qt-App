#include "dialog.hpp"

#include <utils/utils.h>

#include <QtWidgets>

namespace GUI {

#ifndef Q_OS_MACOS
class Dialog::DialogPrivate
{
public:
    DialogPrivate(Dialog *q)
        : q_ptr(q)
    {}
    Dialog *q_ptr;
    QEventLoop loop;
    Dialog::ExecFlags flag = Dialog::ExecFlags::Close;
};

Dialog::Dialog(QWidget *parent)
    : CommonWidget(parent)
    , d_ptr(new DialogPrivate(this))
{
    setMinButtonVisible(false);
    setRestoreMaxButtonVisible(false);
    setSizeGripVisible(false);
    buildConnect();
    resize(600, 370);
}

Dialog::~Dialog() {}

int Dialog::exec()
{
#ifdef Q_OS_LINUX
    auto widget = parentWidget();
    if (widget) {
        QMetaObject::invokeMethod(
            this, [=] { Utils::windowCenter(this, widget); }, Qt::QueuedConnection);
    }
#endif
    setWindowFlags(Qt::Dialog | Qt::Popup | Qt::FramelessWindowHint | Qt::WindowCloseButtonHint
                   | Qt::MSWindowsFixedSizeDialogHint);
    setWindowModality(Qt::ApplicationModal);
    show();
    raise();
    activateWindow();

    d_ptr->flag = Close;
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
    d_ptr->flag = Close;
    d_ptr->loop.quit();
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
    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(QMargins());
    layout->setSpacing(0);
    layout->addWidget(widget);
}
#endif

} // namespace GUI
