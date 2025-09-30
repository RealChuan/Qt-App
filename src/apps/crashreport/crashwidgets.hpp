#pragma once

#include <widgets/mainwidget.hpp>

namespace Crash {

class CrashWidgets : public Widgets::MainWidget
{
    Q_OBJECT
public:
    explicit CrashWidgets(QWidget *parent = nullptr);
    ~CrashWidgets() override;

private slots:
    void onOpenCrashPath();
    void onRestart();

private:
    void setupUI();
    void init();

    class CrashWidgetsPrivate;
    QScopedPointer<CrashWidgetsPrivate> d_ptr;
};

} // namespace Crash
