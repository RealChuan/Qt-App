#include "aboutpage.hpp"

#include <utils/appdata.hpp>
#include <utils/utils.hpp>

#include <QApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QToolButton>
#include <QVBoxLayout>

namespace Core {

AboutPage::AboutPage(QWidget *parent)
    : QWidget(parent)
{
    const QString title = QStringLiteral("<span style=\"font-size: 24px;\"><b>%1</b></span> "
                                         "<span style=\"font-size: 24px;\">%2</span><br>")
                              .arg(Utils::appName, Utils::version);

    auto *textLayout = new QVBoxLayout;
    textLayout->setSpacing(20);
    textLayout->addStretch();
    textLayout->addWidget(new QLabel(title, this));
    textLayout->addWidget(new QLabel(Utils::systemInfo(), this));
    textLayout->addWidget(new QLabel(Utils::copyright, this));
    textLayout->addStretch();

    auto *icon = new QToolButton(this);
    icon->setIconSize({64, 64});
    icon->setIcon(qApp->windowIcon());

    auto *topLayout = new QHBoxLayout;
    topLayout->setSpacing(20);
    topLayout->addWidget(icon);
    topLayout->addStretch();
    topLayout->addLayout(textLayout);
    topLayout->addStretch();

    auto *aboutQt = new QToolButton(this);
    aboutQt->setText(tr("About Qt"));
    connect(aboutQt, &QToolButton::clicked, qApp, &QApplication::aboutQt);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(30, 30, 30, 30);
    layout->addStretch();
    layout->addLayout(topLayout);
    layout->addStretch();
    layout->addWidget(aboutQt, 0, Qt::AlignCenter);
}

} // namespace Core
