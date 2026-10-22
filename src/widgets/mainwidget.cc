#include "mainwidget.hpp"

#include "buttoniconstatemanager.hpp"

#include <utils/utils.hpp>

#include <QApplication>
#include <QCoreApplication>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QPushButton>
#include <QSizeGrip>
#include <QToolButton>

#ifdef Q_OS_MACOS
#include <QMenu>
#include <QMenuBar>
#include <QWidgetAction>
#endif

namespace Widgets {

#ifndef Q_OS_MACOS
class MainWidget::MainWidgetPrivate
{
public:
    explicit MainWidgetPrivate(MainWidget *q)
        : q_ptr(q)
        , titleButton(new QPushButton(qApp->windowIcon(), qAppName(), q))
        , centralContainer(new QWidget(q))
        , titleWidget(new QWidget(q))
        , titleBarContainer(new QWidget(q))
        , sizeGrip(new QSizeGrip(q))
    {
        titleButton->setObjectName(QStringLiteral("AppButton"));
        titleButton->setCursor(Qt::PointingHandCursor);
        titleButton->setFlat(true);

        centralContainer->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        centralContainer->setObjectName("CentralWidget");

        initTitleButtons();
        buildTitleWidget();
        setupUi();
    }

    // 标题栏以外的内容区，setCentralWidget() 往这里的布局里换控件。
    void setupUi()
    {
        centralLayout = new QHBoxLayout(centralContainer);
        centralLayout->setContentsMargins({});
        centralLayout->setSpacing(0);

        auto *frame = new QWidget(q_ptr);
        Utils::addGraphicsDropShadowEffect(frame, shadowPadding);

        auto *grid = new QGridLayout(frame);
        grid->setContentsMargins(0, 0, 0, 0);
        grid->setSpacing(0);
        grid->addWidget(titleWidget, 0, 0);
        grid->addWidget(centralContainer, 1, 0);
        grid->addWidget(sizeGrip, 1, 0, Qt::AlignRight | Qt::AlignBottom);

        outerLayout = new QHBoxLayout(q_ptr);
        outerLayout->setSpacing(0);
        outerLayout->setContentsMargins(shadowPadding, shadowPadding, shadowPadding, shadowPadding);
        outerLayout->addWidget(frame);
    }

    MainWidget *q_ptr = nullptr;

    QPushButton *titleButton = nullptr;
    QToolButton *minButton = nullptr;
    QToolButton *maxButton = nullptr;
    QToolButton *restoreButton = nullptr;
    QToolButton *closeButton = nullptr;
    QWidget *titleWidget = nullptr;
    QWidget *centralContainer = nullptr;
    QWidget *titleBarContainer = nullptr;
    QHBoxLayout *centralLayout = nullptr;
    QHBoxLayout *titleBarLayout = nullptr;
    QHBoxLayout *outerLayout = nullptr;
    QSizeGrip *sizeGrip = nullptr;

    int shadowPadding = 10;
    bool maxRestoreVisible = true;

    // 拖动时鼠标相对窗口左上角的偏移；未按下左键时为空。
    QPoint dragOffset;

private:
    void buildTitleWidget()
    {
        titleWidget->setObjectName("TitleWidget");
        auto *layout = new QHBoxLayout(titleWidget);
        layout->setContentsMargins(12, 6, 10, 6);
        layout->setSpacing(8);
        layout->addWidget(titleButton);
        layout->addStretch();
        layout->addWidget(titleBarContainer);
        layout->addWidget(minButton);
        layout->addWidget(maxButton);
        layout->addWidget(restoreButton);
        layout->addWidget(closeButton);
    }

