#pragma once

#include <widgets/mainwidget.hpp>

#include <QSystemTrayIcon>

namespace Plugin {

class MainWindow : public Widgets::MainWidget
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    void extensionsInitialized();

private slots:
    void onSystrayIconActivated(QSystemTrayIcon::ActivationReason reason);
    void onApplicationStateChanged(Qt::ApplicationState state);
    void onShowGroupButton(int id);
    void onAboutPlugins();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void buildConnect();
    void setupMenu();

    class MainWindowPrivate;
    QScopedPointer<MainWindowPrivate> d_ptr;
};

} // namespace Plugin
