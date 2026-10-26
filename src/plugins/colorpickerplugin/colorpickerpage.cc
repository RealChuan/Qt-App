#include "colorpickerpage.hpp"

#include <QEvent>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

namespace ColorPicker {

class ColorPickerPage::ColorPickerPagePrivate
{
public:
    QLabel *hint = nullptr;
    QPushButton *open = nullptr;
};

ColorPickerPage::ColorPickerPage(QWidget *parent)
    : QWidget(parent)
    , d_ptr(std::make_unique<ColorPickerPagePrivate>())
{
    setupUi();
    retranslateUi();
    setupConnections();
}

ColorPickerPage::~ColorPickerPage() = default;

void ColorPickerPage::setupUi()
{
    d_ptr->hint = new QLabel(this);
    d_ptr->hint->setObjectName("CardHint");
    d_ptr->hint->setWordWrap(true);
    d_ptr->hint->setAlignment(Qt::AlignCenter);

    d_ptr->open = new QPushButton(this);
    d_ptr->open->setObjectName("PrimaryButton");
    d_ptr->open->setCursor(Qt::PointingHandCursor);

    auto *card = new QFrame(this);
    card->setObjectName("Card");
    card->setFixedWidth(520);
    auto *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(36, 30, 36, 30);
    cardLayout->setSpacing(20);
    cardLayout->addWidget(d_ptr->hint);
    cardLayout->addWidget(d_ptr->open, 0, Qt::AlignCenter);

    auto *centerRow = new QHBoxLayout;
    centerRow->addStretch();
    centerRow->addWidget(card);
    centerRow->addStretch();

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins({});
    layout->addStretch();
    layout->addLayout(centerRow);
    layout->addStretch();
}

void ColorPickerPage::retranslateUi()
{
    d_ptr->hint->setText(
        tr("The picker follows the cursor in a small floating window. "
           "Left click copies the HEX value, Esc closes it."));
    d_ptr->open->setText(tr("Open color picker"));
}

void ColorPickerPage::setupConnections()
{
    connect(d_ptr->open, &QPushButton::clicked, this, &ColorPickerPage::openRequested);
}

void ColorPickerPage::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
        retranslateUi();

    QWidget::changeEvent(event);
}

} // namespace ColorPicker
