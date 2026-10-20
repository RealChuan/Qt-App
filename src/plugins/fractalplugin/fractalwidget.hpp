#pragma once

#include <shell/page.hpp>

#include <QImage>
#include <QWidget>

#include <atomic>
#include <memory>

namespace Fractal {

// 一次异步渲染的参数。用共享指针传给工作线程，主线程改视图时把旧的标记为取消。
struct RenderJob
{
    std::atomic_bool cancelled{false};
    int width = 0;
    int height = 0;
    double centerX = 0.0;
    double centerY = 0.0;
    double scale = 3.0;
};

// 后台计算示例：视图每次变化都启动一个新的 QtConcurrent 任务，
// 旧任务通过共享的取消标记自行退出。
class FractalWidget final : public QWidget, public Shell::IPage
{
    Q_OBJECT

public:
    explicit FractalWidget(QWidget *parent = nullptr);
    ~FractalWidget() override;

    auto id() const -> QString override { return QStringLiteral("fractalplugin.page"); }
    auto title() const -> QString override { return tr("Fractal"); }
    auto order() const -> int override { return 30; }
    auto widget() -> QWidget * override { return this; }

    void resetView();

protected:
    void paintEvent(QPaintEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    class FractalWidgetPrivate;
    std::unique_ptr<FractalWidgetPrivate> d_ptr;
};

// 工作线程入口（暴露在头文件里只是为了 QtConcurrent 能直接取地址）。
auto renderFractal(const std::shared_ptr<RenderJob> &job) -> QImage;

} // namespace Fractal
