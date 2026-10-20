#pragma once

#include "widgets_global.hpp"

#include <QSplashScreen>

#include <memory>

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
    std::unique_ptr<WaitWidgetPrivate> d_ptr;
};

} // namespace Widgets
