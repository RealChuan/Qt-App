#include "guiwidget.hpp"

#include <gui/messagebox.h>

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
    layout->addWidget(createInputGroup());
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

    connect(button, &QPushButton::clicked, this, [this] {
        GUI::MessageBox::Info(this,
                              tr("This is an info message."),
                              GUI::MessageBox::Yes | GUI::MessageBox::No);
    });
    connect(blueButton, &QPushButton::clicked, this, [this] {
        GUI::MessageBox::Warning(this, tr("This is a warning message."), GUI::MessageBox::Close);
    });
    grayButton->setMenu(createMenu());

    auto *groupBox = new QGroupBox(tr("Buttons"), this);
    auto *layout = new QHBoxLayout(groupBox);
    layout->setSpacing(30);
    layout->addWidget(button);
    layout->addWidget(blueButton);
    layout->addWidget(grayButton);
    layout->addWidget(new QRadioButton(tr("Radio"), this));
    layout->addWidget(radioButton);
    layout->addWidget(new QCheckBox(tr("Check"), this));
    layout->addWidget(checkBox);

    return groupBox;
}

auto GuiWidget::createInputGroup() -> QGroupBox *
{
    auto *spinBox = new QSpinBox(this);
    spinBox->setRange(0, 100);
    spinBox->setValue(50);

    auto *comboBox = new QComboBox(this);
    for (int i = 0; i < 10; ++i) {
        comboBox->addItem(QString::number(i));
    }
    comboBox->setCurrentText("6");

    auto *lineEdit = new QLineEdit(this);
    lineEdit->setPlaceholderText("Text");
    lineEdit->setClearButtonEnabled(true);

    auto *lineEdit2 = new QLineEdit(this);
    lineEdit2->setPlaceholderText("Password");
    lineEdit2->setText("Password");
    lineEdit2->setEchoMode(QLineEdit::Password);

    auto *groupBox = new QGroupBox(tr("Inputs"), this);
    auto *layout = new QHBoxLayout(groupBox);
    layout->setSpacing(20);
    layout->addWidget(spinBox);
    layout->addWidget(comboBox);
    layout->addWidget(lineEdit);
    layout->addWidget(lineEdit2);
    layout->addWidget(new QDial(this));

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

auto GuiWidget::createMenu() -> QMenu *
{
    auto *menu = new QMenu(this);
    menu->addAction(tr("Action 1"));
    auto *action = menu->addAction(tr("Action 2"));
    action->setCheckable(true);
    action = menu->addAction(tr("Action 3"));
    action->setCheckable(true);
    action->setChecked(true);
    menu->addSeparator();
    menu->addAction(style()->standardIcon(QStyle::SP_DesktopIcon), tr("Action 4"));
    menu->addAction(style()->standardIcon(QStyle::SP_TrashIcon), tr("Action 5"));

    auto *submenu = menu->addMenu(tr("Submenu"));
    submenu->addAction(tr("Action 1"));
    action = submenu->addAction(tr("Action 2"));
    action->setCheckable(true);
    action = submenu->addAction(tr("Action 3"));
    action->setCheckable(true);
    action->setChecked(true);
    submenu->addSeparator();
    submenu->addAction(style()->standardIcon(QStyle::SP_DriveFDIcon), tr("Action 4"));
    submenu->addAction(style()->standardIcon(QStyle::SP_DriveHDIcon), tr("Action 5"));

    return menu;
}

} // namespace Plugin
