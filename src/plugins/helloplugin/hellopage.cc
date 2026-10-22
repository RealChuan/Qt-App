#include "hellopage.hpp"

#include <QEvent>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

namespace Hello {

class HelloPage::HelloPagePrivate
{
public:
    QLabel *title = nullptr;
    QLabel *hint = nullptr;
    QLabel *number = nullptr;
    QLabel *caption = nullptr;
    int count = 0;
};

HelloPage::HelloPage(QWidget *parent)
    : QWidget(parent)
    , d_ptr(std::make_unique<HelloPagePrivate>())
{
    d_ptr->title = new QLabel(this);
    d_ptr->title->setObjectName("HeroTitle");
    d_ptr->title->setAlignment(Qt::AlignCenter);

    d_ptr->hint = new QLabel(this);
    d_ptr->hint->setObjectName("HeroHint");
    d_ptr->hint->setAlignment(Qt::AlignCenter);
    d_ptr->hint->setWordWrap(true);

    d_ptr->number = new QLabel(this);
    d_ptr->number->setObjectName("HeroNumber");
    d_ptr->number->setAlignment(Qt::AlignCenter);

    d_ptr->caption = new QLabel(this);
    d_ptr->caption->setObjectName("HeroMeta");
    d_ptr->caption->setAlignment(Qt::AlignCenter);

    auto *card = new QFrame(this);
    card->setObjectName("Card");
    card->setFixedWidth(520);
    auto *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(36, 30, 36, 30);
    cardLayout->setSpacing(6);
    cardLayout->addWidget(d_ptr->title);
    cardLayout->addWidget(d_ptr->hint);
    cardLayout->addSpacing(22);
    cardLayout->addWidget(d_ptr->number);
    cardLayout->addWidget(d_ptr->caption);

    auto *centerRow = new QHBoxLayout;
    centerRow->addStretch();
    centerRow->addWidget(card);
    centerRow->addStretch();

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins({});
    layout->addStretch();
    layout->addLayout(centerRow);
    layout->addStretch();

    updateText();
}

HelloPage::~HelloPage() = default;

void HelloPage::setCount(int count)
{
    d_ptr->count = count;
    updateText();
}

void HelloPage::increment()
{
    ++d_ptr->count;
    updateText();
    emit countChanged(d_ptr->count);
}

void HelloPage::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
        updateText();
    QWidget::changeEvent(event);
}

void HelloPage::updateText()
{
    d_ptr->title->setText(tr("Hello, Qt plugin"));
    d_ptr->hint->setText(
        tr("This page is added to the shell at runtime by helloplugin.\n"
           "Press Ctrl+Alt+H, or run the hello command, to add one."));
    d_ptr->number->setText(QString::number(d_ptr->count));
    d_ptr->caption->setText(tr("command runs so far"));
}

} // namespace Hello
