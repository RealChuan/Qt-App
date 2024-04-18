#include "hellowidget.hpp"

#include <QtWidgets>

namespace Plugin {

class HelloWidget::HelloWidgetPrivate
{
public:
    explicit HelloWidgetPrivate(HelloWidget *q)
        : q_ptr(q)
    {
        stackedWidget = new QStackedWidget(q_ptr);
        while (labels.size() < labelsCount) {
            auto *label = new QLabel(q_ptr);
            label->setObjectName("HomeLabel");
            label->setAlignment(Qt::AlignCenter);
            label->setWordWrap(true);
            labels.append(label);
            stackedWidget->addWidget(label);
        }

        previousButton = new QToolButton(q_ptr);
        nextButton = new QToolButton(q_ptr);
    }

    void setupUI()
    {
        auto *buttonLayout = new QHBoxLayout;
        buttonLayout->addWidget(previousButton);
        buttonLayout->addStretch();
        buttonLayout->addWidget(nextButton);

        auto *layout = new QVBoxLayout(q_ptr);
        layout->setContentsMargins(30, 30, 30, 30);
        layout->addWidget(stackedWidget);
        layout->addLayout(buttonLayout);
    }

    void enableButtons()
    {
        previousButton->setEnabled(stackedWidget->currentIndex() > 0);
        nextButton->setEnabled(stackedWidget->currentIndex() < labelsCount - 1);
    }

    HelloWidget *q_ptr;

    QStackedWidget *stackedWidget;
    QVector<QLabel *> labels;
    const int labelsCount = 4;

    QToolButton *previousButton;
    QToolButton *nextButton;
};

HelloWidget::HelloWidget(QWidget *parent)
    : QWidget{parent}
    , d_ptr(new HelloWidgetPrivate(this))
{
    d_ptr->setupUI();
    buildConnect();
    setupTr();

    QMetaObject::invokeMethod(this, &HelloWidget::onNext, Qt::QueuedConnection);
}

HelloWidget::~HelloWidget() = default;

void HelloWidget::onPrevious()
{
    d_ptr->stackedWidget->setCurrentIndex((d_ptr->stackedWidget->currentIndex() - 1)
                                          % d_ptr->labelsCount);
    d_ptr->enableButtons();
}

void HelloWidget::onNext()
{
    d_ptr->stackedWidget->setCurrentIndex((d_ptr->stackedWidget->currentIndex() + 1)
                                          % d_ptr->labelsCount);
    d_ptr->enableButtons();
}

void HelloWidget::changeEvent(QEvent *event)
{
    QWidget::changeEvent(event);
    switch (event->type()) {
    case QEvent::LanguageChange: setupTr(); break;
    default: break;
    }
}

void HelloWidget::setupTr()
{
    QStringList list{tr("Hello there! How's your day going so far?"),
                     tr("Hi, it's great to see you again!"),
                     tr("Good morning/afternoon/evening! How are you?"),
                     tr("Hey, hope you're having a wonderful day!")};

    for (int i = 0; i < d_ptr->labelsCount; ++i) {
        d_ptr->labels[i]->setText(list[i]);
    }

    d_ptr->previousButton->setText(tr("Previous"));
    d_ptr->nextButton->setText(tr("Next"));
}

void HelloWidget::buildConnect()
{
    connect(d_ptr->previousButton, &QToolButton::clicked, this, &HelloWidget::onPrevious);
    connect(d_ptr->nextButton, &QToolButton::clicked, this, &HelloWidget::onNext);
}

} // namespace Plugin
