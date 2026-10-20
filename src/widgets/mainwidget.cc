#include "mainwidget.hpp"
#include "buttoniconstatemanager.hpp"

#include <utils/utils.hpp>

#include <QtWidgets>

namespace Widgets {

#ifndef Q_OS_MACOS
class MainWidget::MainWidgetPrivate
{
public:
    explicit MainWidgetPrivate(MainWidget *q)
        : q_ptr(q)
    {
        titleButton = new QPushButton(qApp->windowIcon(), qAppName(), q_ptr);

        centralWidget = new QWidget(q_ptr);
        centralWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        centralWidget->setObjectName("CentralWidget");
        titleBar = new QWidget(q_ptr);
        sizeGrip = new QSizeGrip(q_ptr);

        initTitleButton();
        creatTitleWidget();
    }

    void setupUI()
    {
        auto *widget = new QWidget(q_ptr);
        Utils::addGraphicsDropShadowEffect(widget, shadowPadding);

        auto *layout = new QGridLayout(widget);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(0);
        layout->addWidget(titleWidget, 0, 0);
        layout->addWidget(centralWidget, 1, 0);
        layout->addWidget(sizeGrip, 1, 0, Qt::AlignRight | Qt::AlignBottom);

        this->layout = new QHBoxLayout(q_ptr);
        this->layout->setSpacing(0);
        this->layout->setContentsMargins(shadowPadding, shadowPadding, shadowPadding, shadowPadding);
        this->layout->addWidget(widget);
    }

    MainWidget *q_ptr;

    QPushButton *titleButton;
    QToolButton *minButton;
    QToolButton *maxButton;
    QToolButton *restoreButton;
    QToolButton *closeButton;
    QWidget *titleWidget;
    QWidget *centralWidget;
    QWidget *titleBar;
    QHBoxLayout *layout;
    QSizeGrip *sizeGrip;

    int shadowPadding = 10;

    //记录鼠标位置
    QPoint lastPoint;

private:
    void creatTitleWidget()
    {
        titleWidget = new QWidget(q_ptr);
        titleWidget->setObjectName("TitleWidget");
        auto *layout = new QHBoxLayout(titleWidget);
        layout->setContentsMargins(6, 6, 6, 6);
        layout->setSpacing(10);
        layout->addWidget(titleButton);
        layout->addStretch();
        layout->addWidget(titleBar);
        layout->addWidget(minButton);
        layout->addWidget(maxButton);
        layout->addWidget(restoreButton);
        layout->addWidget(closeButton);
    }

