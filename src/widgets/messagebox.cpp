#include "messagebox.h"

#include <QLabel>
#include <QPushButton>
#include <QStyle>
#include <QToolButton>
#include <QVBoxLayout>

namespace Widgets {

class MessageBox::MessBoxPrivate
{
public:
    explicit MessBoxPrivate(MessageBox *q)
        : q_ptr(q)
        , iconButton(new QToolButton(q))
        , messageLabel(new QLabel(q))
        , yesButton(new QPushButton(MessageBox::tr("Yes"), q))
        , noButton(new QPushButton(MessageBox::tr("No"), q))
        , closeButton(new QPushButton(MessageBox::tr("Close"), q))
    {
        iconButton->setIconSize({48, 48});
        iconButton->setObjectName("IconButton");

        messageLabel->setObjectName("MessageLabel");
        messageLabel->setWordWrap(true);
        messageLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

        closeButton->setObjectName("BlueButton");
        yesButton->setObjectName("BlueButton");
        noButton->setObjectName("GrayButton");
    }

    void setupUi()
    {
        auto *widget = new QWidget(q_ptr);
        auto *layout = new QVBoxLayout(widget);
        layout->setContentsMargins(20, 40, 20, 20);
        layout->setSpacing(10);
        layout->addWidget(iconButton, 0, Qt::AlignCenter);
        layout->addWidget(messageLabel);
        layout->addWidget(yesButton);
        layout->addWidget(noButton);
        layout->addWidget(closeButton);

        q_ptr->setCentralWidget(widget);

        yesButton->hide();
        noButton->hide();
        closeButton->hide();
    }

    MessageBox *q_ptr = nullptr;
    QToolButton *iconButton = nullptr;
    QLabel *messageLabel = nullptr;
    QPushButton *yesButton = nullptr;
    QPushButton *noButton = nullptr;
    QPushButton *closeButton = nullptr;
};

MessageBox::MessageBox(QWidget *parent)
    : Dialog(parent)
    , d_ptr(std::make_unique<MessBoxPrivate>(this))
{
    d_ptr->setupUi();
    buildConnect();
    resize(300, 450);
}

MessageBox::~MessageBox() = default;

auto MessageBox::Info(QWidget *parent, const QString &msg, StandardButtons buttons) -> int
{
    MessageBox box(parent);
    box.setIcon(box.style()->standardIcon(QStyle::SP_MessageBoxInformation));
    box.setMessage(msg);
    box.setButtonVisible(buttons);
    return box.exec();
}

auto MessageBox::Warning(QWidget *parent, const QString &msg, StandardButtons buttons) -> int
{
    MessageBox box(parent);
    box.setIcon(box.style()->standardIcon(QStyle::SP_MessageBoxWarning));
    box.setMessage(msg);
    box.setButtonVisible(buttons);
    return box.exec();
}

auto MessageBox::checkButton(StandardButtons buttons, StandardButton button) -> bool
{ return buttons.testFlag(button); }

void MessageBox::setIcon(const QIcon &icon)
{ d_ptr->iconButton->setIcon(icon); }

void MessageBox::setMessage(const QString &msg)
{ d_ptr->messageLabel->setText(msg); }

void MessageBox::setButtonVisible(StandardButtons buttons)
{
    d_ptr->yesButton->setVisible(checkButton(buttons, StandardButton::Yes));
    d_ptr->noButton->setVisible(checkButton(buttons, StandardButton::No));
    d_ptr->closeButton->setVisible(checkButton(buttons, StandardButton::Close));
}

void MessageBox::buildConnect()
{
    connect(d_ptr->closeButton, &QPushButton::clicked, this, &MessageBox::rejected);
    connect(d_ptr->noButton, &QPushButton::clicked, this, &MessageBox::rejected);
    connect(d_ptr->yesButton, &QPushButton::clicked, this, &MessageBox::accept);
}

} // namespace Widgets
