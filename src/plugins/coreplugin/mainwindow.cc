#include "mainwindow.hpp"

#include "commandpalette.hpp"

#include <shell/registry.hpp>
#include <utils/utils.hpp>

#include <QApplication>
#include <QButtonGroup>
#include <QCloseEvent>
#include <QFrame>
#include <QHash>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QPointer>
#include <QPushButton>
#include <QShortcut>
#include <QStackedWidget>
#include <QSystemTrayIcon>
#include <QTimer>
#include <QVBoxLayout>

#include <limits>

namespace Core {

class MainWindow::MainWindowPrivate
{
public:
    explicit MainWindowPrivate(MainWindow *q)
        : q_ptr(q)
    {}

    void buildChrome();
    void setupTray();
    void addPage(Shell::IPage *page);

    auto buttonForPage(Shell::IPage *page) -> QPushButton *
    {
        auto *button = new QPushButton(page->title(), q_ptr);
        button->setCheckable(true);
        button->setLayoutDirection(Qt::LeftToRight);
        button->setProperty("class",
                            page->area() == Shell::PageArea::Workspace ? "GroupButton"
                                                                       : "GroupItemButton");
        if (!page->icon().isNull())
            button->setIcon(page->icon());
        return button;
    }

    MainWindow *q_ptr = nullptr;

    QVBoxLayout *workspaceLayout = nullptr;
    QVBoxLayout *toolLayout = nullptr;
    QStackedWidget *stack = nullptr;
    QButtonGroup *group = nullptr;
    QLabel *status = nullptr;
    QPointer<CommandPalette> palette;
    QSystemTrayIcon *tray = nullptr;

