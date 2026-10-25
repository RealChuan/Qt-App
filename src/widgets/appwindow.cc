#include "appwindow.hpp"

#include "buttoniconstatemanager.hpp"

#include <utils/qtcassert.hpp>
#include <utils/uihelpers.hpp>

#include <QAction>
#include <QApplication>
#include <QCoreApplication>
#include <QDebug>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLoggingCategory>
#include <QMenu>
#include <QMenuBar>
#include <QMouseEvent>
#include <QPushButton>
#include <QResizeEvent>
#include <QToolButton>
#include <QWindow>

// 和 qtapp.shell / qtapp.plugin / qtapp.crashpad 保持同一套分类，
// 排查时可以用 QT_LOGGING_RULES="qtapp.widgets.warning=false" 单独关掉。
// 置于 #ifdef 内：Windows 分支不需要这个分类，否则会留下未使用的静态对象。
Q_LOGGING_CATEGORY(lcAppWindow, "qtapp.widgets", QtInfoMsg)

#include <array>

namespace Widgets {

#ifndef Q_OS_MACOS

namespace {
// 阴影占用的边距：窗口矩形到可见边框的距离，也是 drop shadow 的模糊半径。
constexpr int kShadowPadding = 10;
// 可见边框内还能抓住拉伸的宽度。阴影边距本身也算热区，所以总热区是两者之和。
constexpr int kResizeBorder = 4;
// 手工兜底缩放的下限，实际取它和 setMinimumSize() 的较大值。
constexpr int kMinWindowWidth = 480;
constexpr int kMinWindowHeight = 320;
// 四周热区的数量：上下左右四条边加四个角。
constexpr int kResizeHandleCount = 8;
// 标题栏里命令按钮的对象名，样式表按 #TitleBarButton 命中，换名字要一起改 QSS。
constexpr auto kTitleBarButtonObjectName = "TitleBarButton";
} // namespace

class AppWindow::AppWindowPrivate
{
public:
    // 盖在窗口边缘的一圈透明热区：只负责光标反馈和发起拉伸，不参与绘制。
    struct ResizeHandle
    {
        Qt::Edges edges;
        QWidget *widget = nullptr;
    };

    explicit AppWindowPrivate(AppWindow *q)
        : q_ptr(q)
        , titleButton(new QPushButton(qApp->windowIcon(), qAppName(), q))
        , titleWidget(new QWidget(q))
        , centralContainer(new QWidget(q))
        , titleBarContainer(new QWidget(q))
    {
        titleButton->setObjectName(QStringLiteral("AppButton"));
        titleButton->setCursor(Qt::PointingHandCursor);
        titleButton->setFlat(true);

        centralContainer->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        centralContainer->setObjectName("CentralWidget");
    }

    // 控件必须等 d_ptr 赋值之后再建：构造期任何 setVisible() 都会发出 Hide 事件，
    // 经由事件过滤器回到 AppWindow 时 d_ptr 还是空的。
    void setupUi()
    {
        initTitleButtons();
        buildTitleWidget();
        buildResizeHandles();
        buildLayout();
    }

    // 标题栏以外的内容区，setCentralWidget() 往这里的布局里换控件。
    void buildLayout()
    {
        centralLayout = new QHBoxLayout(centralContainer);
        centralLayout->setContentsMargins({});
        centralLayout->setSpacing(0);

        // 阴影必须挂在"内容整体"这一层：QGraphicsDropShadowEffect 按控件的 alpha
        // 轮廓做模糊，标题栏和内容区的圆角才会跟着一起圆，自己画矩形渐变带做不到。
        frame = new QWidget(q_ptr);
        // #Frame 的兜底底色在 shell.css：半像素行不至于透出窗口后面的桌面。
        frame->setObjectName("Frame");
        Utils::addDropShadow(frame, {.margin = kShadowPadding});

        auto *frameLayout = new QVBoxLayout(frame);
        frameLayout->setContentsMargins({});
        frameLayout->setSpacing(0);
        frameLayout->addWidget(titleWidget);
        frameLayout->addWidget(centralContainer);

        outerLayout = new QHBoxLayout(q_ptr);
        outerLayout->setSpacing(0);
        outerLayout->setContentsMargins(shadowPadding, shadowPadding, shadowPadding, shadowPadding);
        outerLayout->addWidget(frame);

        updateHandleGeometry();
        raiseHandles();
    }