    void initTitleButtons()
    {
        constexpr auto kWindowButton = "WindowButton";
        const QSize size{18, 18};

        // 统一前缀
        const QString basePath = QStringLiteral(":/icon/icon/mactitle/titlebutton-");

        // 后缀表：普通态总是 4 个（含 alt）；hover / pressed 分「带 alt」与「不带 alt」两种
        const QStringList kNormalSuffix{".png", "@2.png", "-alt.png", "-alt@2.png"};
        const QStringList kHoverSuffix{"-hover.png", "-hover@2.png"};
        const QStringList
            kHoverSuffixAlt{"-hover.png", "-hover@2.png", "-hover-alt.png", "-hover-alt@2.png"};
        const QStringList kPressedSuffix{"-active.png", "-active@2.png"};
        const QStringList kPressedSuffixAlt{
            "-active.png", "-active@2.png", "-active-alt.png", "-active-alt@2.png"};

        auto buildPaths = [&](const QString &name, const QStringList &suffixes) {
            QStringList paths;
            paths.reserve(suffixes.size());
            for (const QString &suffix : suffixes)
                paths.append(basePath + name + suffix);
            return paths;
        };

        // 通用工厂：new QToolButton + 公共属性 + 状态管理器一次完成
        auto createTitleButton = [&](const QString &name,
                                     const QStringList &hoverSuffixes,
                                     const QStringList &pressedSuffixes) {
            auto *button = new QToolButton(q_ptr);
            button->setObjectName(kWindowButton);
            button->setFixedSize(size);
            button->setIconSize(size);
            button->setFocusPolicy(Qt::NoFocus);

            // manager 以 button 为 QObject parent，随 button 生命周期自动释放
            auto *manager = new ButtonIconStateManager(button);
            manager->setNormalIcon(buildPaths(name, kNormalSuffix));
            manager->setHoverIcon(buildPaths(name, hoverSuffixes));
            manager->setPressedIcon(buildPaths(name, pressedSuffixes));

            return button;
        };

        minButton
            = createTitleButton(QStringLiteral("minimize"), kHoverSuffixAlt, kPressedSuffixAlt);
        maxButton = createTitleButton(QStringLiteral("maximize"), kHoverSuffix, kPressedSuffix);
        restoreButton = createTitleButton(QStringLiteral("restore"), kHoverSuffix, kPressedSuffix);
        closeButton = createTitleButton(QStringLiteral("close"), kHoverSuffix, kPressedSuffix);

        // titleButton 是独立的菜单按钮，只共用 iconSize
        titleButton->setIconSize(size);
    }
};

MainWidget::MainWidget(QWidget *parent)
    : QWidget(parent)
    , d_ptr(std::make_unique<MainWidgetPrivate>(this))
{
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::WindowCloseButtonHint);
    setAttribute(Qt::WA_TranslucentBackground);

    retranslateUi();
    setupConnections();
    updateMaxRestoreButtons();

    resize(1000, 618);
}

MainWidget::~MainWidget() = default;

void MainWidget::setRestoreMaxButtonVisible(bool visible)
{
    d_ptr->maxRestoreVisible = visible;
    updateMaxRestoreButtons();
}

void MainWidget::setMinButtonVisible(bool visible)
{
    d_ptr->minButton->setVisible(visible);
}

void MainWidget::setCentralWidget(QWidget *centralWidget)
{
    if (!centralWidget)
        return;

    // 换内容时把上一个摘出布局并隐藏：直接删会让调用方手里的指针悬空。
    if (auto *item = d_ptr->centralLayout->takeAt(0)) {
        if (QWidget *previous = item->widget())
            previous->hide();
        delete item;
    }
    d_ptr->centralLayout->addWidget(centralWidget);
}

void MainWidget::setTitleBar(QWidget *titleBar)
{
    if (!titleBar)
        return;

    if (!d_ptr->titleBarLayout) {
        d_ptr->titleBarLayout = new QHBoxLayout(d_ptr->titleBarContainer);
        d_ptr->titleBarLayout->setContentsMargins({});
        d_ptr->titleBarLayout->setSpacing(0);
    } else if (auto *item = d_ptr->titleBarLayout->takeAt(0)) {
        if (QWidget *previous = item->widget())
            previous->hide();
        delete item;
    }

    d_ptr->titleBarLayout->addWidget(titleBar);
}

auto MainWidget::shadowPadding() const -> int
{
    return d_ptr->shadowPadding;
}

void MainWidget::setSizeGripVisible(bool visible)
{
    d_ptr->sizeGrip->setVisible(visible);
}

