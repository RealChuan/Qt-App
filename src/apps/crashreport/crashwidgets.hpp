#ifndef CRASHWIDGETS_HPP
#define CRASHWIDGETS_HPP

#include <gui/mainwidget.hpp>

namespace Crash {

class CrashWidgets : public GUI::MainWidget
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

#endif // CRASHWIDGETS_HPP
