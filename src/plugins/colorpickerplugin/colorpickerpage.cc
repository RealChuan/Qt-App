#include "colorpickerpage.hpp"

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

namespace ColorPicker {

ColorPickerPage::ColorPickerPage(QWidget *parent)
    : QWidget(parent)
{
    auto *text = new QLabel(this);
    text->setObjectName("CardHint");
    text->setWordWrap(true);
    text->setAlignment(Qt::AlignCenter);
    text->setText(
        tr("The picker follows the cursor in a small floating window. "
           "Left click copies the HEX value, Esc closes it."));

    auto *open = new QPushButton(tr("Open color picker"), this);
    open->setObjectName("PrimaryButton");
    open->setCursor(Qt::PointingHandCursor);
    connect(open, &QPushButton::clicked, this, &ColorPickerPage::openRequested);

    auto *card = new QFrame(this);
    card->setObjectName("Card");
    card->setFixedWidth(520);
    auto *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(36, 30, 36, 30);
    cardLayout->setSpacing(20);
    cardLayout->addWidget(text);
    cardLayout->addWidget(open, 0, Qt::AlignCenter);

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

} // namespace ColorPicker
