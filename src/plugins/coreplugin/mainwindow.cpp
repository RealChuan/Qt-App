#include "mainwindow.h"
#include "configwidget.h"
#include "plugindialog.h"

#include <core/corewidget.hpp>
#include <extensionsystem/pluginmanager.h>
#include <gui/messagebox.h>
#include <utils/utils.h>

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

        initMaskWidget();
    }

    void setupUI()
    {
        createSystemTray();
        setInitWidget(MainWindow::tr("Hello World!"));

        auto *widget = new QWidget(q_ptr);
        auto *layout = new QHBoxLayout(widget);
        layout->setContentsMargins({});
        layout->setSpacing(0);
        layout->addWidget(createSidebar());
        layout->addWidget(stackedWidget);

        q_ptr->setCentralWidget(widget);
        q_ptr->setMinimumSize(1000, 618);
        Utils::windowCenter(q_ptr);
    }

    MainWindow *q_ptr;

    QButtonGroup *switchBtnGroup;
    QButtonGroup *menuBtnGroup;
    QStackedWidget *stackedWidget;
    QVBoxLayout *vLayoutGroup1;
    QVBoxLayout *vLayoutGroup2;
    QVBoxLayout *vLayoutGroup3;

    QWidget *maskWidget;

private:
    void createSystemTray()
    {
        if (!QSystemTrayIcon::isSystemTrayAvailable()) {
            GUI::MessageBox::Info(q_ptr,
                                  tr("Systray, I couldn't detect any system "
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
                       [this](QSystemTrayIcon::ActivationReason reason) {
                           switch (reason) {
                           case QSystemTrayIcon::DoubleClick: q_ptr->show(); break;
                           default: break;
                           }
                       });

        qApp->setQuitOnLastWindowClosed(false);

        q_ptr->connect(qApp,
                       &QApplication::applicationStateChanged,
                       q_ptr,
                       [this](Qt::ApplicationState state) {
                           if (state == Qt::ApplicationActive) {
                               q_ptr->show();
                           }
                       });

        QMetaObject::invokeMethod(
            q_ptr,
            [systemTrayIcon] {
                systemTrayIcon->showMessage(MainWindow::tr("Hello World!"),
                                            MainWindow::tr("This is an Qt-App."),
                                            QIcon(":/icon/icon/app.png"));
            },
            Qt::QueuedConnection);
    }

    void setInitWidget(const QString &text) const
    {
        auto *label = new QLabel(text, q_ptr);
        label->setAlignment(Qt::AlignCenter);
        label->setObjectName("HomeLabel");
        stackedWidget->addWidget(label);
        stackedWidget->setCurrentWidget(label);
    }

    auto createSidebar() -> QWidget *
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

    void initMaskWidget()
    {
        maskWidget = new QWidget(q_ptr);
        maskWidget->setStyleSheet("background:rgba(0, 0, 0, 128);");
        maskWidget->hide();
    }
};

MainWindow::MainWindow(QWidget *parent)
    : GUI::CommonWidget(parent)
    , d_ptr(new MainWindowPrivate(this))
{
    d_ptr->setupUI();
    buildConnect();
    installEventFilter(this);
}

MainWindow::~MainWindow() = default;

void MainWindow::extensionsInitialized()
{
    for (auto *const page : ExtensionSystem::PluginManager::getObjects<Core::CoreWidget>()) {
        if (page->widget() == nullptr) {
            continue;
        }
        if (page->button()->property("Type") == Core::CoreWidget::Type::Main) {
            d_ptr->vLayoutGroup1->addWidget(page->button());
        } else if (page->button()->property("Type") == Core::CoreWidget::Type::Help) {
            d_ptr->vLayoutGroup2->addWidget(page->button());
        } else {
            continue;
        }
        d_ptr->menuBtnGroup->addButton(page->button());
        d_ptr->stackedWidget->addWidget(page->widget());
        connect(page->button(), &QPushButton::clicked, this, [this, page] {
            d_ptr->stackedWidget->setCurrentWidget(page->widget());
        });
    }

    initMenu();
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

void MainWindow::onAboutPlugins()
{
    PluginDialog dialog(this);
    dialog.exec();
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == this) {
        switch (event->type()) {
        case QEvent::WindowBlocked: {
            auto size = this->size();
            auto padding = shadowPadding();
            size -= QSize(padding, padding) * 2;
            d_ptr->maskWidget->resize(size);
            d_ptr->maskWidget->move(padding, padding);
            d_ptr->maskWidget->show();
            d_ptr->maskWidget->raise();
        } break;
        case QEvent::WindowUnblocked: d_ptr->maskWidget->hide(); break;
        default: break;
        }
    }
    return GUI::CommonWidget::eventFilter(watched, event);
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

void MainWindow::initMenu()
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
