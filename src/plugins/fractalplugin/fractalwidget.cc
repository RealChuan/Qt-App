#include "fractalwidget.hpp"

#include <QFutureWatcher>
#include <QMouseEvent>
#include <QPainter>
#include <QResizeEvent>
#include <QWheelEvent>
#include <QtConcurrent>

#include <algorithm>

namespace Fractal {

namespace {
constexpr int kMaxIterations = 120;
constexpr int kCancelCheckRows = 16;

auto colorFor(int iterations) -> QRgb
{
    if (iterations >= kMaxIterations)
        return qRgb(12, 14, 20);

    const double t = double(iterations) / kMaxIterations;
    const int r = int(9 * (1 - t) * t * t * t * 255);
    const int g = int(15 * (1 - t) * (1 - t) * t * t * 255);
    const int b = int(8.5 * (1 - t) * (1 - t) * (1 - t) * t * 255);
    return qRgb(r, g, b);
}
} // namespace

auto renderFractal(const std::shared_ptr<RenderJob> &job) -> QImage
{
    QImage image(job->width, job->height, QImage::Format_RGB32);
    image.fill(Qt::black);
    if (image.isNull())
        return {};

    const double aspect = double(job->width) / std::max(1, job->height);
    const double halfWidth = job->scale * aspect / 2.0;
    const double minX = job->centerX - halfWidth;
    const double minY = job->centerY - job->scale / 2.0;

    for (int y = 0; y < job->height; ++y) {
        if (y % kCancelCheckRows == 0 && job->cancelled.load(std::memory_order_relaxed))
            return {};

        auto *line = reinterpret_cast<QRgb *>(image.scanLine(y));
        const double ci = minY + job->scale * y / job->height;
        for (int x = 0; x < job->width; ++x) {
            const double cr = minX + 2.0 * halfWidth * x / job->width;
            double zr = 0.0;
            double zi = 0.0;
            int iterations = 0;
            while (zr * zr + zi * zi <= 4.0 && iterations < kMaxIterations) {
                const double tmp = zr * zr - zi * zi + cr;
                zi = 2.0 * zr * zi + ci;
                zr = tmp;
                ++iterations;
            }
            line[x] = colorFor(iterations);
        }
    }
    return image;
}

class FractalWidget::FractalWidgetPrivate
{
public:
    explicit FractalWidgetPrivate(FractalWidget *q)
        : q_ptr(q)
    {}

    void scheduleRender();

    FractalWidget *q_ptr = nullptr;
    QImage image;
    QFutureWatcher<QImage> watcher;
    std::shared_ptr<RenderJob> job;
    double centerX = -0.6;
    double centerY = 0.0;
    double scale = 3.0;
    QPoint lastPos;
};

void FractalWidget::FractalWidgetPrivate::scheduleRender()
{
    if (job)
        job->cancelled.store(true, std::memory_order_relaxed);

    job = std::make_shared<RenderJob>();
    job->width = std::max(1, q_ptr->width());
    job->height = std::max(1, q_ptr->height());
    job->centerX = centerX;
    job->centerY = centerY;
    job->scale = scale;

    watcher.setFuture(QtConcurrent::run(renderFractal, job));
}

FractalWidget::FractalWidget(QWidget *parent)
    : QWidget(parent)
    , d_ptr(std::make_unique<FractalWidgetPrivate>(this))
{
    setMinimumSize(320, 240);

    connect(&d_ptr->watcher, &QFutureWatcher<QImage>::finished, this, [this] {
        const QImage result = d_ptr->watcher.result();
        if (!result.isNull()) {
            d_ptr->image = result;
            update();
        }
    });
}

FractalWidget::~FractalWidget()
{
    // 退出时如果还有渲染任务在跑，先取消再等它结束：
    // 否则窗口销毁后任务仍可能运行，进程退出时线程池收尾会很难看。
    if (d_ptr->job)
        d_ptr->job->cancelled.store(true, std::memory_order_relaxed);
    d_ptr->watcher.waitForFinished();
}

void FractalWidget::resetView()
{
    d_ptr->centerX = -0.6;
    d_ptr->centerY = 0.0;
    d_ptr->scale = 3.0;
    d_ptr->scheduleRender();
}

void FractalWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    if (!d_ptr->image.isNull())
        painter.drawImage(rect(), d_ptr->image);
    else
        painter.fillRect(rect(), QColor(12, 14, 20));

    painter.setPen(QColor(230, 230, 230, 160));
    painter.drawText(rect().adjusted(12, 12, -12, -12),
                     Qt::AlignLeft | Qt::AlignBottom,
                     tr("Wheel: zoom    Drag: pan    Zoom: %1").arg(d_ptr->scale, 0, 'f', 3));
}

void FractalWidget::wheelEvent(QWheelEvent *event)
{
    d_ptr->scale = std::clamp(d_ptr->scale * (event->angleDelta().y() > 0 ? 0.8 : 1.25),
                              1e-6,
                              8.0);
    d_ptr->scheduleRender();
}

void FractalWidget::mousePressEvent(QMouseEvent *event)
{
    d_ptr->lastPos = event->pos();
}

void FractalWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (!(event->buttons() & Qt::LeftButton))
        return;

    const QPoint delta = event->pos() - d_ptr->lastPos;
    d_ptr->lastPos = event->pos();

    const double aspect = double(width()) / std::max(1, height());
    d_ptr->centerX -= delta.x() * d_ptr->scale * aspect / std::max(1, width());
    d_ptr->centerY -= delta.y() * d_ptr->scale / std::max(1, height());
    d_ptr->scheduleRender();
}

void FractalWidget::mouseReleaseEvent(QMouseEvent *)
{
    d_ptr->scheduleRender();
}

void FractalWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    d_ptr->scheduleRender();
}

} // namespace Fractal
