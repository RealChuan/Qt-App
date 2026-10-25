#include "mainwindow.hpp"

#include "commandpalette.hpp"
#include "mainwindow_p.hpp"

#include <shell/registry.hpp>
#include <utils/appinfo.hpp>
#include <utils/singletonmanager.hpp>
#include <utils/theme.hpp>
#include <utils/uihelpers.hpp>
#include <utils/utils.hpp>
#include <widgets/messagebox.hpp>

#include <QAction>
#include <QApplication>
#include <QButtonGroup>
#include <QCloseEvent>
#include <QEvent>
#include <QKeySequence>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QShortcut>
#include <QStackedWidget>
#include <QSystemTrayIcon>
#include <QTimer>

namespace Core {

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
        QStringLiteral("%1 %2")
            .arg(QLatin1String(Utils::AppInfo::appName), QLatin1String(Utils::AppInfo::version)));

    updateThemeAction();
    d_ptr->languageAction->setText(
        Utils::languageManager()->currentLanguage() == Utils::LanguageManager::Chinese
            ? QStringLiteral("zh-CN")
            : QStringLiteral("EN"));
    d_ptr->languageAction->setToolTip(tr("Switch language"));

    d_ptr->retranslatePages();

    if (d_ptr->tray) {
        d_ptr->tray->setToolTip(Utils::AppInfo::appName);
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
    const auto answer = Widgets::MessageBox::question(
        this,
        tr("Restart required"),
        reason.toString(),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::Yes);
    if (answer != QMessageBox::Yes)
        return false;

    Utils::requestRestart();
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
