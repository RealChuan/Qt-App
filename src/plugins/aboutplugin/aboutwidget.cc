#include "aboutwidget.hpp"

#include <utils/appdata.hpp>
#include <utils/utils.hpp>

#include <QtWidgets>

namespace Plugin {

AboutWidget::AboutWidget(QWidget *parent)
    : QWidget{parent}
{
    setupUI();
}

void AboutWidget::setupUI()
{
    auto text = QString("<span style=\"font-size: 24px;\"><b>%1</b></span> <span "
                        "style=\"font-size: 24px;\">%2</span><br>")
                    .arg(Utils::appName, Utils::version);

    auto *textLayout = new QVBoxLayout;
    textLayout->setSpacing(20);
    textLayout->addStretch();
    textLayout->addWidget(new QLabel{text, this});
    textLayout->addWidget(new QLabel{Utils::systemInfo(), this});
    textLayout->addWidget(new QLabel{Utils::copyright, this});
    textLayout->addStretch();

    auto *iconButton = new QToolButton(this);
    iconButton->setIconSize({64, 64});
    iconButton->setIcon(qApp->windowIcon());

    auto *topLayout = new QHBoxLayout;
    topLayout->setSpacing(20);
    topLayout->addWidget(iconButton);
    topLayout->addStretch();
    topLayout->addLayout(textLayout);
    topLayout->addStretch();

    auto *aboutQtButton = new QToolButton(this);
    aboutQtButton->setText(tr("About Qt"));
    connect(aboutQtButton, &QToolButton::clicked, qApp, &QApplication::aboutQt);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(30, 30, 30, 30);
    layout->addStretch();
    layout->addLayout(topLayout);
    layout->addStretch();
    layout->addWidget(aboutQtButton, 0, Qt::AlignCenter);
}

} // namespace Plugin
