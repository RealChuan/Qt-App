#include "messagebox.h"

#include <QtWidgets>

namespace GUI {

class MessageBox::MessBoxPrivate
{
public:
    explicit MessBoxPrivate(MessageBox *q)
        : q_ptr(q)
    {
        iconButton = new QToolButton(q_ptr);
        iconButton->setIconSize({48, 48});
        iconButton->setObjectName("IconButton");
        messageLabel = new QLabel(q_ptr);
        messageLabel->setObjectName("MessageLabel");
        messageLabel->setWordWrap(true);
        messageLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        closeButton = new QPushButton(MessageBox::tr("Close"), q_ptr);
        closeButton->setObjectName("BlueButton");
        yesButton = new QPushButton(MessageBox::tr("Yes"), q_ptr);
        yesButton->setObjectName("BlueButton");
        noButton = new QPushButton(MessageBox::tr("No"), q_ptr);
        noButton->setObjectName("GrayButton");
    }

    void setupUI()
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

    MessageBox *q_ptr;

    QToolButton *iconButton;
    QLabel *messageLabel;
    QPushButton *yesButton;
    QPushButton *noButton;
    QPushButton *closeButton;
};

MessageBox::MessageBox(QWidget *parent)
    : Dialog(parent)
    , d_ptr(new MessBoxPrivate(this))
{
    d_ptr->setupUI();
    buildConnect();
    resize(300, 450);
}

MessageBox::~MessageBox() = default;

auto MessageBox::Info(QWidget *parent, const QString &msg, StandardButtons buttons) -> int
{
    MessageBox messBox(parent);
    messBox.setIcon(messBox.style()->standardIcon(QStyle::SP_MessageBoxInformation));
    messBox.setMessage(msg);
    messBox.setButtonVisible(buttons);
    return messBox.exec();
}

auto MessageBox::Warning(QWidget *parent, const QString &msg, StandardButtons buttons) -> int
{
    MessageBox messBox(parent);
    messBox.setIcon(messBox.style()->standardIcon(QStyle::SP_MessageBoxWarning));
    messBox.setMessage(msg);
    messBox.setButtonVisible(buttons);
    return messBox.exec();
}

auto MessageBox::checkButton(StandardButtons buttons, StandardButton button) -> bool
{
    return (buttons & button) == button;
}

void MessageBox::setIcon(const QIcon &icon)
{
    d_ptr->iconButton->setIcon(icon);
}

void MessageBox::setMessage(const QString &msg)
{
    d_ptr->messageLabel->setText(msg);
}

void MessageBox::setButtonVisible(StandardButtons buttons)
{
    d_ptr->yesButton->setVisible(checkButton(buttons, Yes));
    d_ptr->noButton->setVisible(checkButton(buttons, No));
    d_ptr->closeButton->setVisible(checkButton(buttons, Close));
}

void MessageBox::buildConnect()
{
    connect(d_ptr->closeButton, &QPushButton::clicked, this, &MessageBox::rejected);
    connect(d_ptr->noButton, &QPushButton::clicked, this, &MessageBox::rejected);
    connect(d_ptr->yesButton, &QPushButton::clicked, this, &MessageBox::accept);
}

} // namespace GUI
