#include "mainwindow.hpp"

#include "commandpalette.hpp"

#include <shell/registry.hpp>
#include <utils/appinfo.hpp>
#include <utils/singletonmanager.hpp>
#include <utils/theme.hpp>
#include <utils/uihelpers.hpp>
#include <utils/utils.hpp>

#include <QAction>
#include <QApplication>
#include <QButtonGroup>
#include <QCloseEvent>
#include <QEvent>
#include <QGraphicsBlurEffect>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QPointer>
#include <QPushButton>
#include <QShortcut>
#include <QSignalBlocker>
#include <QSizePolicy>
#include <QStackedWidget>
#include <QSystemTrayIcon>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>

#include <limits>

namespace Core {

namespace {

constexpr auto kPageIdProperty = "pageId";
constexpr auto kPageOrderProperty = "pageOrder";
constexpr auto kCommandShortcut = "Ctrl+K";
constexpr auto kMaskObjectName = "BlockMask";
auto maskStyleSheet() -> const QString &
{
    static const QString style = QStringLiteral("#%1 { background: rgba(0, 0, 0, 128); }")
                                     .arg(QLatin1String(kMaskObjectName));
    return style;
}
constexpr int kWindowWidth = 1060;
constexpr int kWindowHeight = 680;
constexpr int kSectionCount = 4;

auto sectionIndex(Shell::PageArea area) -> int
{
    switch (area) {
    case Shell::PageArea::Workspace:
        return 0;
    case Shell::PageArea::Games:
        return 1;
    case Shell::PageArea::Settings:
        return 2;
    case Shell::PageArea::Help:
        return 3;
    }
    return 0;
}

// 每个页面外层套一层壳：标题与说明由外壳统一排版，
// 插件只交出内容，所有页面因此天然共享同一种版式与边距。
class PageFrame final : public QWidget
{
public:
    explicit PageFrame(QWidget *content, QWidget *parent)
        : QWidget(parent)
    {
        setObjectName(QStringLiteral("PageFrame"));

        title = new QLabel(this);
        title->setObjectName(QStringLiteral("PageTitle"));

        subtitle = new QLabel(this);
        subtitle->setObjectName(QStringLiteral("PageSubtitle"));
        subtitle->setWordWrap(true);

        auto *layout = new QVBoxLayout(this);
        layout->setContentsMargins(28, 22, 28, 22);
        layout->setSpacing(4);
        layout->addWidget(title);
        layout->addWidget(subtitle);
        layout->addSpacing(16);
        layout->addWidget(content, 1);
    }

    void setTexts(const QString &titleText, const QString &subtitleText)
    {
        title->setText(titleText);
        subtitle->setText(subtitleText);
        subtitle->setVisible(!subtitleText.isEmpty());
    }

private:
    QLabel *title = nullptr;
    QLabel *subtitle = nullptr;
};

} // namespace

class MainWindow::MainWindowPrivate
{
public:
    struct Section
    {
        QLabel *label = nullptr;
        QWidget *container = nullptr;
        QVBoxLayout *layout = nullptr;
    };

    explicit MainWindowPrivate(MainWindow *q)
        : q_ptr(q)
    {}

    void buildChrome();
    void buildTray();
    void buildOverlay();
    void setBlocked(bool blocked);
    void updateSections();
    void retranslatePages();
    void addPage(Shell::IPage *page);
    auto buttonForPage(Shell::IPage *page) -> QPushButton *;
    auto sectionAt(Shell::PageArea area) -> Section &;

    MainWindow *q_ptr = nullptr;

    QWidget *central = nullptr;
    QWidget *maskWidget = nullptr;
    QGraphicsBlurEffect *blurEffect = nullptr;

