#include "messagebox.hpp"

#include <QEvent>
#include <QLabel>
#include <QPushButton>
#include <QStyle>
#include <QToolButton>
#include <QVBoxLayout>

namespace Widgets {

class MessageBox::MessageBoxPrivate
{
public:
    explicit MessageBoxPrivate(MessageBox *q)
        : q_ptr(q)
        , iconButton(new QToolButton(q))
        , messageLabel(new QLabel(q))
        , yesButton(new QPushButton(q))
        , noButton(new QPushButton(q))
        , closeButton(new QPushButton(q))
    {
        iconButton->setIconSize({40, 40});
        iconButton->setObjectName("MessageIcon");

        messageLabel->setObjectName("MessageLabel");
        messageLabel->setWordWrap(true);
        messageLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        messageLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);

        closeButton->setObjectName("PrimaryButton");
        yesButton->setObjectName("PrimaryButton");
        noButton->setObjectName("GrayButton");
    }

    void setupUi()
    {
        auto *widget = new QWidget(q_ptr);

        // 图标在左、正文在右；按钮统一靠右，和正文左对齐形成一条干净的基线。
        auto *textRow = new QHBoxLayout;
        textRow->setContentsMargins({});
        textRow->setSpacing(16);
        textRow->addWidget(iconButton, 0, Qt::AlignTop);
        textRow->addWidget(messageLabel, 1);

        auto *buttonRow = new QHBoxLayout;
        buttonRow->setContentsMargins({});
        buttonRow->setSpacing(8);
        buttonRow->addStretch();
        buttonRow->addWidget(noButton);
        buttonRow->addWidget(yesButton);
        buttonRow->addWidget(closeButton);

        auto *layout = new QVBoxLayout(widget);
        layout->setContentsMargins(24, 24, 24, 20);
        layout->setSpacing(20);
        layout->addLayout(textRow, 1);
        layout->addLayout(buttonRow);

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
    , d_ptr(std::make_unique<MessageBoxPrivate>(this))
{
    d_ptr->setupUi();
    retranslateUi();
    setupConnections();
    resize(420, 220);
}

void MessageBox::changeEvent(QEvent *event)
{
    Dialog::changeEvent(event);
    if (event->type() == QEvent::LanguageChange)
        retranslateUi();
}

void MessageBox::retranslateUi()
{
    d_ptr->yesButton->setText(tr("Yes"));
    d_ptr->noButton->setText(tr("No"));
    d_ptr->closeButton->setText(tr("Close"));
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
{
    return buttons.testFlag(button);
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
    d_ptr->yesButton->setVisible(checkButton(buttons, StandardButton::Yes));
    d_ptr->noButton->setVisible(checkButton(buttons, StandardButton::No));
    d_ptr->closeButton->setVisible(checkButton(buttons, StandardButton::Close));
}

void MessageBox::setupConnections()
{
    connect(d_ptr->closeButton, &QPushButton::clicked, this, &MessageBox::rejected);
    connect(d_ptr->noButton, &QPushButton::clicked, this, &MessageBox::rejected);
    connect(d_ptr->yesButton, &QPushButton::clicked, this, &MessageBox::accept);
}

} // namespace Widgets
