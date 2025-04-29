#include "mainwidget.hpp"
#include "pushbutton.hpp"

#include <QtWidgets>

namespace GUI {

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
        auto *effect = new QGraphicsDropShadowEffect(q_ptr);
        effect->setOffset(0, 0);
        effect->setColor(Qt::gray);
        effect->setBlurRadius(shadowPadding);
        widget->setGraphicsEffect(effect);

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
    PushButton *minButton;
    PushButton *maxButton;
    PushButton *restoreButton;
    PushButton *closeButton;
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
        minButton = createPushButton({":/icon/icon/mactitle/titlebutton-minimize.png",
                                      ":/icon/icon/mactitle/titlebutton-minimize@2.png",
                                      ":/icon/icon/mactitle/titlebutton-minimize-alt.png",
                                      ":/icon/icon/mactitle/titlebutton-minimize-alt@2.png"},
                                     {":/icon/icon/mactitle/titlebutton-minimize-hover.png",
                                      ":/icon/icon/mactitle/titlebutton-minimize-hover@2.png",
                                      ":/icon/icon/mactitle/titlebutton-minimize-hover-alt.png",
                                      ":/icon/icon/mactitle/titlebutton-minimize-hover-alt@2.png"},
                                     {":/icon/icon/mactitle/titlebutton-minimize-active.png",
                                      ":/icon/icon/mactitle/titlebutton-minimize-active@2.png",
                                      ":/icon/icon/mactitle/titlebutton-minimize-active-alt.png",
                                      ":/icon/icon/mactitle/titlebutton-minimize-active-alt@2.png"},
                                     q_ptr);

        maxButton = createPushButton({":/icon/icon/mactitle/titlebutton-maximize.png",
                                      ":/icon/icon/mactitle/titlebutton-maximize@2.png",
                                      ":/icon/icon/mactitle/titlebutton-maximize-alt.png",
                                      ":/icon/icon/mactitle/titlebutton-maximize-alt@2.png"},
                                     {":/icon/icon/mactitle/titlebutton-maximize-hover.png",
                                      ":/icon/icon/mactitle/titlebutton-maximize-hover@2.png",
                                      /*":/icon/icon/mactitle/titlebutton-maximize-hover-alt.png",
                                      ":/icon/icon/mactitle/titlebutton-maximize-hover-alt@2.png"*/},
                                     {":/icon/icon/mactitle/titlebutton-maximize-active.png",
                                      ":/icon/icon/mactitle/titlebutton-maximize-active@2.png",
                                      /*":/icon/icon/mactitle/titlebutton-maximize-active-alt.png",
                                      ":/icon/icon/mactitle/titlebutton-maximize-active-alt@2.png"*/},
                                     q_ptr);
        restoreButton = createPushButton({":/icon/icon/mactitle/titlebutton-restore.png",
                                          ":/icon/icon/mactitle/titlebutton-restore@2.png",
                                          ":/icon/icon/mactitle/titlebutton-restore-alt.png",
                                          ":/icon/icon/mactitle/titlebutton-restore-alt@2.png"},
                                         {":/icon/icon/mactitle/titlebutton-restore-hover.png",
                                          ":/icon/icon/mactitle/titlebutton-restore-hover@2.png",
                                          /*":/icon/icon/mactitle/titlebutton-restore-hover-alt.png",
                                          ":/icon/icon/mactitle/titlebutton-restore-hover-alt@2.png"*/},
                                         {":/icon/icon/mactitle/titlebutton-restore-active.png",
                                          ":/icon/icon/mactitle/titlebutton-restore-active@2.png",
                                          /*":/icon/icon/mactitle/titlebutton-restore-active-alt.png",
                                          ":/icon/icon/mactitle/titlebutton-restore-active-alt@2.png"*/},
                                         q_ptr);
        closeButton = createPushButton({":/icon/icon/mactitle/titlebutton-close.png",
                                        ":/icon/icon/mactitle/titlebutton-close@2.png",
                                        ":/icon/icon/mactitle/titlebutton-close-alt.png",
                                        ":/icon/icon/mactitle/titlebutton-close-alt@2.png"},
                                       {":/icon/icon/mactitle/titlebutton-close-hover.png",
                                        ":/icon/icon/mactitle/titlebutton-close-hover@2.png",
                                        /*":/icon/icon/mactitle/titlebutton-close-hover-alt.png",
                                        ":/icon/icon/mactitle/titlebutton-close-hover-alt@2.png"*/},
                                       {":/icon/icon/mactitle/titlebutton-close-active.png",
                                        ":/icon/icon/mactitle/titlebutton-close-active@2.png",
                                        /*":/icon/icon/mactitle/titlebutton-close-active-alt.png",
                                        ":/icon/icon/mactitle/titlebutton-close-active-alt@2.png"*/},
                                       q_ptr);

        const QSize size{18, 18};
        titleButton->setIconSize(size);
        minButton->setFixedSize(size);
        maxButton->setFixedSize(size);
        restoreButton->setFixedSize(size);
        closeButton->setFixedSize(size);

        const auto *const objectName("TitleButton");
        titleButton->setObjectName(objectName);
        minButton->setObjectName(objectName);
        maxButton->setObjectName(objectName);
        restoreButton->setObjectName(objectName);
        closeButton->setObjectName(objectName);
    }
};

MainWidget::MainWidget(QWidget *parent)
    : QWidget{parent}
    , d_ptr(new MainWidgetPrivate(this))
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
{
    d_ptr->minButton->setVisible(visible);
}

void MainWidget::setTitle(const QString &title)
{
    setWindowTitle(title);
    d_ptr->titleButton->setToolTip(title);
}

void MainWidget::setIcon(const QIcon &icon)
{
    if (icon.isNull()) {
        d_ptr->titleButton->setIcon({});
        return;
    }

    setWindowIcon(icon);
}

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

void MainWidget::setShadowPadding(int shadowPadding)
{
    d_ptr->shadowPadding = shadowPadding;
}

auto MainWidget::shadowPadding() -> int
{
    return d_ptr->layout->contentsMargins().left();
}

void MainWidget::setSizeGripVisible(bool visible)
{
    d_ptr->sizeGrip->setVisible(visible);
}

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
    if (isMaximized() || isFullScreen()) {
    } else if (!d_ptr->lastPoint.isNull()) {
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

} // namespace GUI
