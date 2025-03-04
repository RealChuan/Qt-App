#include "messbox.h"

#include <QtWidgets>

namespace GUI {

class MessBox::MessBoxPrivate
{
public:
    explicit MessBoxPrivate(MessBox *q)
        : q_ptr(q)
    {
        iconButton = new QToolButton(q_ptr);
        iconButton->setIconSize({48, 48});
        iconButton->setObjectName("IconButton");
        messageLabel = new QLabel(q_ptr);
        messageLabel->setObjectName("MessageLabel");
        messageLabel->setWordWrap(true);
        messageLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        closeButton = new QPushButton(MessBox::tr("Close"), q_ptr);
        closeButton->setObjectName("BlueButton");
        yesButton = new QPushButton(MessBox::tr("Yes"), q_ptr);
        yesButton->setObjectName("BlueButton");
        noButton = new QPushButton(MessBox::tr("No"), q_ptr);
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

    MessBox *q_ptr;

    QToolButton *iconButton;
    QLabel *messageLabel;
    QPushButton *yesButton;
    QPushButton *noButton;
    QPushButton *closeButton;
};

MessBox::MessBox(QWidget *parent)
    : Dialog(parent)
    , d_ptr(new MessBoxPrivate(this))
{
    d_ptr->setupUI();
    buildConnect();
    resize(300, 450);
}

MessBox::~MessBox() = default;

auto MessBox::Info(QWidget *parent, const QString &msg, MessButton button) -> int
{
    MessBox messBox(parent);
    messBox.setIcon(messBox.style()->standardIcon(QStyle::SP_MessageBoxInformation));
    messBox.setMessage(msg);
    switch (button) {
    case YesAndNoButton: messBox.setYesAndNoButtonVisible(true); break;
    case CloseButton: messBox.setCloseButtonVisible(true); break;
    default: break;
    }
    return messBox.exec();
}

auto MessBox::Warning(QWidget *parent, const QString &msg, MessButton button) -> int
{
    MessBox messBox(parent);
    messBox.setIcon(messBox.style()->standardIcon(QStyle::SP_MessageBoxWarning));
    messBox.setMessage(msg);
    switch (button) {
    case YesAndNoButton: messBox.setYesAndNoButtonVisible(true); break;
    case CloseButton: messBox.setCloseButtonVisible(true); break;
    default: break;
    }
    return messBox.exec();
}

void MessBox::setIcon(const QIcon &icon)
{
    d_ptr->iconButton->setIcon(icon);
}

void MessBox::setMessage(const QString &msg)
{
    d_ptr->messageLabel->setText(msg);
}

void MessBox::setYesAndNoButtonVisible(bool visible)
{
    d_ptr->yesButton->setVisible(visible);
    d_ptr->noButton->setVisible(visible);
}

void MessBox::setCloseButtonVisible(bool visible)
{
    d_ptr->closeButton->setVisible(visible);
}

void MessBox::buildConnect()
{
    connect(d_ptr->closeButton, &QPushButton::clicked, this, &MessBox::rejected);
    connect(d_ptr->yesButton, &QPushButton::clicked, this, &MessBox::accept);
    connect(d_ptr->noButton, &QPushButton::clicked, this, &MessBox::rejected);
}

} // namespace GUI