    // 最大化时窗口铺满屏幕，阴影边距必须收掉，否则四周会留一圈透明缝。
    // 同时摘掉 effect：最大化时本来就没有阴影，没必要每帧为整窗做离屏渲染。
    void setShadowVisible(bool visible)
    {
        const int value = visible ? kShadowPadding : 0;
        if (shadowPadding == value)
            return;

        shadowPadding = value;
        outerLayout->setContentsMargins(value, value, value, value);

        if (visible)
            Utils::addDropShadow(frame, {.margin = kShadowPadding});
        else
            frame->setGraphicsEffect(nullptr);

        updateHandleGeometry();
    }

    // 标题栏里那类"看起来像按钮"的控件：命令用文字，窗口按钮是自绘图标。
    auto createTitleBarButton() -> QToolButton *
    {
        auto *button = new QToolButton(q_ptr);
        button->setObjectName(QLatin1String(kTitleBarButtonObjectName));
        button->setToolButtonStyle(Qt::ToolButtonTextOnly);
        button->setCursor(Qt::PointingHandCursor);
        button->setFocusPolicy(Qt::NoFocus);
        return button;
    }

    void appendMenu(QMenu *menu)
    {
        auto *button = createTitleBarButton();
        button->setPopupMode(QToolButton::InstantPopup);
        button->setMenu(menu);
        button->setText(menu->title());
        menuLayout->addWidget(button);
    }

    void appendChromeAction(QAction *action)
    {
        auto *button = createTitleBarButton();
        button->setDefaultAction(action);
        // setDefaultAction() 只同步文字/图标/提示，显示方式得再钉一次。
        button->setToolButtonStyle(Qt::ToolButtonTextOnly);
        actionLayout->addWidget(button);
    }

    void buildResizeHandles()
    {
        struct HandleDefinition
        {
            Qt::Edges edges;
            Qt::CursorShape cursor;
        };

        const HandleDefinition definitions[kResizeHandleCount] = {
            {Qt::TopEdge, Qt::SizeVerCursor},
            {Qt::BottomEdge, Qt::SizeVerCursor},
            {Qt::LeftEdge, Qt::SizeHorCursor},
            {Qt::RightEdge, Qt::SizeHorCursor},
            {Qt::TopEdge | Qt::LeftEdge, Qt::SizeFDiagCursor},
            {Qt::BottomEdge | Qt::RightEdge, Qt::SizeFDiagCursor},
            {Qt::TopEdge | Qt::RightEdge, Qt::SizeBDiagCursor},
            {Qt::BottomEdge | Qt::LeftEdge, Qt::SizeBDiagCursor},
        };

        for (int i = 0; i < kResizeHandleCount; ++i) {
            auto *handle = new QWidget(q_ptr);
            handle->setCursor(definitions[i].cursor);
            // 热区不该参与 Tab 焦点链，也不该在切换焦点时露出虚线框。
            handle->setFocusPolicy(Qt::NoFocus);
            handle->installEventFilter(q_ptr);

            // 可见性交给 updateHandleGeometry() 统一设置：子控件默认就不可见，
            // 这里再 hide() 会多发一次 Hide 事件，而此刻 d_ptr 还没构造完。
            handles[static_cast<size_t>(i)] = {definitions[i].edges, handle};
        }
    }