auto MainWidget::isInTitleBar(const QPoint &pos) const -> bool
{
    // 用 mapTo 而不是手工加阴影边距：布局调整后仍然正确。
    const QRect rect(d_ptr->titleWidget->mapTo(this, QPoint(0, 0)), d_ptr->titleWidget->size());
    return rect.contains(pos);
}

void MainWidget::updateMaxRestoreButtons()
{
    const bool maximized = isMaximized() || isFullScreen();
    d_ptr->maxButton->setVisible(d_ptr->maxRestoreVisible && !maximized);
    d_ptr->restoreButton->setVisible(d_ptr->maxRestoreVisible && maximized);
}

void MainWidget::onShowMaximized()
{
    d_ptr->outerLayout->setContentsMargins({});
    showMaximized();
    updateMaxRestoreButtons();
}

void MainWidget::onShowNormal()
{
    const int padding = d_ptr->shadowPadding;
    d_ptr->outerLayout->setContentsMargins(padding, padding, padding, padding);
    showNormal();
    updateMaxRestoreButtons();
}

void MainWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && isInTitleBar(event->position().toPoint()))
        d_ptr->dragOffset = event->globalPosition().toPoint() - pos();

    QWidget::mousePressEvent(event);
}

void MainWidget::mouseMoveEvent(QMouseEvent *event)
{
    if ((event->buttons() & Qt::LeftButton) && !isMaximized() && !isFullScreen()
        && !d_ptr->dragOffset.isNull()) {
        // ubuntu24.04 not working
        // 在main函数开始设置qputenv("QT_QPA_PLATFORM", "xcb");可以解决
        move(event->globalPosition().toPoint() - d_ptr->dragOffset);
    }

    QWidget::mouseMoveEvent(event);
}

void MainWidget::mouseReleaseEvent(QMouseEvent *event)
{
    d_ptr->dragOffset = QPoint();

    QWidget::mouseReleaseEvent(event);
}

void MainWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (isInTitleBar(event->position().toPoint())) {
        if (d_ptr->maxButton->isVisible())
            d_ptr->maxButton->click();
        else if (d_ptr->restoreButton->isVisible())
            d_ptr->restoreButton->click();
    }

    QWidget::mouseDoubleClickEvent(event);
}

void MainWidget::changeEvent(QEvent *event)
{
    QWidget::changeEvent(event);
    if (event->type() == QEvent::LanguageChange)
        retranslateUi();
}

void MainWidget::setupConnections()
{
    connect(d_ptr->minButton, &QToolButton::clicked, this, &MainWidget::showMinimized);
    connect(d_ptr->maxButton, &QToolButton::clicked, this, &MainWidget::onShowMaximized);
    connect(d_ptr->restoreButton, &QToolButton::clicked, this, &MainWidget::onShowNormal);
    connect(d_ptr->closeButton, &QToolButton::clicked, this, &MainWidget::aboutToClose);
    connect(this, &MainWidget::aboutToClose, this, &MainWidget::close, Qt::QueuedConnection);
    connect(this, &MainWidget::windowTitleChanged, d_ptr->titleButton, &QPushButton::setText);
    connect(this, &MainWidget::windowTitleChanged, d_ptr->titleButton, &QPushButton::setToolTip);
    connect(this, &MainWidget::windowIconChanged, d_ptr->titleButton, &QPushButton::setIcon);
}

void MainWidget::retranslateUi()
{
    d_ptr->minButton->setToolTip(tr("Minimize"));
    d_ptr->maxButton->setToolTip(tr("Maximize"));
    d_ptr->restoreButton->setToolTip(tr("Restore"));
    d_ptr->closeButton->setToolTip(tr("Close"));
}
#else
void MainWidget::setTitleBar(QWidget *widget)
{
    auto *widgetAction = new QWidgetAction(this);
    widgetAction->setDefaultWidget(widget);
    auto *menu = new QMenu(this);
    menu->setTitle(tr("TitleBar"));
    menu->addAction(widgetAction);
    menuBar()->addMenu(menu);
}
#endif

} // namespace Widgets
