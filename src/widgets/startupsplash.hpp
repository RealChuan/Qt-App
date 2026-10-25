#pragma once

#include "widgets_global.hpp"

#include <QSplashScreen>

#include <memory>

namespace Widgets {

// 启动期的遮挡画面。主线程加载插件时事件循环不转，定时器一个 tick 都收不到，
// 所以这里只显示一句状态文案，不假装知道百分比。
// 进度条只在调用方主动 setProgress() 之后才出现：留给"主线程等子线程、事件循环仍在转"
// 这类场景，目前的启动流程用不到。
class WIDGETS_EXPORT StartupSplash : public QSplashScreen
{
    Q_OBJECT
public:
    StartupSplash();
    ~StartupSplash() override;

    // 第一次调用时把进度条显示出来，之后按百分比推进（0-100，越界会被夹住）。
    void setProgress(int percent);

protected:
    void changeEvent(QEvent *event) override;

private:
    void retranslateUi();

    class StartupSplashPrivate;
    std::unique_ptr<StartupSplashPrivate> d_ptr;
};

} // namespace Widgets