    // 热区从窗口外沿往里铺：先吃掉阴影边距，再压进可见边框 kResizeBorder 像素。
    // 这样半透明阴影区也能抓住，且不会盖住标题栏里的按钮。
    void updateHandleGeometry()
    {
        const QRect area = q_ptr->rect();
        const int width = area.width();
        const int height = area.height();
        const int thickness = qMin(shadowPadding + kResizeBorder, qMin(width, height) / 2);
        const bool visible = isResizable && thickness > 0;

        placeHandle(0, visible, QRect(thickness, 0, width - 2 * thickness, thickness));
        placeHandle(
            1, visible, QRect(thickness, height - thickness, width - 2 * thickness, thickness));
        placeHandle(2, visible, QRect(0, thickness, thickness, height - 2 * thickness));
        placeHandle(
            3, visible, QRect(width - thickness, thickness, thickness, height - 2 * thickness));
        placeHandle(4, visible, QRect(0, 0, thickness, thickness));
        placeHandle(5, visible, QRect(width - thickness, height - thickness, thickness, thickness));
        placeHandle(6, visible, QRect(width - thickness, 0, thickness, thickness));
        placeHandle(7, visible, QRect(0, height - thickness, thickness, thickness));
    }

    // 只在建完布局后抬一次：内容控件都比热区晚建，会压在热区上面。
    // 不能放进 updateHandleGeometry()——那是每次 resize 都会走的路径。
    void raiseHandles()
    {
        for (const auto &handle : handles)
            handle.widget->raise();
    }

    [[nodiscard]] auto edgesFor(const QObject *watched) const -> Qt::Edges
    {
        for (const auto &handle : handles) {
            if (handle.widget == watched)
                return handle.edges;
        }
        return {};
    }

    // 交给窗口管理器做交互式拉伸：能拿到贴边吸附、跟手的光标这些原生手感。
    // 平台不支持时退回自己算，X11 上部分窗口管理器会拒绝 _NET_WM_MOVERESIZE。
    auto beginResize(const QMouseEvent *event, QWidget *handle, Qt::Edges edges) -> bool
    {
        if (!isResizable)
            return false;

        if (QWindow *window = q_ptr->windowHandle()) {
            if (window->startSystemResize(edges))
                return true;
        }

        manualEdges = edges;
        manualPressGlobal = event->globalPosition().toPoint();
        manualStartGeometry = q_ptr->geometry();
        handle->grabMouse();
        return true;
    }

    auto continueManualResize(const QMouseEvent *event) -> bool
    {
        if (manualEdges == Qt::Edges())
            return false;

        const QPoint delta = event->globalPosition().toPoint() - manualPressGlobal;
        const QSize minimum(
            qMax(q_ptr->minimumWidth(), kMinWindowWidth),
            qMax(q_ptr->minimumHeight(), kMinWindowHeight));

        // 基于按下时的快照算，不用增量：同一次事件里同时改左边和右边会互相叠加导致窗口漂移。
        QRect geometry = manualStartGeometry;
        if (manualEdges & Qt::LeftEdge) {
            geometry.setLeft(
                qMin(geometry.left() + delta.x(), geometry.right() - minimum.width() + 1));
        }
        if (manualEdges & Qt::RightEdge) {
            geometry.setRight(
                qMax(geometry.right() + delta.x(), geometry.left() + minimum.width() - 1));
        }
        if (manualEdges & Qt::TopEdge) {
            geometry.setTop(
                qMin(geometry.top() + delta.y(), geometry.bottom() - minimum.height() + 1));
        }
        if (manualEdges & Qt::BottomEdge) {
            geometry.setBottom(
                qMax(geometry.bottom() + delta.y(), geometry.top() + minimum.height() - 1));
        }

        q_ptr->setGeometry(geometry);
        return true;
    }

    auto endManualResize() -> bool
    {
        if (manualEdges == Qt::Edges())
            return false;

        manualEdges = {};
        if (QWidget *grabber = QWidget::mouseGrabber())
            grabber->releaseMouse();
        return true;
    }

    AppWindow *q_ptr = nullptr;

