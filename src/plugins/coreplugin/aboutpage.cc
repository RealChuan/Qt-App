#include "aboutpage.hpp"

#include <utils/appinfo.hpp>
#include <utils/utils.hpp>

#include <QApplication>
#include <QEvent>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

namespace Core {

class AboutPage::AboutPagePrivate
{
public:
    QLabel *icon = nullptr;
    QLabel *name = nullptr;
    QLabel *version = nullptr;
    QLabel *info = nullptr;
    QLabel *copyright = nullptr;
    QPushButton *aboutQt = nullptr;
};

AboutPage::AboutPage(QWidget *parent)
    : QWidget(parent)
    , d_ptr(std::make_unique<AboutPagePrivate>())
{
    setupUi();
    retranslateUi();
    setupConnections();
}

AboutPage::~AboutPage() = default;

void AboutPage::setupUi()
{
    d_ptr->icon = new QLabel(this);
    d_ptr->icon->setPixmap(qApp->windowIcon().pixmap(QSize(72, 72)));
    d_ptr->icon->setAlignment(Qt::AlignCenter);

    d_ptr->name = new QLabel(this);
    d_ptr->name->setObjectName("AboutName");
    d_ptr->name->setAlignment(Qt::AlignCenter);

    d_ptr->version = new QLabel(this);
    d_ptr->version->setObjectName("AboutVersion");
    d_ptr->version->setAlignment(Qt::AlignCenter);

    d_ptr->info = new QLabel(this);
    d_ptr->info->setObjectName("AboutInfo");
    d_ptr->info->setAlignment(Qt::AlignCenter);
    d_ptr->info->setWordWrap(true);
    d_ptr->info->setTextInteractionFlags(Qt::TextSelectableByMouse);

    d_ptr->copyright = new QLabel(this);
    d_ptr->copyright->setObjectName("AboutCopyright");
    d_ptr->copyright->setAlignment(Qt::AlignCenter);
    d_ptr->copyright->setWordWrap(true);

    // 默认的次级按钮样式：有边框和底色，才看得出这里可以点。
    d_ptr->aboutQt = new QPushButton(this);
    d_ptr->aboutQt->setCursor(Qt::PointingHandCursor);

    auto *card = new QFrame(this);
    card->setObjectName("Card");
    // 560 以上才能让 systemInfo() 的第一行不折在 "CPU Cores:" 后面。
    card->setFixedWidth(600);
    auto *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(32, 30, 32, 24);
    cardLayout->setSpacing(6);
    cardLayout->addWidget(d_ptr->icon);
    cardLayout->addSpacing(10);
    cardLayout->addWidget(d_ptr->name);
    cardLayout->addWidget(d_ptr->version);
    cardLayout->addSpacing(18);
    cardLayout->addWidget(d_ptr->info);
    cardLayout->addSpacing(14);
    cardLayout->addWidget(d_ptr->copyright);
    cardLayout->addSpacing(12);
    cardLayout->addWidget(d_ptr->aboutQt, 0, Qt::AlignCenter);

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

void AboutPage::retranslateUi()
{
    d_ptr->name->setText(QLatin1String(Utils::AppInfo::appName));
    d_ptr->copyright->setText(QLatin1String(Utils::AppInfo::copyright));
    d_ptr->version->setText(tr("Version %1").arg(QLatin1String(Utils::AppInfo::version)));
    d_ptr->info->setText(Utils::systemInfo());
    d_ptr->aboutQt->setText(tr("About Qt"));
}

void AboutPage::setupConnections()
{
    connect(d_ptr->aboutQt, &QPushButton::clicked, qApp, &QApplication::aboutQt);
}

void AboutPage::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
        retranslateUi();

    QWidget::changeEvent(event);
}

} // namespace Core