    Section sections[kSectionCount];
    QStackedWidget *stack = nullptr;
    QButtonGroup *group = nullptr;
    QLabel *commandHint = nullptr;
    QLabel *commandShortcut = nullptr;
    QLabel *footer = nullptr;
    QLabel *status = nullptr;
    QTimer *statusTimer = nullptr;
    QPushButton *commandButton = nullptr;
    QAction *themeAction = nullptr;
    QAction *languageAction = nullptr;
    QPointer<CommandPalette> palette;
    QSystemTrayIcon *tray = nullptr;
    QMenu *trayMenu = nullptr;
    QAction *quitAction = nullptr;

    QHash<QString, int> pageIndex;
    bool defaultPageChosen = false;
    int defaultPageOrder = std::numeric_limits<int>::max();
    bool quitting = false;
};

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

    // 三个分组各自带标题，空分组连标题一起隐藏。
    for (Section &section : sections) {
        section.label = new QLabel(sidebar);
        section.label->setObjectName("SidebarSection");

        section.container = new QWidget(sidebar);
        section.layout = new QVBoxLayout(section.container);
        section.layout->setContentsMargins({});
        section.layout->setSpacing(2);

        sidebarLayout->addWidget(section.label);
        sidebarLayout->addWidget(section.container);
    }