    QPushButton *titleButton = nullptr;
    QToolButton *minButton = nullptr;
    QToolButton *maxButton = nullptr;
    QToolButton *restoreButton = nullptr;
    QToolButton *closeButton = nullptr;
    QWidget *titleWidget = nullptr;
    QWidget *centralContainer = nullptr;
    QWidget *titleBarContainer = nullptr;
    QWidget *menuContainer = nullptr;
    QHBoxLayout *menuLayout = nullptr;
    QWidget *actionContainer = nullptr;
    QHBoxLayout *actionLayout = nullptr;
    QWidget *frame = nullptr;
    QHBoxLayout *centralLayout = nullptr;
    QHBoxLayout *titleBarLayout = nullptr;
    QHBoxLayout *outerLayout = nullptr;
    std::array<ResizeHandle, kResizeHandleCount> handles{};

    int shadowPadding = kShadowPadding;
    bool maxRestoreVisible = true;
    bool isResizable = true;

    // 手工兜底拉伸用：窗口管理器拒绝系统拉伸时才用到。
    Qt::Edges manualEdges;
    QPoint manualPressGlobal;
    QRect manualStartGeometry;

private:
    void placeHandle(int index, bool visible, const QRect &rect)
    {
        auto *handle = handles[static_cast<size_t>(index)].widget;
        handle->setGeometry(rect);
        handle->setVisible(visible && rect.width() > 0 && rect.height() > 0);
    }

    void buildTitleWidget()
    {
        titleWidget->setObjectName("TitleWidget");
        auto *layout = new QHBoxLayout(titleWidget);
        layout->setContentsMargins(12, 6, 10, 6);
        layout->setSpacing(8);

        // 左侧放菜单，右侧依次是自定义控件、命令按钮、窗口按钮。
        menuContainer = new QWidget(q_ptr);
        menuLayout = new QHBoxLayout(menuContainer);
        menuLayout->setContentsMargins({});
        menuLayout->setSpacing(4);

        actionContainer = new QWidget(q_ptr);
        actionLayout = new QHBoxLayout(actionContainer);
        actionLayout->setContentsMargins({});
        actionLayout->setSpacing(4);

        layout->addWidget(titleButton);
        layout->addWidget(menuContainer);
        layout->addStretch();
        layout->addWidget(titleBarContainer);
        layout->addWidget(actionContainer);
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
        const QString basePath = QStringLiteral(":/titlebar/titlebutton-");

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

AppWindow::AppWindow(QWidget *parent)
    : QWidget(parent)
    , d_ptr(std::make_unique<AppWindowPrivate>(this))
{
    // 先把控件建好再动窗口标志：setWindowFlags() 可能触发 WindowStateChange，
    // 而 changeEvent() 会去改 rootLayout 的边距。
    d_ptr->setupUi();

    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    retranslateUi();
    setupConnections();
    updateMaxRestoreButtons();

    resize(1000, 618);
    d_ptr->updateHandleGeometry();
}

AppWindow::~AppWindow() = default;

void AppWindow::setRestoreMaxButtonVisible(bool visible)
{
    d_ptr->maxRestoreVisible = visible;
    updateMaxRestoreButtons();
}

void AppWindow::setMinButtonVisible(bool visible)
{
    d_ptr->minButton->setVisible(visible);
}

void AppWindow::setResizable(bool resizable)
{
    d_ptr->isResizable = resizable;
    d_ptr->updateHandleGeometry();
}

void AppWindow::setCentralWidget(QWidget *centralWidget)
{
    // 下面这几个 setter 的指针参数都是必填的：传空是调用方的错，喊一声再退出。
    // 与 ModalOverlay 不同，这里窗口自身状态完好，后续调用不受影响。
    QTC_ASSERT(centralWidget, return);

    // 换内容时把上一个摘出布局并隐藏：直接删会让调用方手里的指针悬空。
    if (auto *item = d_ptr->centralLayout->takeAt(0)) {
        if (QWidget *previous = item->widget())
            previous->hide();
        delete item;
    }
    d_ptr->centralLayout->addWidget(centralWidget);
}

void AppWindow::addChromeAction(QAction *action)
{
    QTC_ASSERT(action, return);

    d_ptr->appendChromeAction(action);
}

void AppWindow::addMenu(QMenu *menu)
{
    QTC_ASSERT(menu, return);

    d_ptr->appendMenu(menu);
}

void AppWindow::setTitleBarWidget(QWidget *titleBar)
{
    QTC_ASSERT(titleBar, return);

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

auto AppWindow::shadowPadding() const -> int
{
    return d_ptr->shadowPadding;
}

auto AppWindow::isInTitleBar(const QPoint &pos) const -> bool
{
    // 用 mapTo 而不是手工加阴影边距：布局调整后仍然正确。
    const QRect rect(d_ptr->titleWidget->mapTo(this, QPoint(0, 0)), d_ptr->titleWidget->size());
    return rect.contains(pos);
}

void AppWindow::updateMaxRestoreButtons()
{
    const bool maximized = isMaximized() || isFullScreen();
    d_ptr->maxButton->setVisible(d_ptr->maxRestoreVisible && !maximized);
    d_ptr->restoreButton->setVisible(d_ptr->maxRestoreVisible && maximized);
}

void AppWindow::onShowMaximized()
{
    // 先收掉阴影边距再最大化：否则窗口会先按带边距的尺寸闪一下，再被撑到屏幕外。
    d_ptr->setShadowVisible(false);
    showMaximized();
}

void AppWindow::onShowNormal()
{
    d_ptr->setShadowVisible(true);
    showNormal();
}

void AppWindow::onToggleMaximized()
{
    // 最大化按钮被藏起来时（对话框外壳），双击标题栏也不该改变窗口状态。
    if (!d_ptr->maxRestoreVisible)
        return;

    if (isMaximized() || isFullScreen())
        onShowNormal();
    else
        onShowMaximized();
}

void AppWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && isInTitleBar(event->position().toPoint())) {
        // 交给窗口管理器做交互式移动。Wayland 下 setPosition() 无效，只有这条路能拖动
        // 窗口；顺带拿到贴边分屏、最大化状态下拖出还原这些原生行为。
        if (QWindow *window = windowHandle()) {
            if (window->startSystemMove())
                return;
        }
    }

    QWidget::mousePressEvent(event);
}

void AppWindow::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (isInTitleBar(event->position().toPoint()))
        onToggleMaximized();

