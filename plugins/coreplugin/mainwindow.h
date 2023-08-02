#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <gui/commonwidget.hpp>

class QStackedWidget;
class QAbstractButton;

namespace Plugin {

class MainWindow : public GUI::CommonWidget
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void extensionsInitialized();

private slots:
    void onShowGroupButton(int id);
    void onAboutPlugins();
    void onQuit();

private:
    void setupUI();
    void buildConnect();
    void initMenu();

    class MainWindowPrivate;
    QScopedPointer<MainWindowPrivate> d_ptr;
};

} // namespace Plugin

#endif // MAINWINDOW_H