    sidebarLayout->addStretch();

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
    q_ptr->setMinimumSize(kWindowWidth, kWindowHeight);
    q_ptr->resize(kWindowWidth, kWindowHeight);
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

// 遮罩与模糊都盖在内容区（含标题栏），标题栏按钮此时不可点，与模态语义一致。
void MainWindow::MainWindowPrivate::buildOverlay()
{
    maskWidget = new QWidget(q_ptr);
    maskWidget->setObjectName(kMaskObjectName);
    maskWidget->setStyleSheet(maskStyleSheet());
    maskWidget->hide();

    blurEffect = new QGraphicsBlurEffect(q_ptr);
    blurEffect->setEnabled(false);
    central->setGraphicsEffect(blurEffect);
}

void MainWindow::MainWindowPrivate::setBlocked(bool blocked)
{
    if (!blocked) {
        maskWidget->hide();
        blurEffect->setEnabled(false);
        return;
    }

    if (Utils::widgetManager()->showMask()) {
        const int padding = q_ptr->shadowPadding();
        maskWidget->resize(q_ptr->size() - QSize(padding, padding) * 2);
        maskWidget->move(padding, padding);
        maskWidget->show();
        maskWidget->raise();
    }

    if (Utils::widgetManager()->blurBackground())
        blurEffect->setEnabled(true);
}

// 插件按需注册，空分组连标题一起隐藏，避免侧栏出现孤立的小标题。
void MainWindow::MainWindowPrivate::updateSections()
{
    for (Section &section : sections) {
        const bool visible = section.layout->count() > 0;
        section.label->setVisible(visible);
        section.container->setVisible(visible);
    }
}

auto MainWindow::MainWindowPrivate::sectionAt(Shell::PageArea area) -> Section &
{
    return sections[sectionIndex(area)];
}

// 语言切换后，侧栏按钮与页面标题都来自 IPage::title()/subtitle()，需要重新取一次。
void MainWindow::MainWindowPrivate::retranslatePages()
{
    for (Shell::IPage *page : Shell::Registry::instance()->pages()) {
        const auto it = pageIndex.constFind(page->id());
        if (it == pageIndex.cend())
            continue;

        if (auto *button = group->button(it.value()))
            button->setText(page->title());
        // stack 里只放 PageFrame，index 来自 pageIndex，类型是确定的。
        static_cast<PageFrame *>(stack->widget(it.value()))
            ->setTexts(page->title(), page->subtitle());
    }
}

auto MainWindow::MainWindowPrivate::buttonForPage(Shell::IPage *page) -> QPushButton *
{
    auto *button = new QPushButton(page->title());
    button->setObjectName(QStringLiteral("SidebarButton"));
    button->setCheckable(true);
    button->setCursor(Qt::PointingHandCursor);
    button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    if (!page->icon().isNull())
        button->setIcon(page->icon());
    button->setProperty(kPageIdProperty, page->id());
    button->setProperty(kPageOrderProperty, page->order());
    return button;
}

void MainWindow::MainWindowPrivate::addPage(Shell::IPage *page)
{
    if (!page || pageIndex.contains(page->id()))
        return;

    QWidget *widget = page->widget();
    if (!widget)
        return;

    QPushButton *button = buttonForPage(page);
    QVBoxLayout *layout = sectionAt(page->area()).layout;

    // 同组内按 IPage::order() 插入，插件注册顺序不影响侧边栏顺序。
    const int order = page->order();
    int position = layout->count();
    for (int i = 0; i < layout->count(); ++i) {
        auto *existing = qobject_cast<QPushButton *>(layout->itemAt(i)->widget());
        if (!existing)
            continue;
        if (existing->property(kPageOrderProperty).toInt() > order) {
            position = i;
            break;
        }
    }
    layout->insertWidget(position, button);

    auto *frame = new PageFrame(widget, stack);
    frame->setTexts(page->title(), page->subtitle());

    const int index = stack->addWidget(frame);
    group->addButton(button, index);
    pageIndex.insert(page->id(), index);

    // 默认页取 order() 最小的 Workspace 页。
    if (page->area() == Shell::PageArea::Workspace
        && (!defaultPageChosen || order < defaultPageOrder)) {
        if (auto *selected = group->button(index))
            selected->setChecked(true);
        stack->setCurrentIndex(index);
        defaultPageChosen = true;
        defaultPageOrder = order;
    }

    updateSections();
}

MainWindow::MainWindow(QWidget *parent)
    : Widgets::AppWindow(parent)
    , d_ptr(std::make_unique<MainWindowPrivate>(this))
{
    d_ptr->buildChrome();
    d_ptr->buildTray();
    d_ptr->buildOverlay();
    retranslateUi();
    setupConnections();
    installEventFilter(this);

    const QList<Shell::IPage *> pages = Shell::Registry::instance()->pages();
    for (Shell::IPage *page : pages)
        d_ptr->addPage(page);
}

MainWindow::~MainWindow() = default;

void MainWindow::retranslateUi()
{
    d_ptr->commandHint->setText(tr("Search commands"));
    d_ptr->sections[0].label->setText(tr("Pages"));
    d_ptr->sections[1].label->setText(tr("Games"));
    d_ptr->sections[2].label->setText(tr("Settings"));
    d_ptr->sections[3].label->setText(tr("Help"));
    d_ptr->footer->setText(
        QStringLiteral("%1 %2").arg(QLatin1String(Utils::appName), QLatin1String(Utils::version)));

    updateThemeAction();
    d_ptr->languageAction->setText(
        Utils::languageManager()->currentLanguage() == Utils::LanguageManager::Chinese
            ? QStringLiteral("zh-CN")
            : QStringLiteral("EN"));
    d_ptr->languageAction->setToolTip(tr("Switch language"));

    d_ptr->retranslatePages();

    if (d_ptr->tray) {
        d_ptr->tray->setToolTip(Utils::appName);
        if (d_ptr->quitAction)
            d_ptr->quitAction->setText(tr("Quit"));
    }
}

void MainWindow::updateThemeAction()
{
    using Utils::ThemeMode;

    const ThemeMode mode = Utils::theme()->mode();
    switch (mode) {
    case ThemeMode::Light:
        d_ptr->themeAction->setText(tr("Light"));
        break;
    case ThemeMode::Dark:
        d_ptr->themeAction->setText(tr("Dark"));
        break;
    case ThemeMode::System:
        d_ptr->themeAction->setText(tr("Auto"));
        break;
    }
    d_ptr->themeAction->setToolTip(tr("Color scheme, click to switch"));
}

void MainWindow::onThemeActionTriggered()
{
    using Utils::ThemeMode;

    // 与“外观”页的下拉框同序：跟随系统 → 浅色 → 深色 → 跟随系统。
    ThemeMode next = ThemeMode::System;
    switch (Utils::theme()->mode()) {
    case ThemeMode::System:
        next = ThemeMode::Light;
        break;
    case ThemeMode::Light:
        next = ThemeMode::Dark;
        break;
    case ThemeMode::Dark:
        next = ThemeMode::System;
        break;
    }
    Utils::theme()->setMode(next);
}

void MainWindow::onThemeApplied(bool dark)
{
    Q_UNUSED(dark)
    updateThemeAction();
}

void MainWindow::onLanguageActionTriggered()
{
    const auto current = Utils::languageManager()->currentLanguage();
    Utils::languageManager()->loadLanguage(
        current == Utils::LanguageManager::Chinese ? Utils::LanguageManager::English
                                                   : Utils::LanguageManager::Chinese);
}

void MainWindow::setupConnections()
{
    connect(d_ptr->commandButton, &QPushButton::clicked, this, &MainWindow::openCommandPalette);
    connect(d_ptr->group, &QButtonGroup::idClicked, this, &MainWindow::onSidebarButtonClicked);
    connect(d_ptr->statusTimer, &QTimer::timeout, d_ptr->status, &QLabel::hide);
    connect(d_ptr->themeAction, &QAction::triggered, this, &MainWindow::onThemeActionTriggered);
    connect(d_ptr->languageAction, &QAction::triggered, this, &MainWindow::onLanguageActionTriggered);
    connect(Utils::theme(), &Utils::Theme::themeApplied, this, &MainWindow::onThemeApplied);

    if (d_ptr->tray) {
        // 托盘菜单的"退出"要在事件循环里执行，避免在菜单回调里销毁自己。
        connect(
            d_ptr->quitAction,
            &QAction::triggered,
            qApp,
            [] { Utils::quitApplication(); },
            Qt::QueuedConnection);
        connect(d_ptr->tray, &QSystemTrayIcon::activated, this, &MainWindow::onTrayActivated);
    }

    connect(
        Shell::Registry::instance(),
        &Shell::Registry::pagesChanged,
        this,
        &MainWindow::onPagesChanged);

    auto *shortcut = new QShortcut(QKeySequence(QLatin1String(kCommandShortcut)), this);
    connect(shortcut, &QShortcut::activated, this, &MainWindow::openCommandPalette);
}

void MainWindow::onSidebarButtonClicked(int index)
{
    d_ptr->stack->setCurrentIndex(index);
}

void MainWindow::onTrayActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::Trigger || reason == QSystemTrayIcon::DoubleClick)
        Utils::restoreAndActivate(this);
}

