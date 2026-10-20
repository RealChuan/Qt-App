#include "colorpickerpage.hpp"

#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

namespace ColorPicker {

ColorPickerPage::ColorPickerPage(QWidget *parent)
    : QWidget(parent)
{
    auto *text = new QLabel(
        tr("Click the button, then move the cursor over any pixel on screen.\n"
           "The magnifier follows the cursor; left click copies the HEX value.\n\n"
           "This page demonstrates a plugin that owns an independent tool window\n"
           "instead of a normal page."),
        this);
    text->setWordWrap(true);
    text->setAlignment(Qt::AlignCenter);

    auto *open = new QPushButton(tr("Open color picker"), this);
    connect(open, &QPushButton::clicked, this, &ColorPickerPage::openRequested);

    auto *layout = new QVBoxLayout(this);
    layout->addStretch();
    layout->addWidget(text);
    layout->addWidget(open, 0, Qt::AlignCenter);
    layout->addStretch();
}

} // namespace ColorPicker
