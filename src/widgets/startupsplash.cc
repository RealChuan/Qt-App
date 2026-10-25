#include "startupsplash.hpp"

#include <utils/uihelpers.hpp>

#include <QEvent>
#include <QFrame>
#include <QLabel>
#include <QProgressBar>
#include <QVBoxLayout>

#include <algorithm>

namespace Widgets {

namespace {

// 窗口比卡片大一圈，留出阴影的模糊半径。
constexpr int kShadowMargin = 8;
constexpr int kSplashWidth = 360;
constexpr int kSplashHeight = 88;
// 与 base.css 里 QProgressBar 的 min-height 对齐，否则样式表会把高度顶回去。
constexpr int kProgressHeight = 6;
// 进度条出现后窗口要加高，否则卡片会把它挤成一条缝。
constexpr int kProgressExtraHeight = 24;

} // namespace

class StartupSplash::StartupSplashPrivate
{
public:
    explicit StartupSplashPrivate(StartupSplash *q)
        : q_ptr(q)
    {}

    void setupUi()
    {
        statusLabel = new QLabel(q_ptr);
        statusLabel->setObjectName("SplashText");
        statusLabel->setAlignment(Qt::AlignCenter);

        progressBar = new QProgressBar(q_ptr);
        progressBar->setObjectName("SplashProgress");
        progressBar->setRange(0, 100);
        progressBar->setTextVisible(false);
        progressBar->setMaximumHeight(kProgressHeight);
        progressBar->hide();

        auto *card = new QFrame(q_ptr);
        card->setObjectName("SplashCard");
        Utils::addDropShadow(card, {.margin = kShadowMargin});

        // 上下留 stretch：没有进度条时文案垂直居中，出现后两者一起居中。
        auto *cardLayout = new QVBoxLayout(card);
        cardLayout->setContentsMargins(20, 14, 20, 14);
        cardLayout->setSpacing(10);
        cardLayout->addStretch(1);
        cardLayout->addWidget(statusLabel);
        cardLayout->addWidget(progressBar);
        cardLayout->addStretch(1);

        auto *layout = new QVBoxLayout(q_ptr);
        layout->setContentsMargins(kShadowMargin, kShadowMargin, kShadowMargin, kShadowMargin);
        layout->setSpacing(0);
        layout->addWidget(card);
    }

    StartupSplash *q_ptr = nullptr;
    QLabel *statusLabel = nullptr;
    QProgressBar *progressBar = nullptr;
};

StartupSplash::StartupSplash()
    : d_ptr(std::make_unique<StartupSplashPrivate>(this))
{
    d_ptr->setupUi();

    setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::MSWindowsFixedSizeDialogHint);
    setAttribute(Qt::WA_TranslucentBackground);

    retranslateUi();
    resize(kSplashWidth, kSplashHeight);
    Utils::centerOnScreen(this);
}

StartupSplash::~StartupSplash() = default;

void StartupSplash::setProgress(int percent)
{
    if (d_ptr->progressBar->isHidden()) {
        d_ptr->progressBar->show();
        resize(kSplashWidth, kSplashHeight + kProgressExtraHeight);
    }

    d_ptr->progressBar->setValue(std::clamp(percent, 0, 100));
}

void StartupSplash::changeEvent(QEvent *event)
{
    QSplashScreen::changeEvent(event);
    if (event->type() == QEvent::LanguageChange)
        retranslateUi();
}

void StartupSplash::retranslateUi()
{
    d_ptr->statusLabel->setText(tr("Starting…"));
}

} // namespace Widgets