    QWidget::mouseDoubleClickEvent(event);
}

void AppWindow::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    d_ptr->updateHandleGeometry();
}

void AppWindow::changeEvent(QEvent *event)
{
    QWidget::changeEvent(event);

    switch (event->type()) {
    case QEvent::LanguageChange:
        retranslateUi();
        break;
    case QEvent::WindowStateChange:
        // 这里兜住所有改变窗口状态的途径（快捷键、窗口管理器），不只是按钮。
        d_ptr->setShadowVisible(!isMaximized() && !isFullScreen());
        updateMaxRestoreButtons();
        break;
    default:
        break;
    }
}

bool AppWindow::eventFilter(QObject *watched, QEvent *event)
{
    // d_ptr 在 AppWindowPrivate 构造完成前仍是空的，而构造期发出的 Hide/ChildAdded
    // 事件会经由事件过滤器回到这里；先挡住这段空窗期，不依赖 setupUi() 的调用时机。
    if (!d_ptr)
        return QWidget::eventFilter(watched, event);

    const Qt::Edges edges = d_ptr->edgesFor(watched);
    if (edges != Qt::Edges()) {
        switch (event->type()) {
        case QEvent::MouseButtonPress: {
            auto *mouseEvent = static_cast<QMouseEvent *>(event);
            if (mouseEvent->button() == Qt::LeftButton
                && d_ptr->beginResize(mouseEvent, static_cast<QWidget *>(watched), edges)) {
                return true;
            }
            break;
        }
        case QEvent::MouseMove:
            if (d_ptr->continueManualResize(static_cast<QMouseEvent *>(event)))
                return true;
            break;
        case QEvent::MouseButtonRelease:
            if (d_ptr->endManualResize())
                return true;
            break;
        case QEvent::MouseButtonDblClick:
            // 热区盖住了标题栏上沿，双击要透传成最大化，否则那条边是死区。
            if (edges.testFlag(Qt::TopEdge))
                onToggleMaximized();
            break;
        default:
            break;
        }
    }

    return QWidget::eventFilter(watched, event);
}

