#pragma once

#include "mainwindow.hpp"

#include <QHash>
#include <QPointer>
#include <QString>

#include <limits>
#include <memory>

class QAction;
class QButtonGroup;
class QLabel;
class QMenu;
class QPushButton;
class QStackedWidget;
class QTimer;
class QVBoxLayout;
class QWidget;

namespace Widgets {
class ModalOverlay;
} // namespace Widgets

namespace Core {

class CommandPalette;

inline constexpr char kCommandShortcut[] = "Ctrl+K";

class MainWindow::MainWindowPrivate
{
public:
    struct Section
    {
        QLabel *label = nullptr;
        QWidget *container = nullptr;
        QVBoxLayout *layout = nullptr;
    };

    static constexpr int kSectionCount = 4;

    // out-of-line: unique_ptr<ModalOverlay> 需要完整类型
    explicit MainWindowPrivate(MainWindow *q);
    ~MainWindowPrivate();

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
    // 遮挡层是通用组件：这里的窗口只是它的一个使用者。
    std::unique_ptr<Widgets::ModalOverlay> overlay;

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

} // namespace Core
