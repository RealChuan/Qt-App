#include "guiwidget.hpp"

#include <QtWidgets>

namespace Plugin {

GuiWidget::GuiWidget(QWidget *parent)
    : QWidget{parent}
{
    setupUI();
}

void GuiWidget::setupUI()
{
    auto *layout = new QVBoxLayout(this);
    layout->addWidget(createButtonGroup());
    layout->addWidget(createBoxGroup());
    layout->addWidget(createBarGroup());
}

auto GuiWidget::createButtonGroup() -> QGroupBox *
{
    auto *button = new QPushButton(tr("Default"), this);
    auto *blueButton = new QPushButton(tr("Blue"), this);
    blueButton->setObjectName("BlueButton");
    auto *grayButton = new QPushButton(tr("Gray"), this);
    grayButton->setObjectName("GrayButton");
    auto *radioButton = new QRadioButton(tr("Radio"), this);
    radioButton->setChecked(true);
    auto *checkBox = new QCheckBox(tr("Check"), this);
    checkBox->setChecked(true);

    auto *groupBox = new QGroupBox(tr("Buttons"), this);
    auto *layout = new QHBoxLayout(groupBox);
    layout->setSpacing(20);
    layout->addWidget(button);
    layout->addWidget(blueButton);
    layout->addWidget(grayButton);
    layout->addWidget(new QRadioButton(tr("Radio"), this));
    layout->addWidget(radioButton);
    layout->addWidget(new QCheckBox(tr("Check"), this));
    layout->addWidget(checkBox);

    return groupBox;
}

auto GuiWidget::createBoxGroup() -> QGroupBox *
{
    auto *spinBox = new QSpinBox(this);
    spinBox->setRange(0, 100);
    spinBox->setValue(50);

    auto *comboBox = new QComboBox(this);
    for (int i = 0; i < 10; ++i) {
        comboBox->addItem(QString::number(i));
    }
    comboBox->setCurrentText("6");

    auto *groupBox = new QGroupBox(tr("Boxes"), this);
    auto *layout = new QHBoxLayout(groupBox);
    layout->setSpacing(20);
    layout->addWidget(spinBox);
    layout->addWidget(comboBox);

    return groupBox;
}

auto GuiWidget::createBarGroup() -> QGroupBox *
{
    auto *silder = new QSlider(Qt::Horizontal, this);
    silder->setRange(0, 100);
    silder->setValue(25);

    auto *progressBar = new QProgressBar(this);
    progressBar->setRange(0, 100);
    progressBar->setValue(25);

    connect(silder, &QSlider::valueChanged, progressBar, &QProgressBar::setValue);

    auto *groupBox = new QGroupBox(tr("Bars"), this);
    auto *layout = new QHBoxLayout(groupBox);
    layout->setSpacing(20);
    layout->addWidget(silder);
    layout->addWidget(progressBar);

    return groupBox;
}

} // namespace Plugin
