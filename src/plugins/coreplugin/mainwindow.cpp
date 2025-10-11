#include "mainwindow.h"
#include "configwidget.h"
#include "plugindialog.h"

#include <core/corewidget.hpp>
#include <extensionsystem/pluginmanager.h>
#include <utils/singletonmanager.hpp>
#include <utils/utils.hpp>
#include <widgets/messagebox.h>

#include <QtWidgets>

namespace Plugin {

class MainWindow::MainWindowPrivate
{
public:
    explicit MainWindowPrivate(MainWindow *q)
        : q_ptr(q)
    {
        switchBtnGroup = new QButtonGroup(q_ptr);
        switchBtnGroup->setExclusive(true);
        menuBtnGroup = new QButtonGroup(q_ptr);
        menuBtnGroup->setExclusive(true);
        stackedWidget = new QStackedWidget(q_ptr);

        vLayoutGroup1 = new QVBoxLayout;
        vLayoutGroup2 = new QVBoxLayout;
        vLayoutGroup3 = new QVBoxLayout;
        vLayoutGroup1->setContentsMargins({});
        vLayoutGroup1->setSpacing(0);
        vLayoutGroup2->setContentsMargins({});
        vLayoutGroup2->setSpacing(0);
        vLayoutGroup3->setContentsMargins({});
        vLayoutGroup3->setSpacing(0);
    }

    void setupUI()
    {
        setupSystemTray();
        setupInitWidget(MainWindow::tr("Hello World!"));

        auto *widget = new QWidget(q_ptr);
        auto *layout = new QHBoxLayout(widget);
        layout->setContentsMargins({});
        layout->setSpacing(0);
        layout->addWidget(setupSidebar());
        layout->addWidget(stackedWidget);

        q_ptr->setCentralWidget(widget);
        q_ptr->setMinimumSize(1000, 618);
        Utils::windowCenter(q_ptr);

        setupMaskWidget();
        setupBlurEffect();

        widget->setGraphicsEffect(blurEffect);
    }

    MainWindow *q_ptr;

    QButtonGroup *switchBtnGroup;
    QButtonGroup *menuBtnGroup;
    QStackedWidget *stackedWidget;
    QVBoxLayout *vLayoutGroup1;
    QVBoxLayout *vLayoutGroup2;
    QVBoxLayout *vLayoutGroup3;

    QWidget *maskWidget;
    QGraphicsBlurEffect *blurEffect;

private:
    void setupSystemTray()
    {
        if (!QSystemTrayIcon::isSystemTrayAvailable()) {
            Widgets::MessageBox::Info(q_ptr,
                                      MainWindow::tr("Systray, I couldn't detect any system "
                                                     "tray on this system."));
            return;
        }

        auto *menu = new QMenu(q_ptr);
        menu->addAction(
            MainWindow::tr("Quit"), q_ptr, [] { Utils::quitApplication(); }, Qt::QueuedConnection);

        auto *systemTrayIcon = new QSystemTrayIcon(q_ptr);
        systemTrayIcon->setToolTip(MainWindow::tr("This is an Qt-App."));
        systemTrayIcon->setIcon(QIcon(":/icon/icon/app.png"));
        systemTrayIcon->setContextMenu(menu);
        systemTrayIcon->show();
        q_ptr->connect(systemTrayIcon,
                       &QSystemTrayIcon::activated,
                       q_ptr,
                       &MainWindow::onSystrayIconActivated);

        qApp->setQuitOnLastWindowClosed(false);
        // for macOS, when the app is active, click the dock icon to show the main window
        q_ptr->connect(qApp,
                       &QApplication::applicationStateChanged,
                       q_ptr,
                       &MainWindow::onApplicationStateChanged);

        QMetaObject::invokeMethod(
            q_ptr,
            [systemTrayIcon] {
                systemTrayIcon->showMessage(MainWindow::tr("Hello World!"),
                                            MainWindow::tr("This is an Qt-App."),
                                            systemTrayIcon->icon());
            },
            Qt::QueuedConnection);
    }

    void setupInitWidget(const QString &text) const
    {
        auto *label = new QLabel(text, q_ptr);
        label->setAlignment(Qt::AlignCenter);
        label->setObjectName("HomeLabel");
        stackedWidget->addWidget(label);
        stackedWidget->setCurrentWidget(label);
    }

