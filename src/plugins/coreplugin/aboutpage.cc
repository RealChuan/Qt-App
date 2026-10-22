#include "aboutpage.hpp"

#include <utils/appinfo.hpp>
#include <utils/utils.hpp>

#include <QApplication>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

namespace Core {

AboutPage::AboutPage(QWidget *parent)
    : QWidget(parent)
{
    auto *icon = new QLabel(this);
    icon->setPixmap(qApp->windowIcon().pixmap(QSize(72, 72)));
    icon->setAlignment(Qt::AlignCenter);

    auto *name = new QLabel(QLatin1String(Utils::appName), this);
    name->setObjectName("AboutName");
    name->setAlignment(Qt::AlignCenter);

    auto *version = new QLabel(tr("Version %1").arg(QLatin1String(Utils::version)), this);
    version->setObjectName("AboutVersion");
    version->setAlignment(Qt::AlignCenter);

    auto *info = new QLabel(Utils::systemInfo(), this);
    info->setObjectName("AboutInfo");
    info->setAlignment(Qt::AlignCenter);
    info->setWordWrap(true);
    info->setTextInteractionFlags(Qt::TextSelectableByMouse);

    auto *copyright = new QLabel(QLatin1String(Utils::copyright), this);
    copyright->setObjectName("AboutCopyright");
    copyright->setAlignment(Qt::AlignCenter);
    copyright->setWordWrap(true);

    // 默认的次级按钮样式：有边框和底色，才看得出这里可以点。
    auto *aboutQt = new QPushButton(tr("About Qt"), this);
    aboutQt->setCursor(Qt::PointingHandCursor);
    connect(aboutQt, &QPushButton::clicked, qApp, &QApplication::aboutQt);

    auto *card = new QFrame(this);
    card->setObjectName("Card");
    // 560 以上才能让 systemInfo() 的第一行不折在 "CPU Cores:" 后面。
    card->setFixedWidth(600);
    auto *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(32, 30, 32, 24);
    cardLayout->setSpacing(6);
    cardLayout->addWidget(icon);
    cardLayout->addSpacing(10);
    cardLayout->addWidget(name);
    cardLayout->addWidget(version);
    cardLayout->addSpacing(18);
    cardLayout->addWidget(info);
    cardLayout->addSpacing(14);
    cardLayout->addWidget(copyright);
    cardLayout->addSpacing(12);
    cardLayout->addWidget(aboutQt, 0, Qt::AlignCenter);

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

} // namespace Core
