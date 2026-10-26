#pragma once

#include <shell/page.hpp>

#include <QColor>
#include <QEvent>
#include <QImage>
#include <QWidget>

#include <atomic>
#include <memory>

using namespace Qt::StringLiterals;

namespace Fractal {

// 一次异步渲染的参数。用共享指针传给工作线程，主线程改视图时把旧的标记为取消。
struct RenderJob
{
    std::atomic_bool cancelled{false};
    int width = 0;
    int height = 0;
    // 高分屏按物理像素算，画面上屏后才是 1:1，不会糊成一片。
    // 取真实 DPR 而不是取整：150% 屏 round 成 2 会白算 78% 的像素。
    qreal pixelRatio = 1.0;
    double centerX = 0.0;
    double centerY = 0.0;
    double scale = 3.0;
    // 配色由主线程从主题调色板里取出，工作线程只管填像素。
    bool isDark = true;
    QColor accentColor;
    QColor baseColor;
    QColor interiorColor;
};

// 后台计算示例：视图变化时启动一个 QtConcurrent 任务，同一时刻只跑一个，
// 新视图到来就把旧任务标记取消，等它退出后再补一帧。这里的计算量值得开线程，
// 与 pomodoro 那种"纯定时器不需要线程"的场景正好形成对照。
class FractalPage final : public QWidget, public Shell::IPage
{
    Q_OBJECT

public:
    explicit FractalPage(QWidget *parent = nullptr);
    ~FractalPage() override;

    [[nodiscard]] auto id() const -> QString override { return u"fractalplugin.page"_s; }
    [[nodiscard]] auto title() const -> QString override { return tr("Fractal"); }
    [[nodiscard]] auto subtitle() const -> QString override
    {
        return tr("Drag to pan, scroll to zoom. Each frame is rendered on a worker thread.");
    }
    [[nodiscard]] auto order() const -> int override { return Shell::PageOrder::kThird; }
    [[nodiscard]] auto widget() -> QWidget * override { return this; }

    void resetView();

private slots:
    void onRenderFinished();
    void onRenderTimeout();
    void onResetClicked();
    void onThemeApplied(bool dark);

protected:
    void paintEvent(QPaintEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void changeEvent(QEvent *event) override;

private:
    void retranslateUi();
    void setupConnections();

    class FractalPagePrivate;
    std::unique_ptr<FractalPagePrivate> d_ptr;
};

// 工作线程入口（暴露在头文件里只是为了 QtConcurrent 能直接取地址）。
auto renderFractal(const std::shared_ptr<RenderJob> &job) -> QImage;

} // namespace Fractal