    QHash<QString, int> pageIndex;
    QHash<QString, int> pageOrder;
    bool workspaceSelected = false;
    int selectedOrder = std::numeric_limits<int>::max();
    bool quitting = false;
};

void MainWindow::MainWindowPrivate::buildChrome()
{
    stack = new QStackedWidget(q_ptr);
    group = new QButtonGroup(q_ptr);
    group->setExclusive(true);

    workspaceLayout = new QVBoxLayout;
    toolLayout = new QVBoxLayout;
    for (QVBoxLayout *layout : {workspaceLayout, toolLayout}) {
        layout->setContentsMargins({});
        layout->setSpacing(0);
    }

    auto *sidebar = new QWidget(q_ptr);
    sidebar->setObjectName("MenuWidget");
    auto *sidebarLayout = new QVBoxLayout(sidebar);
    sidebarLayout->setContentsMargins({});
    sidebarLayout->setSpacing(0);

    // 命令面板入口单独成型：它不是页面，不该混在页面按钮里。
    auto *commandButton = new QPushButton(QObject::tr("Search commands   Ctrl+K"), sidebar);
    commandButton->setObjectName("CommandButton");
    commandButton->setCursor(Qt::PointingHandCursor);
    QObject::connect(commandButton, &QPushButton::clicked, q_ptr, &MainWindow::openCommandPalette);
    sidebarLayout->addWidget(commandButton);

    sidebarLayout->addLayout(workspaceLayout);

    auto *separator = new QFrame(sidebar);
    separator->setObjectName("SidebarSeparator");
    separator->setFrameShape(QFrame::HLine);
    sidebarLayout->addWidget(separator);

    sidebarLayout->addLayout(toolLayout);
    sidebarLayout->addStretch();

    status = new QLabel(q_ptr);
    status->hide();
    status->setObjectName("StatusLabel");

    auto *content = new QWidget(q_ptr);
    auto *contentLayout = new QVBoxLayout(content);
    contentLayout->setContentsMargins({});
    contentLayout->setSpacing(0);
    contentLayout->addWidget(stack, 1);
    contentLayout->addWidget(status);

    auto *central = new QWidget(q_ptr);
    auto *layout = new QHBoxLayout(central);
    layout->setContentsMargins({});
    layout->setSpacing(0);
    layout->addWidget(sidebar);
    layout->addWidget(content, 1);

    q_ptr->setCentralWidget(central);
    q_ptr->setMinimumSize(1000, 618);
    Utils::windowCenter(q_ptr);

    QObject::connect(group, &QButtonGroup::idClicked, q_ptr, [this](int id) {
        stack->setCurrentIndex(id);
    });
}

void MainWindow::MainWindowPrivate::setupTray()
{
    if (!QSystemTrayIcon::isSystemTrayAvailable())
        return;

    auto *menu = new QMenu(q_ptr);
    menu->addAction(QObject::tr("Quit"), qApp, [] { Utils::quitApplication(); },
                    Qt::QueuedConnection);

    tray = new QSystemTrayIcon(q_ptr);
    tray->setToolTip(QObject::tr("Qt-App"));
    tray->setIcon(qApp->windowIcon());
    tray->setContextMenu(menu);
    tray->show();

    QObject::connect(tray,
                     &QSystemTrayIcon::activated,
                     q_ptr,
                     [this](QSystemTrayIcon::ActivationReason reason) {
                         if (reason == QSystemTrayIcon::Trigger
                             || reason == QSystemTrayIcon::DoubleClick) {
                             Utils::restoreAndActivate(q_ptr);
                         }
                     });

    qApp->setQuitOnLastWindowClosed(false);
}

void MainWindow::MainWindowPrivate::addPage(Shell::IPage *page)
{
    if (!page || pageIndex.contains(page->id()))
        return;

    QWidget *widget = page->widget();
    if (!widget)
        return;

    QPushButton *button = buttonForPage(page);
    QVBoxLayout *layout = page->area() == Shell::PageArea::Workspace ? workspaceLayout
                                                                     : toolLayout;

    // 同组内按 IPage::order() 插入，插件注册顺序不影响侧边栏顺序。
    int position = layout->count();
    for (int i = 0; i < layout->count(); ++i) {
        auto *existing = qobject_cast<QPushButton *>(layout->itemAt(i)->widget());
        if (!existing)
            continue;
        const int order = pageOrder.value(existing->property("pageId").toString(), 100);
        if (page->order() < order) {
            position = i;
            break;
        }
    }
    button->setProperty("pageId", page->id());
    layout->insertWidget(position, button);

    const int index = stack->addWidget(widget);
    group->addButton(button, index);
    pageIndex.insert(page->id(), index);
    pageOrder.insert(page->id(), page->order());

    QObject::connect(button, &QPushButton::clicked, q_ptr, [this, index] {
        stack->setCurrentIndex(index);
    });

    // 默认页取 order() 最小的 Workspace 页。
    if (page->area() == Shell::PageArea::Workspace
        && (!workspaceSelected || page->order() < selectedOrder)) {
        if (auto *selected = group->button(index))
            selected->setChecked(true);
        stack->setCurrentIndex(index);
        workspaceSelected = true;
        selectedOrder = page->order();
    }
}

MainWindow::MainWindow(QWidget *parent)
    : Widgets::MainWidget(parent)
    , d_ptr(std::make_unique<MainWindowPrivate>(this))
{
    d_ptr->buildChrome();
    d_ptr->setupTray();

    auto *registry = Shell::Registry::instance();
    connect(registry, &Shell::Registry::pagesChanged, this, [this] {
        const QList<Shell::IPage *> pages = Shell::Registry::instance()->pages();
        for (Shell::IPage *page : pages)
            d_ptr->addPage(page);
    });

    const QList<Shell::IPage *> pages = registry->pages();
    for (Shell::IPage *page : pages)
        d_ptr->addPage(page);

    auto *shortcut = new QShortcut(QKeySequence(QStringLiteral("Ctrl+K")), this);
    connect(shortcut, &QShortcut::activated, this, &MainWindow::openCommandPalette);

    installEventFilter(this);
}

MainWindow::~MainWindow() = default;

auto MainWindow::mainWindow() -> QWidget *
{
    return this;
}

void MainWindow::showPage(QAnyStringView pageId)
{
    const auto it = d_ptr->pageIndex.constFind(pageId.toString());
    if (it == d_ptr->pageIndex.cend())
        return;

    d_ptr->stack->setCurrentIndex(it.value());
    if (auto *button = d_ptr->group->button(it.value()))
        button->setChecked(true);
}

void MainWindow::showStatusMessage(QAnyStringView text, int timeoutMs)
{
    d_ptr->status->setText(text.toString());
    d_ptr->status->show();
    QTimer::singleShot(timeoutMs, d_ptr->status, &QLabel::hide);
}

auto MainWindow::askForRestart(QAnyStringView reason) -> bool
{
    const auto answer = QMessageBox::question(this,
                                              tr("Restart required"),
                                              reason.toString(),
                                              QMessageBox::Yes | QMessageBox::No,
                                              QMessageBox::Yes);
    if (answer != QMessageBox::Yes)
        return false;

    qApp->setProperty("restart", true);
    Utils::quitApplication();
    return true;
}

void MainWindow::openCommandPalette()
{
    if (!d_ptr->palette)
        d_ptr->palette = new CommandPalette(this);
    d_ptr->palette->openPalette();
}

void MainWindow::prepareShutdown()
{
    d_ptr->quitting = true;

    // 托盘图标必须显式 hide + delete：只等进程结束时回收，通知区域会留下
    // 一个"幽灵图标"，尤其是调试构建退出较慢时非常明显。
    if (d_ptr->tray) {
        d_ptr->tray->hide();
        d_ptr->tray->setContextMenu(nullptr);
        delete d_ptr->tray;
        d_ptr->tray = nullptr;
    }

    if (d_ptr->palette)
        d_ptr->palette->hide();

    hide();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    // 关闭窗口即退出：否则进程会带着托盘图标留在后台，用户找不到退出入口。
    if (!d_ptr->quitting) {
        d_ptr->quitting = true;
        Utils::quitApplication();
    }
    Widgets::MainWidget::closeEvent(event);
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    return Widgets::MainWidget::eventFilter(watched, event);
}

} // namespace Core
