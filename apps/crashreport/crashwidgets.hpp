#ifndef CRASHWIDGETS_HPP
#define CRASHWIDGETS_HPP

#include <gui/commonwidget.hpp>

namespace Crash {

class CrashWidgets : public GUI::CommonWidget
{
    Q_OBJECT
public:
    explicit CrashWidgets(QWidget *parent = nullptr);
    ~CrashWidgets() override;

private slots:
    void onOpenCrashPath();
    void onRestart();
    void onQuit();

private:
    void setupUI();
    void init();

    class CrashWidgetsPrivate;
    QScopedPointer<CrashWidgetsPrivate> d_ptr;
};

} // namespace Crash

#endif // CRASHWIDGETS_HPP
