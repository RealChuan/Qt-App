#ifndef WAITWIDGET_H
#define WAITWIDGET_H

#include "gui_global.hpp"

#include <QSplashScreen>

namespace GUI {

class GUI_EXPORT WaitWidget : public QSplashScreen
{
    Q_OBJECT
public:
    explicit WaitWidget();
    ~WaitWidget() override;

    void fullProgressBar();

private slots:
    void updateProgressBar();

private:
    class WaitWidgetPrivate;
    QScopedPointer<WaitWidgetPrivate> d_ptr;
};

} // namespace GUI

#endif // WAITWIDGET_H