void MainWindow::onPagesChanged()
{
    const QList<Shell::IPage *> pages = Shell::Registry::instance()->pages();
    for (Shell::IPage *page : pages)
        d_ptr->addPage(page);
}

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
    // 重启计时器：连续两条消息时，前一条的定时器不该提前把后一条关掉。
    d_ptr->statusTimer->start(timeoutMs);
}

auto MainWindow::askForRestart(QAnyStringView reason) -> bool
{
    const auto answer = QMessageBox::question(
        this,
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

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    // 顶部窗口被模态对话框阻塞时，Qt 会把 WindowBlocked/WindowUnblocked 发给它。
    if (watched == this) {
        switch (event->type()) {
        case QEvent::WindowBlocked:
            d_ptr->setBlocked(true);
            break;
        case QEvent::WindowUnblocked:
            d_ptr->setBlocked(false);
            break;
        default:
            break;
        }
    }
    return Widgets::AppWindow::eventFilter(watched, event);
}

void MainWindow::changeEvent(QEvent *event)
{
    Widgets::AppWindow::changeEvent(event);
    if (event->type() == QEvent::LanguageChange)
        retranslateUi();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    // 关闭窗口即退出：否则进程会带着托盘图标留在后台，用户找不到退出入口。
    if (!d_ptr->quitting) {
        d_ptr->quitting = true;
        Utils::quitApplication();
    }
    Widgets::AppWindow::closeEvent(event);
}

} // namespace Core
