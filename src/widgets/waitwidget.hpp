#pragma once

#include "widgets_global.hpp"

#include <QSplashScreen>

#include <memory>

namespace Widgets {

// 启动期的等待条：只做一件事——在插件加载完之前自己爬到 95%，
// 由调用方在收尾时用 fullProgressBar() 补满。
class WIDGETS_EXPORT WaitWidget : public QSplashScreen
{
    Q_OBJECT
public:
    WaitWidget();
    ~WaitWidget() override;

    // 补满并停表；可重复调用。
    void fullProgressBar();

private slots:
    void onProgressTick();

private:
    class WaitWidgetPrivate;
    std::unique_ptr<WaitWidgetPrivate> d_ptr;
};

} // namespace Widgets
