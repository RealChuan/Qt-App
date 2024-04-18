#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <gui/commonwidget.hpp>

namespace Plugin {

class MainWindow : public GUI::CommonWidget
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    void extensionsInitialized();

private slots:
    void onShowGroupButton(int id);
    void onAboutPlugins();

private:
    void setupUI();
    void buildConnect();
    void initMenu();

    class MainWindowPrivate;
    QScopedPointer<MainWindowPrivate> d_ptr;
};

} // namespace Plugin

#endif // MAINWINDOW_H