    void initTitleButton()
    {
        constexpr auto kTitleButton = "TitleButton";
        const QSize size{18, 18};

        // 统一前缀
        const QString basePath = QStringLiteral(":/icon/icon/mactitle/titlebutton-");

        // 后缀表：普通态总是 4 个（含 alt）；hover / pressed 分「带 alt」与「不带 alt」两种
        const QStringList kNormalSuffix{".png", "@2.png", "-alt.png", "-alt@2.png"};
        const QStringList kHoverSuffix{"-hover.png", "-hover@2.png"};
        const QStringList kHoverSuffixAlt{"-hover.png",
                                          "-hover@2.png",
                                          "-hover-alt.png",
                                          "-hover-alt@2.png"};
        const QStringList kPressedSuffix{"-active.png", "-active@2.png"};
        const QStringList kPressedSuffixAlt{"-active.png",
                                            "-active@2.png",
                                            "-active-alt.png",
                                            "-active-alt@2.png"};

        auto buildPaths = [&](const QString &name, const QStringList &suffixes) {
            QStringList paths;
            paths.reserve(suffixes.size());
            for (const QString &suffix : suffixes) {
                paths.append(basePath + name + suffix);
            }
            return paths;
        };

        // 通用工厂：new QToolButton + 公共属性 + 状态管理器一次完成
        auto createTitleButton = [&](const QString &name,
                                     const QStringList &hoverSuffixes,
                                     const QStringList &pressedSuffixes) {
            auto *button = new QToolButton(q_ptr);
            button->setObjectName(kTitleButton);
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

        minButton = createTitleButton(QStringLiteral("minimize"),
                                      kHoverSuffixAlt,
                                      kPressedSuffixAlt);
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
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::WindowCloseButtonHint
                   | Qt::MSWindowsFixedSizeDialogHint);
    setAttribute(Qt::WA_TranslucentBackground);
    d_ptr->setupUI();
    buildConnnect();
    resize(1000, 618);
    setTr();
    QMetaObject::invokeMethod(this, &MainWidget::onShowNormal, Qt::QueuedConnection);
}

MainWidget::~MainWidget() = default;

void MainWidget::setRestoreMaxButtonVisible(bool visible)
{
    if (!visible) {
        d_ptr->maxButton->setVisible(visible);
        d_ptr->restoreButton->setVisible(visible);
        return;
    }
    if (isMaximized()) {
        d_ptr->restoreButton->setVisible(true);
        d_ptr->maxButton->setVisible(false);
    } else {
        d_ptr->restoreButton->setVisible(false);
        d_ptr->maxButton->setVisible(true);
    }
}

void MainWidget::setMinButtonVisible(bool visible)
{ d_ptr->minButton->setVisible(visible); }

void MainWidget::setCentralWidget(QWidget *centralWidget)
{
    auto *layout = new QHBoxLayout(d_ptr->centralWidget);
    layout->setContentsMargins({});
    layout->setSpacing(0);
    layout->addWidget(centralWidget);
}

void MainWidget::setTitleBar(QWidget *titleBar)
{
    auto *layout = new QHBoxLayout(d_ptr->titleBar);
    layout->setContentsMargins({});
    layout->setSpacing(0);
    layout->addWidget(titleBar);
}

auto MainWidget::shadowPadding() -> int
{ return d_ptr->layout->contentsMargins().left(); }

void MainWidget::setSizeGripVisible(bool visible)
{ d_ptr->sizeGrip->setVisible(visible); }

void MainWidget::onShowMaximized()
{
    d_ptr->layout->setContentsMargins({});
    showMaximized();
    d_ptr->maxButton->hide();
    d_ptr->restoreButton->show();
}

void MainWidget::onShowNormal()
{
    d_ptr->layout->setContentsMargins(d_ptr->shadowPadding,
                                      d_ptr->shadowPadding,
                                      d_ptr->shadowPadding,
                                      d_ptr->shadowPadding);
    showNormal();
    if (d_ptr->maxButton->isVisible() || d_ptr->restoreButton->isVisible()) {
        d_ptr->maxButton->show();
        d_ptr->restoreButton->hide();
    }
}

void MainWidget::mousePressEvent(QMouseEvent *event)
{
    const auto margins(d_ptr->layout->contentsMargins());
    auto rect(d_ptr->titleWidget->rect());
    rect = QRect(rect.x() + margins.left(), rect.y() + margins.top(), rect.width(), rect.height());
    if (rect.contains(event->pos())) {
        d_ptr->lastPoint = event->globalPosition().toPoint() - pos();
    }

    QWidget::mousePressEvent(event);
}

void MainWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (!isMaximized() && !isFullScreen() && !d_ptr->lastPoint.isNull()) {
        // ubuntu24.04 not working
        // 在main函数开始设置qputenv("QT_QPA_PLATFORM", "xcb");可以解决
        move(event->globalPosition().toPoint() - d_ptr->lastPoint);
    }

    QWidget::mouseMoveEvent(event);
}

void MainWidget::mouseReleaseEvent(QMouseEvent *event)
{
    d_ptr->lastPoint = QPoint();

    QWidget::mouseReleaseEvent(event);
}

void MainWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (!d_ptr->titleWidget->rect().contains(event->pos())) {
    } else if (d_ptr->maxButton->isVisible()) {
        d_ptr->maxButton->click();
    } else if (d_ptr->restoreButton->isVisible()) {
        d_ptr->restoreButton->click();
    }

    QWidget::mouseDoubleClickEvent(event);
}

void MainWidget::changeEvent(QEvent *event)
{
    QWidget::changeEvent(event);
    switch (event->type()) {
    case QEvent::LanguageChange: setTr(); break;
    default: break;
    }
}

void MainWidget::buildConnnect()
{
    connect(d_ptr->minButton, &QToolButton::clicked, this, &MainWidget::showMinimized);
    connect(d_ptr->maxButton, &QToolButton::clicked, this, &MainWidget::onShowMaximized);
    connect(d_ptr->restoreButton, &QToolButton::clicked, this, &MainWidget::onShowNormal);
    connect(d_ptr->closeButton, &QToolButton::clicked, this, &MainWidget::aboutToclose);
    connect(this, &MainWidget::aboutToclose, this, &MainWidget::close, Qt::QueuedConnection);
    connect(this, &MainWidget::windowTitleChanged, d_ptr->titleButton, &QPushButton::setText);
    connect(this, &MainWidget::windowTitleChanged, d_ptr->titleButton, &QPushButton::setToolTip);
    connect(this, &MainWidget::windowIconChanged, d_ptr->titleButton, &QPushButton::setIcon);
}

void MainWidget::setTr()
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