    auto setupSidebar() -> QWidget *
    {
        auto *toolsButton = new QPushButton(MainWindow::tr("Main"), q_ptr);
        auto *helpButton = new QPushButton(MainWindow::tr("Help"), q_ptr);
        auto *settingsButton = new QPushButton(MainWindow::tr("Settings"), q_ptr);
        auto *pluginButton = new QPushButton(MainWindow::tr("Plugins"), q_ptr);

        auto *configWidget = new ConfigWidget(q_ptr);
        stackedWidget->addWidget(configWidget);
        q_ptr->connect(settingsButton, &QPushButton::clicked, q_ptr, [this, configWidget] {
            stackedWidget->setCurrentWidget(configWidget);
        });
        q_ptr->connect(pluginButton, &QPushButton::clicked, q_ptr, &MainWindow::onAboutPlugins);

        toolsButton->setProperty("Type", Core::CoreWidget::Type::Main);
        helpButton->setProperty("Type", Core::CoreWidget::Type::Help);
        settingsButton->setProperty("Type", Core::CoreWidget::Type::Help);
        pluginButton->setProperty("Type", Core::CoreWidget::Type::Help);

        switchBtnGroup->addButton(toolsButton, 0);
        switchBtnGroup->addButton(helpButton, 1);

        menuBtnGroup->addButton(settingsButton);
        menuBtnGroup->addButton(pluginButton);

        vLayoutGroup1->addWidget(toolsButton);

        vLayoutGroup2->addWidget(helpButton);
        vLayoutGroup2->addWidget(settingsButton);
        vLayoutGroup2->addWidget(pluginButton);

        auto *widget = new QWidget(q_ptr);
        widget->setObjectName("MenuWidget");
        auto *layout = new QVBoxLayout(widget);
        layout->setContentsMargins({});
        layout->setSpacing(0);
        layout->addLayout(vLayoutGroup1);
        layout->addLayout(vLayoutGroup2);
        layout->addLayout(vLayoutGroup3);
        layout->addStretch();

        return widget;
    }

    void setupMaskWidget()
    {
        maskWidget = new QWidget(q_ptr);
        maskWidget->setStyleSheet("background:rgba(0, 0, 0, 128);");
        maskWidget->hide();
    }

    void setupBlurEffect()
    {
        blurEffect = new QGraphicsBlurEffect(q_ptr);
        blurEffect->setEnabled(false);
    }
};

MainWindow::MainWindow(QWidget *parent)
    : Widgets::MainWidget(parent)
    , d_ptr(new MainWindowPrivate(this))
{
    d_ptr->setupUI();
    buildConnect();
    installEventFilter(this);
}

MainWindow::~MainWindow() = default;

void MainWindow::extensionsInitialized()
{
    auto coreWidgets = Core::getCoreWidgets();

    for (auto *const page : std::as_const(coreWidgets)) {
        auto *widget = page->widget();
        if (widget == nullptr) {
            continue;
        }
        switch (page->type()) {
        case Core::CoreWidget::Type::Main: d_ptr->vLayoutGroup1->addWidget(page->button()); break;
        case Core::CoreWidget::Type::Help: d_ptr->vLayoutGroup2->addWidget(page->button()); break;
        default: continue;
        }
        auto *button = page->button();
        d_ptr->menuBtnGroup->addButton(button);
        d_ptr->stackedWidget->addWidget(widget);
        connect(button, &QPushButton::clicked, this, [this, page] {
            d_ptr->stackedWidget->setCurrentWidget(page->widget());
        });
    }

    setupMenu();
}

void MainWindow::onShowGroupButton(int id)
{
    auto *button = d_ptr->switchBtnGroup->button(id);
    auto type = button->property("Type");
    auto buttons = d_ptr->menuBtnGroup->buttons();
    for (auto *btn : buttons) {
        if (btn->property("Type") == type) {
            btn->show();
        } else {
            btn->hide();
        }
    }
}

void MainWindow::onSystrayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason) {
    case QSystemTrayIcon::DoubleClick:
    case QSystemTrayIcon::Trigger: Utils::restoreAndActivate(this); break;
    default: break;
    }
}

void MainWindow::onApplicationStateChanged(Qt::ApplicationState state)
{
    if (state == Qt::ApplicationActive) {
        Utils::restoreAndActivate(this);
    }
}

void MainWindow::onAboutPlugins()
{
    showAboutPlugins();
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == this) {
        switch (event->type()) {
        case QEvent::WindowBlocked:
            if (WIDGET_MANAGER->showMask()) {
                auto size = this->size();
                auto padding = shadowPadding();
                size -= QSize(padding, padding) * 2;
                d_ptr->maskWidget->resize(size);
                d_ptr->maskWidget->move(padding, padding);
                d_ptr->maskWidget->show();
                d_ptr->maskWidget->raise();
            }
            if (WIDGET_MANAGER->blurBackground()) {
                d_ptr->blurEffect->setEnabled(true);
            }
            break;
        case QEvent::WindowUnblocked:
            d_ptr->maskWidget->hide();
            d_ptr->blurEffect->setEnabled(false);
            break;
        default: break;
        }
    }
    return Widgets::MainWidget::eventFilter(watched, event);
}

void MainWindow::buildConnect()
{
    connect(
        this,
        &MainWindow::aboutToclose,
        this,
        [] { Utils::quitApplication(); },
        Qt::QueuedConnection);
}

void MainWindow::setupMenu()
{
    auto buttons = d_ptr->switchBtnGroup->buttons();
    for (auto *btn : buttons) {
        btn->setLayoutDirection(Qt::LeftToRight);
        btn->setProperty("class", "GroupButton");
    }
    buttons = d_ptr->menuBtnGroup->buttons();
    for (auto *btn : buttons) {
        btn->setProperty("class", "GroupItemButton");
        btn->setCheckable(true);
    }
    onShowGroupButton(0);
    connect(d_ptr->switchBtnGroup, &QButtonGroup::idClicked, this, &MainWindow::onShowGroupButton);
}

} // namespace Plugin
