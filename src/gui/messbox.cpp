#include "messbox.h"

#include <QtWidgets>

namespace GUI {

class MessBox::MessBoxPrivate
{
public:
    explicit MessBoxPrivate(MessBox *q)
        : q_ptr(q)
    {
        iconLabel = new QLabel(q_ptr);
        messageLabel = new QLabel(q_ptr);
        messageLabel->setObjectName("MessageLabel");
        messageLabel->setWordWrap(true);
        closeButton = new QPushButton(QCoreApplication::translate("MessBoxPrivate", "Close"), q_ptr);
        closeButton->setObjectName("BlueButton");
        yesButton = new QPushButton(QCoreApplication::translate("MessBoxPrivate", "Yes"), q_ptr);
        yesButton->setObjectName("BlueButton");
        noButton = new QPushButton(QCoreApplication::translate("MessBoxPrivate", "No"), q_ptr);
        noButton->setObjectName("GrayButton");
    }

    void setupUI()
    {
        auto *hLayout = new QHBoxLayout;
        hLayout->setSpacing(20);
        hLayout->addSpacerItem(
            new QSpacerItem(20, 10, QSizePolicy::Maximum, QSizePolicy::Expanding));
        hLayout->addWidget(iconLabel);
        hLayout->addWidget(messageLabel);
        hLayout->addSpacerItem(
            new QSpacerItem(20, 10, QSizePolicy::Maximum, QSizePolicy::Expanding));

        auto *btnWidget = new QWidget(q_ptr);
        btnWidget->setObjectName("MessBtnWidget");
        auto *btnLayout = new QHBoxLayout(btnWidget);
        btnLayout->setContentsMargins(10, 5, 20, 5);
        btnLayout->setSpacing(5);
        btnLayout->addStretch();
        btnLayout->addWidget(yesButton);
        btnLayout->addWidget(noButton);
        btnLayout->addWidget(closeButton);

        yesButton->hide();
        noButton->hide();
        closeButton->hide();

        auto *widget = new QWidget(q_ptr);
        auto *layout = new QVBoxLayout(widget);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->addLayout(hLayout);
        layout->addWidget(btnWidget);

        q_ptr->setCentralWidget(widget);
    }

    MessBox *q_ptr;

    QLabel *iconLabel;
    QLabel *messageLabel;
    QPushButton *yesButton;
    QPushButton *noButton;
    QPushButton *closeButton;
};

MessBox::MessBox(QWidget *parent)
    : Dialog(parent)
    , d_ptr(new MessBoxPrivate(this))
{
    setMinButtonVisible(false);
    setRestoreMaxButtonVisible(false);
    d_ptr->setupUI();
    buildConnect();
    resize(400, 250);
}

MessBox::~MessBox() = default;

auto MessBox::Info(QWidget *parent, const QString &msg, MessButton button) -> int
{
    MessBox messBox(parent);
    messBox.setIconLabelObjectName("InfoLabel");
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
    messBox.setIconLabelObjectName("WarningLabel");
    messBox.setMessage(msg);
    switch (button) {
    case YesAndNoButton: messBox.setYesAndNoButtonVisible(true); break;
    case CloseButton: messBox.setCloseButtonVisible(true); break;
    default: break;
    }
    return messBox.exec();
}

void MessBox::setMessage(const QString &msg)
{
    d_ptr->messageLabel->setText(msg);
}

void MessBox::setIconLabelObjectName(const QString &objectName)
{
    d_ptr->iconLabel->setObjectName(objectName);
}

void MessBox::setYesAndNoButtonVisible(bool state)
{
    d_ptr->yesButton->setVisible(state);
    d_ptr->noButton->setVisible(state);
}

void MessBox::setCloseButtonVisible(bool state)
{
    d_ptr->closeButton->setVisible(state);
}

void MessBox::buildConnect()
{
    connect(d_ptr->closeButton, &QPushButton::clicked, this, &MessBox::rejected);
    connect(d_ptr->yesButton, &QPushButton::clicked, this, &MessBox::accept);
    connect(d_ptr->noButton, &QPushButton::clicked, this, &MessBox::rejected);
}

} // namespace GUI
