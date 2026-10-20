#include "dialog.hpp"

#include <utils/utils.hpp>

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
    : MainWidget(parent)
    , d_ptr(std::make_unique<DialogPrivate>())
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
    d_ptr->flag = QDialog::Rejected;
    d_ptr->loop.quit();
}

void Dialog::onMoveParentCenter()
{
    if (QWidget *owner = parentWidget()) {
        const QRect target = QStyle::alignedRect(Qt::LeftToRight,
                                                 Qt::AlignCenter,
                                                 size(),
                                                 owner->geometry());
        move(target.topLeft());
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
