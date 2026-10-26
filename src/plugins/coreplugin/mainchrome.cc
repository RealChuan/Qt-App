#include "mainwindow_p.hpp"

#include <utils/singletonmanager.hpp>
#include <utils/uihelpers.hpp>
#include <widgets/modaloverlay.hpp>

#include <QAction>
#include <QApplication>
#include <QButtonGroup>
#include <QFrame>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QPushButton>
#include <QScreen>
#include <QScrollArea>
#include <QSize>
#include <QStackedWidget>
#include <QSystemTrayIcon>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

#include <algorithm>

namespace Core {

namespace {

constexpr int kWindowWidth = 1060;
constexpr int kWindowHeight = 680;
constexpr int kMinWindowWidth = 900;
constexpr int kMinWindowHeight = 600;
constexpr int kScreenMargin = 80;

// Prefer the designed size, but never open taller or wider than the screen.
auto preferredWindowSize() -> QSize
{
    QSize preferred(kWindowWidth, kWindowHeight);
    if (const QScreen *screen = QGuiApplication::primaryScreen()) {
        const QSize available = screen->availableGeometry().size();
        preferred.setWidth(std::min(preferred.width(), available.width() - kScreenMargin));
        preferred.setHeight(std::min(preferred.height(), available.height() - kScreenMargin));
    }
    return preferred;
}

} // namespace

MainWindow::MainWindowPrivate::MainWindowPrivate(MainWindow *q)
    : q_ptr(q)
{}

MainWindow::MainWindowPrivate::~MainWindowPrivate() = default;

// 只搭结构与父子关系；文案在 retranslateUi()，信号在 setupConnections()。
void MainWindow::MainWindowPrivate::buildChrome()
{
    stack = new QStackedWidget(q_ptr);
    group = new QButtonGroup(q_ptr);
    group->setExclusive(true);

    auto *sidebar = new QWidget(q_ptr);
    sidebar->setObjectName("MenuWidget");
    auto *sidebarLayout = new QVBoxLayout(sidebar);
    sidebarLayout->setContentsMargins(10, 12, 10, 8);
    sidebarLayout->setSpacing(2);

    // 命令面板入口单独成型：它不是页面，不该混在页面按钮里。
    // 用两个 QLabel 拼出“提示文字 + 键位”，比一整行纯文本更接近输入框。
    commandButton = new QPushButton(sidebar);
    commandButton->setObjectName("CommandButton");
    commandButton->setCursor(Qt::PointingHandCursor);
    auto *commandLayout = new QHBoxLayout(commandButton);
    commandLayout->setContentsMargins(10, 3, 6, 3);
    commandLayout->setSpacing(6);
    commandHint = new QLabel(commandButton);
    commandHint->setObjectName("CommandHint");
    commandShortcut = new QLabel(QLatin1String(kCommandShortcut), commandButton);
    commandShortcut->setObjectName("CommandShortcut");
    commandLayout->addWidget(commandHint);
    commandLayout->addStretch();
    commandLayout->addWidget(commandShortcut);
    sidebarLayout->addWidget(commandButton);
    sidebarLayout->addSpacing(10);

    // 只有页面列表滚动：命令入口和版本号都固定，不跟着列表走。
    auto *sidebarScroll = new QScrollArea(sidebar);
    sidebarScroll->setObjectName("SidebarScroll");
    sidebarScroll->setWidgetResizable(true);
    sidebarScroll->setFrameShape(QFrame::NoFrame);
    sidebarScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    sidebarScroll->viewport()->setAutoFillBackground(false);

    auto *scrollContent = new QWidget(sidebarScroll);
    scrollContent->setObjectName("SidebarScrollContent");
    auto *scrollLayout = new QVBoxLayout(scrollContent);
    scrollLayout->setContentsMargins({});
    scrollLayout->setSpacing(2);

    // 三个分组各自带标题，空分组连标题一起隐藏。
    for (Section &section : sections) {
        section.label = new QLabel(scrollContent);
        section.label->setObjectName("SidebarSection");

        section.container = new QWidget(scrollContent);
        section.layout = new QVBoxLayout(section.container);
        section.layout->setContentsMargins({});
        section.layout->setSpacing(2);

        scrollLayout->addWidget(section.label);
        scrollLayout->addWidget(section.container);
    }

    scrollLayout->addStretch();

    sidebarScroll->setWidget(scrollContent);
    sidebarLayout->addWidget(sidebarScroll, 1);

    footer = new QLabel(sidebar);
    footer->setObjectName("SidebarFooter");
    sidebarLayout->addWidget(footer);

    // 两个高频开关：主题与语言。真正的设置项在“外观”页里，
    // 这里只是让最常用的两个动作不必先进设置页。
    // 交给 addChromeAction()：Windows/Ubuntu 渲染成标题栏按钮，macOS 进系统菜单。
    themeAction = new QAction(q_ptr);
    languageAction = new QAction(q_ptr);
    q_ptr->addChromeAction(themeAction);
    q_ptr->addChromeAction(languageAction);

    status = new QLabel(q_ptr);
    status->setObjectName("StatusLabel");
    status->hide();

    statusTimer = new QTimer(q_ptr);
    statusTimer->setSingleShot(true);

    auto *content = new QWidget(q_ptr);
    auto *contentLayout = new QVBoxLayout(content);
    contentLayout->setContentsMargins({});
    contentLayout->setSpacing(0);
    contentLayout->addWidget(stack, 1);
    contentLayout->addWidget(status);

    central = new QWidget(q_ptr);
    auto *layout = new QHBoxLayout(central);
    layout->setContentsMargins({});
    layout->setSpacing(0);
    layout->addWidget(sidebar);
    layout->addWidget(content, 1);

    q_ptr->setCentralWidget(central);
    q_ptr->setMinimumSize(kMinWindowWidth, kMinWindowHeight);
    q_ptr->resize(preferredWindowSize());
    Utils::centerOnScreen(q_ptr);
}

void MainWindow::MainWindowPrivate::buildTray()
{
    if (!QSystemTrayIcon::isSystemTrayAvailable())
        return;

    trayMenu = new QMenu(q_ptr);
    quitAction = trayMenu->addAction(QString());

    tray = new QSystemTrayIcon(q_ptr);
    tray->setIcon(qApp->windowIcon());
    tray->setContextMenu(trayMenu);
    tray->show();

    qApp->setQuitOnLastWindowClosed(false);
}

// 遮罩盖住整个窗口（含标题栏），模糊只挂在内容区：
// 两者不能是同一个控件，否则遮罩自己也会被糊掉。
void MainWindow::MainWindowPrivate::buildOverlay()
{
    overlay = std::make_unique<Widgets::ModalOverlay>(q_ptr);
    overlay->setBlurTarget(central);
}

void MainWindow::MainWindowPrivate::setBlocked(bool blocked)
{
    // 阴影留白在最大化/还原时会变，每次都重新取，别让遮罩留下一条错位的边。
    overlay->setInset(q_ptr->shadowPadding());
    // 两个开关在遮挡发生的这一刻读：改完设置，下一次弹窗就生效。
    overlay->setMaskVisible(blocked && Utils::widgetManager()->showMask());
    overlay->setBlurActive(blocked && Utils::widgetManager()->blurBackground());
}

} // namespace Core