void AppWindow::setupConnections()
{
    connect(d_ptr->minButton, &QToolButton::clicked, this, &AppWindow::showMinimized);
    connect(d_ptr->maxButton, &QToolButton::clicked, this, &AppWindow::onShowMaximized);
    connect(d_ptr->restoreButton, &QToolButton::clicked, this, &AppWindow::onShowNormal);
    // 排队而不是直接调 close()：closeEvent 里可能销毁控件，别在按钮自己的信号栈里做。
    connect(d_ptr->closeButton, &QToolButton::clicked, this, &AppWindow::close, Qt::QueuedConnection);
    connect(this, &AppWindow::windowTitleChanged, d_ptr->titleButton, &QPushButton::setText);
    connect(this, &AppWindow::windowTitleChanged, d_ptr->titleButton, &QPushButton::setToolTip);
    connect(this, &AppWindow::windowIconChanged, d_ptr->titleButton, &QPushButton::setIcon);
}

void AppWindow::retranslateUi()
{
    d_ptr->minButton->setToolTip(tr("Minimize"));
    d_ptr->maxButton->setToolTip(tr("Maximize"));
    d_ptr->restoreButton->setToolTip(tr("Restore"));
    d_ptr->closeButton->setToolTip(tr("Close"));
}
#else
AppWindow::AppWindow(QWidget *parent)
    : QMainWindow(parent)
{}

AppWindow::~AppWindow() = default;

auto AppWindow::shadowPadding() const -> int
{
    // 系统装饰下窗口没有自绘阴影占位，调用方拿到的就该是 0。
    return 0;
}

void AppWindow::setResizable(bool resizable)
{
    // 系统装饰下能否缩放由窗口管理器决定，这里改不了它，只能让调用方知道。
    if (!resizable) {
        qCWarning(lcAppWindow).noquote()
            << "AppWindow::setResizable(false): the window manager owns resizing on macOS and "
               "it cannot be disabled; constrain the layout and the minimum/maximum sizes "
               "instead";
    }
}

void AppWindow::setTitleBarWidget(QWidget *widget)
{
    QTC_ASSERT(widget, return);

    // 带上类名：这个调用不报错、不崩溃，只是界面少一块，没有类名几乎查不出来。
    qCWarning(lcAppWindow).noquote()
        << QStringLiteral(
               "AppWindow::setTitleBarWidget(%1): macOS has no custom title bar, "
               "the widget is dropped; use addChromeAction() for code that has to "
               "work on every platform")
               .arg(QLatin1String(widget->metaObject()->className()));
}

void AppWindow::addMenu(QMenu *menu)
{
    if (menu)
        menuBar()->addMenu(menu);
}

void AppWindow::addChromeAction(QAction *action)
{
    QTC_ASSERT(action, return);

    // macOS 没有标题栏可以放按钮，命令只能落进系统顶栏的菜单。
    // 菜单按需建一次，用 objectName 找回来，省得为它再加一个成员。
    const auto kMenuName = QStringLiteral("ChromeActionsMenu");
    auto *menu = menuBar()->findChild<QMenu *>(kMenuName);
    if (!menu) {
        menu = menuBar()->addMenu(tr("View"));
        menu->setObjectName(kMenuName);
    }
    menu->addAction(action);
}
#endif

} // namespace Widgets
