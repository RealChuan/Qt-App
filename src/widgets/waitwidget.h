#ifndef WAITWIDGET_H
#define WAITWIDGET_H

#include "widgets_global.hpp"

#include <QSplashScreen>

namespace Widgets {

class WIDGETS_EXPORT WaitWidget : public QSplashScreen
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

} // namespace Widgets

#endif // WAITWIDGET_H
