#include "fractalpage.hpp"

#include <utils/singletonmanager.hpp>
#include <utils/theme.hpp>

#include <QFutureWatcher>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPalette>
#include <QPushButton>
#include <QResizeEvent>
#include <QThread>
#include <QTimer>
#include <QVBoxLayout>
#include <QVector>
#include <QWheelEvent>
#include <QtConcurrent>

#include <algorithm>
#include <array>
#include <cmath>
#include <thread>
#include <vector>

namespace Fractal {

namespace {

constexpr int kMaxIterations = 120;
constexpr int kCancelCheckRows = 16;
constexpr double kDefaultScale = 3.0;
constexpr double kDefaultCenterX = -0.6;
constexpr double kMinScale = 1e-6;
constexpr double kMaxScale = 8.0;

// 卡片外观对齐 pages.css 的 #Card：12px 圆角 + 1px 描边。
// 插件自绘的页面只有照抄这套尺寸，才不会看起来像另一个程序里的控件。
constexpr qreal kCardRadius = 12.0;
constexpr qreal kBorderWidth = 1.0;

constexpr int kFooterMarginX = 14;
constexpr int kFooterMarginTop = 8;
constexpr int kFooterMarginBottom = 10;
constexpr int kFooterSpacing = 10;
constexpr int kCoordinateDecimals = 4;

// 连续滚轮 / 拖拽只渲染最后一帧：中间帧没人看得清，只会占满线程池。
constexpr int kRenderDelayMs = 60;
constexpr int kWheelNotch = 120;
constexpr double kZoomStep = 1.2;
// 按窗口的真实 DPR 渲染：150% 屏 round 成 2 会多算 78% 的像素，还要多一次上屏缩放。
constexpr qreal kMaxPixelRatio = 2.0;
// 每个线程至少分这么多行，太窄的条带起线程不划算。
constexpr int kMinRowsPerThread = 16;

// 色带：迭代次数每走满 kBandPeriod，颜色从底色走到高光再折回。
constexpr double kBandPeriod = 12.0;
// 色带大部分停在底色附近，只有靠近集合时才亮起来：亮环细，画面不糊。
constexpr double kWaveGamma = 1.6;
constexpr int kPaletteSize = 512;
// 波形查表的精度：三角波本来就走满 512 级色带，1024 已经看不出台阶。
constexpr int kWaveTableSize = 1024;

auto mix(const QColor &from, const QColor &to, double t) -> QColor
{
    return QColor::fromRgbF(
        from.redF() + (to.redF() - from.redF()) * t,
        from.greenF() + (to.greenF() - from.greenF()) * t,
        from.blueF() + (to.blueF() - from.blueF()) * t);
}

// 由主题色生成一条 512 级的色带。深浅两套方案的气质不同：
// 深色是"暗底上的一圈光"，浅色是"白纸上的一圈等高线"，
// 所以亮色只占色带末尾很短一段，大片区域留在底色附近。
auto buildRamp(const RenderJob &job) -> QVector<QRgb>
{
    const QColor &accent = job.accentColor;
    std::array<QColor, 4> stops;
    std::array<double, 4> positions;

    if (job.isDark) {
        stops
            = {mix(job.baseColor, accent, 0.10),
               accent.darker(230),
               accent,
               mix(accent, Qt::white, 0.85)};
        positions = {0.0, 0.50, 0.80, 1.0};
    } else {
        stops
            = {mix(job.baseColor, accent, 0.02),
               mix(accent, Qt::white, 0.88),
               mix(accent, Qt::white, 0.42),
               accent.darker(160)};
        positions = {0.0, 0.55, 0.82, 1.0};
    }

    QVector<QRgb> ramp(kPaletteSize);
    for (int i = 0; i < kPaletteSize; ++i) {
        const double position = double(i) / double(kPaletteSize - 1);
        qsizetype segment = 1;
        while (segment < qsizetype(positions.size()) - 1 && position > positions[size_t(segment)])
            ++segment;

        const double low = positions[size_t(segment) - 1];
        const double high = positions[size_t(segment)];
        const double local = high > low ? (position - low) / (high - low) : 0.0;
        ramp[i] = mix(stops[size_t(segment) - 1], stops[size_t(segment)], local).rgb();
    }
    return ramp;
}

using WaveTable = std::array<int, kWaveTableSize + 1>;

// 三角波 + pow 的查表版：取值只取决于 triangle，没必要每像素各算一次 pow。
auto waveTable() -> const WaveTable &
{
    static const WaveTable table = [] {
        WaveTable values{};
        for (int i = 0; i <= kWaveTableSize; ++i) {
            const double wave = std::pow(double(i) / kWaveTableSize, kWaveGamma);
            values[size_t(i)] = std::clamp(int(wave * (kPaletteSize - 1)), 0, kPaletteSize - 1);
        }
        return values;
    }();
    return table;
}

// 三角波映射：色带走到高光后原路折回，相邻两圈之间不会出现硬边。
auto rampColor(double iterations, const QVector<QRgb> &ramp, const WaveTable &wave) -> QRgb
{
    const double phase = iterations / kBandPeriod;
    const double wrapped = phase - std::floor(phase);
    const double triangle = wrapped < 0.5 ? wrapped * 2.0 : (1.0 - wrapped) * 2.0;
    return ramp[wave[size_t(std::clamp(int(triangle * kWaveTableSize), 0, kWaveTableSize))]];
}

// 主心形与二周期球内的点一定属于集合，不必再跑迭代。
auto isInsideSet(double cr, double ci) -> bool
{
    const double dx = cr - 0.25;
    const double q = dx * dx + ci * ci;
    if (q * (q + dx) <= 0.25 * ci * ci)
        return true;
    const double bx = cr + 1.0;
    return bx * bx + ci * ci <= 0.0625;
}

// 主题切换后分形要跟着换配色。换肤的主流程走 themeApplied，
// 这条指纹是兜底：每次渲染记下当时的调色板，绘制时再核对一次，对不上就补一帧。
auto paletteKey(const QPalette &palette) -> quint64
{
    constexpr quint64 kOffset = 1469598103934665603ULL;
    constexpr quint64 kPrime = 1099511628211ULL;

    quint64 key = kOffset;
    for (const QRgb rgb :
         {palette.color(QPalette::Window).rgb(),
          palette.color(QPalette::Base).rgb(),
          palette.color(QPalette::AlternateBase).rgb(),
          palette.color(QPalette::Text).rgb(),
          palette.color(QPalette::Mid).rgb(),
          palette.color(QPalette::Highlight).rgb()}) {
        key = (key ^ rgb) * kPrime;
    }
    return key;
}

// 放大倍数相对默认视图，比"复平面高度 0.003"更像给用户看的数字。
auto formatZoom(double factor) -> QString
{
    if (factor < 10.0)
        return QString::number(factor, 'f', 2);
    if (factor < 1000.0)
        return QString::number(factor, 'f', 0);
    return QString::number(factor, 'g', 3);
}

} // namespace

auto renderFractal(const std::shared_ptr<RenderJob> &job) -> QImage
{
    if (!job || job->width <= 0 || job->height <= 0)
        return {};

    const QVector<QRgb> ramp = buildRamp(*job);
    const WaveTable &wave = waveTable();
    const QRgb interior = job->interiorColor.rgb();

    QImage image(job->width, job->height, QImage::Format_RGB32);
    if (image.isNull())
        return {};
    image.setDevicePixelRatio(job->pixelRatio);

    const double width = job->width;
    const double height = std::max(1, job->height);
    const double halfWidth = job->scale * (width / height) / 2.0;
    const double minX = job->centerX - halfWidth;
    const double minY = job->centerY - job->scale / 2.0;

    // 各线程只写自己那几行。先 detach 一次再把裸指针发下去：scanLine() 会 detach，
    // 多线程各自调用它不是线程安全的。
    uchar *const base = image.bits();
    const qsizetype stride = image.bytesPerLine();

    const auto paintRows = [&](int first, int last) {
        for (int y = first; y < last; ++y) {
            if (y % kCancelCheckRows == 0 && job->cancelled.load(std::memory_order_relaxed))
                return;

            auto *line = reinterpret_cast<QRgb *>(base + qsizetype(y) * stride);
            const double ci = minY + job->scale * y / height;
            for (int x = 0; x < job->width; ++x) {
                const double cr = minX + 2.0 * halfWidth * x / width;

                if (isInsideSet(cr, ci)) {
                    line[x] = interior;
                    continue;
                }

                double zr = 0.0;
                double zi = 0.0;
                double zr2 = 0.0;
                double zi2 = 0.0;
                int iterations = 0;
                while (zr2 + zi2 <= 4.0 && iterations < kMaxIterations) {
                    zi = 2.0 * zr * zi + ci;
                    zr = zr2 - zi2 + cr;
                    zr2 = zr * zr;
                    zi2 = zi * zi;
                    ++iterations;
                }

                if (iterations >= kMaxIterations) {
                    line[x] = interior;
                    continue;
                }

                // 平滑着色：把逃逸前最后一刻的小数部分也算进去，色带才没有台阶。
                // log(sqrt(n)) 就是 0.5*log(n)，省掉一次开方。
                const double smooth = iterations + 1.0
                                      - std::log2(0.5 * std::log(std::max(zr2 + zi2, 4.0)));
                line[x] = rampColor(smooth, ramp, wave);
            }
        }
    };

    const int threads
        = std::clamp(job->height / kMinRowsPerThread, 1, std::max(1, QThread::idealThreadCount()));
    if (threads == 1) {
        paintRows(0, job->height);
    } else {
        std::vector<std::thread> workers;
        workers.reserve(size_t(threads - 1));
        for (int i = 1; i < threads; ++i) {
            workers
                .emplace_back(paintRows, job->height * i / threads, job->height * (i + 1) / threads);
        }
        // 起线程的那一帧自己也干一份，不多占一个核。
        paintRows(0, job->height / threads);
        for (std::thread &worker : workers)
            worker.join();
    }

    // 取消时和原来一样交回空图，调用方据此丢掉这一帧。
    if (job->cancelled.load(std::memory_order_relaxed))
        return {};
    return image;
}

class FractalPage::FractalPagePrivate
{
public:
    explicit FractalPagePrivate(FractalPage *q)
        : q_ptr(q)
    {}

    void setupUi();
    void scheduleRender();
    void startRender();
    void cancelRender();
    void discardRenderedImage();
    void updateStatus();
    void resetView();
    void zoomAt(const QPointF &position, double factor);
    void panBy(const QPoint &delta);

    // 页脚由布局摆在底部，它上面的区域就是画布。
    [[nodiscard]] auto canvasRect() const -> QRectF;
    // 主题颜色一律取自应用调色板：带 QSS 的控件，自己的 palette() 是半套的。
    [[nodiscard]] auto colors() const -> QPalette;
    [[nodiscard]] auto isDark() const -> bool;

    FractalPage *q_ptr = nullptr;
    QLabel *coordinates = nullptr;
    QLabel *zoom = nullptr;
    QPushButton *reset = nullptr;
    QWidget *footer = nullptr;

    QImage image;
    QFutureWatcher<QImage> watcher;
    QTimer renderTimer;
    std::shared_ptr<RenderJob> job;

    double centerX = kDefaultCenterX;
    double centerY = 0.0;
    double scale = kDefaultScale;

    QPoint lastPos;
    // 渲染完成前先把旧画面平移，拖拽才有即时反馈；新图到达后归零。
    QPointF panOffset;
    bool isDragging = false;
    // 同一时刻只允许一个渲染任务：视图又变了就先取消、等它退出再补一帧。
    bool isRenderPending = false;
    // 正在算的那一帧用的是哪套调色板；成图上屏时才转成 imagePaletteKey。
    // 两个分开记：任务被取消时不能把旧图说成新配色，否则对账会失效。
    quint64 jobPaletteKey = 0;
    // 当前这张图是按哪套调色板渲染的，用于主题切换后的对账。
    quint64 imagePaletteKey = 0;
};

void FractalPage::FractalPagePrivate::setupUi()
{
    coordinates = new QLabel(q_ptr);
    coordinates->setObjectName(u"FieldLabel"_s);

    zoom = new QLabel(q_ptr);
    zoom->setObjectName(u"HeroMeta"_s);

    reset = new QPushButton(q_ptr);
    reset->setCursor(Qt::PointingHandCursor);

    footer = new QWidget(q_ptr);
    auto *footerLayout = new QHBoxLayout(footer);
    footerLayout
        ->setContentsMargins(kFooterMarginX, kFooterMarginTop, kFooterMarginX, kFooterMarginBottom);
    footerLayout->setSpacing(kFooterSpacing);
    footerLayout->addWidget(coordinates);
    footerLayout->addStretch(1);
    footerLayout->addWidget(zoom);
    footerLayout->addWidget(reset);

    auto *layout = new QVBoxLayout(q_ptr);
    layout->setContentsMargins({});
    layout->setSpacing(0);
    // 画布由 paintEvent 直接绘制，这里只需要把它上方的高度让出来。
    layout->addStretch(1);
    layout->addWidget(footer);

    renderTimer.setInterval(kRenderDelayMs);
    renderTimer.setSingleShot(true);
}

auto FractalPage::FractalPagePrivate::canvasRect() const -> QRectF
{
    const qreal width = std::max(0.0, q_ptr->width() - 2.0 * kBorderWidth);
    const qreal height = std::max(0.0, footer->geometry().top() - kBorderWidth);
    return {kBorderWidth, kBorderWidth, width, height};
}

auto FractalPage::FractalPagePrivate::colors() const -> QPalette
{
    return QGuiApplication::palette();
}

auto FractalPage::FractalPagePrivate::isDark() const -> bool
{
    // 直接问主题：换肤期间控件自己的调色板会分几次刷新，拿它判断深浅并不可靠。
    return Utils::theme()->isDark();
}

void FractalPage::FractalPagePrivate::updateStatus()
{
    coordinates->setText(u"%1, %2"_s.arg(centerX, 0, 'f', kCoordinateDecimals)
                             .arg(centerY, 0, 'f', kCoordinateDecimals));
    // 语言无关，因此不用 tr()，Private 里也不该出现 tr()。
    zoom->setText(u"%1×"_s.arg(formatZoom(kDefaultScale / scale)));
}

void FractalPage::FractalPagePrivate::resetView()
{
    centerX = kDefaultCenterX;
    centerY = 0.0;
    scale = kDefaultScale;
    panOffset = QPointF();
    updateStatus();
    scheduleRender();
}

void FractalPage::FractalPagePrivate::zoomAt(const QPointF &position, double factor)
{
    const QRectF area = canvasRect();
    if (area.width() < 1.0 || area.height() < 1.0)
        return;

    const double next = std::clamp(scale * factor, kMinScale, kMaxScale);
    if (next == scale)
        return;

    // 以光标处的复数坐标为锚点缩放：放大的是"我看的地方"，而不是画面中心。
    const double nx = (position.x() - area.left()) / area.width() - 0.5;
    const double ny = (position.y() - area.top()) / area.height() - 0.5;
    const double aspect = area.width() / area.height();
    const double anchorX = centerX + nx * scale * aspect;
    const double anchorY = centerY + ny * scale;

    scale = next;
    centerX = anchorX - nx * scale * aspect;
    centerY = anchorY - ny * scale;
    panOffset = QPointF();
    updateStatus();
    scheduleRender();
}

void FractalPage::FractalPagePrivate::panBy(const QPoint &delta)
{
    const QRectF area = canvasRect();
    if (area.width() < 1.0 || area.height() < 1.0)
        return;

    const double aspect = area.width() / area.height();
    centerX -= delta.x() * scale * aspect / area.width();
    centerY -= delta.y() * scale / area.height();
    panOffset += QPointF(delta);
    updateStatus();
    scheduleRender();
}

void FractalPage::FractalPagePrivate::scheduleRender()
{
    renderTimer.start();
}

void FractalPage::FractalPagePrivate::startRender()
{
    // 上一个任务还在跑就先让它退出。不要在它运行期间 setFuture：
    // 换 future 之后旧任务的 finished 信号会和新的 future 对不上，
    // 一次绘制被白白丢掉，画面就会停在主题切换前的旧图上。
    if (watcher.isRunning()) {
        isRenderPending = true;
        cancelRender();
        return;
    }

    const QRectF area = canvasRect();
    if (area.width() < 1.0 || area.height() < 1.0)
        return;

    isRenderPending = false;

    job = std::make_shared<RenderJob>();
    job->pixelRatio = std::min(q_ptr->devicePixelRatioF(), kMaxPixelRatio);
    job->width = int(std::lround(area.width() * job->pixelRatio));
    job->height = int(std::lround(area.height() * job->pixelRatio));
    job->centerX = centerX;
    job->centerY = centerY;
    job->scale = scale;

    const QPalette pal = colors();
    // 指纹先记在 job 上，成图上屏时才转正：任务中途被取消，旧图不能被当成新配色。
    jobPaletteKey = paletteKey(pal);
    job->isDark = isDark();
    job->accentColor = pal.color(QPalette::Highlight);
    job->baseColor = pal.color(QPalette::AlternateBase);
    job->interiorColor = job->isDark ? mix(pal.color(QPalette::Window), Qt::black, 0.55)
                                     : mix(pal.color(QPalette::Text), job->accentColor, 0.12);

    watcher.setFuture(QtConcurrent::run(renderFractal, job));
}

void FractalPage::FractalPagePrivate::cancelRender()
{
    if (job)
        job->cancelled.store(true, std::memory_order_relaxed);
}

// 换肤后旧图是上一条色带画的，留着它就是"主题换了、颜色还是旧的"。
// 直接丢掉，让底板先用新底色，新图算完再画上去。
void FractalPage::FractalPagePrivate::discardRenderedImage()
{
    image = QImage();
    imagePaletteKey = 0;
    panOffset = QPointF();
    q_ptr->update();
}

FractalPage::FractalPage(QWidget *parent)
    : QWidget(parent)
    , d_ptr(std::make_unique<FractalPagePrivate>(this))
{
    setMinimumSize(320, 240);
    setCursor(Qt::OpenHandCursor);

    d_ptr->setupUi();
    retranslateUi();
    setupConnections();
}

FractalPage::~FractalPage()
{
    // 退出时如果还有渲染任务在跑，先取消再等它结束：
    // 否则窗口销毁后任务仍可能运行，进程退出时线程池收尾会很难看。
    d_ptr->cancelRender();
    if (d_ptr->watcher.isRunning())
        d_ptr->watcher.waitForFinished();
}

void FractalPage::setupConnections()
{
    connect(&d_ptr->watcher, &QFutureWatcher<QImage>::finished, this, &FractalPage::onRenderFinished);
    connect(&d_ptr->renderTimer, &QTimer::timeout, this, &FractalPage::onRenderTimeout);
    connect(d_ptr->reset, &QPushButton::clicked, this, &FractalPage::onResetClicked);
    // 换肤是整棵控件树重新 polish，Debug 下要好几秒；等它彻底结束再出图。
    connect(Utils::theme(), &Utils::Theme::themeApplied, this, &FractalPage::onThemeApplied);
}

void FractalPage::retranslateUi()
{
    d_ptr->reset->setText(tr("Reset view"));
    d_ptr->updateStatus();
}

void FractalPage::onRenderFinished()
{
    // 视图在渲染期间又变过：这一帧已经过时，直接接着渲染最新的一帧。
    if (d_ptr->isRenderPending) {
        d_ptr->startRender();
        return;
    }

    // 这条 finished 可能是上一个 future 排进事件队列的余党：
    // 此时直接取 result() 会阻塞 GUI 线程，等新任务自己发 finished 就好。
    if (!d_ptr->watcher.isFinished())
        return;

    const QImage result = d_ptr->watcher.result();
    if (result.isNull())
        return;

    d_ptr->image = result;
    // 指纹跟着成图一起生效，paintEvent 的对账才不会误判。
    d_ptr->imagePaletteKey = d_ptr->jobPaletteKey;
    d_ptr->panOffset = QPointF();
    update();
}

void FractalPage::onThemeApplied(bool dark)
{
    Q_UNUSED(dark)
    // 换肤不是连续手势，不必等 60ms 去抖：多等一帧就多一帧的错色。
    d_ptr->discardRenderedImage();
    d_ptr->startRender();
}

void FractalPage::onRenderTimeout()
{
    d_ptr->startRender();
}

void FractalPage::onResetClicked()
{
    resetView();
}

void FractalPage::resetView()
{
    d_ptr->resetView();
}

void FractalPage::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    // 卡片底板自己画：颜色取自应用调色板，尺寸与 QSS 的 #Card 一致。
    const QPalette pal = d_ptr->colors();
    painter.setPen(QPen(pal.color(QPalette::Mid), kBorderWidth));
    painter.setBrush(pal.color(QPalette::Base));
    const qreal inset = kBorderWidth / 2.0;
    painter.drawRoundedRect(
        QRectF(rect()).adjusted(inset, inset, -inset, -inset), kCardRadius, kCardRadius);

    // 兜底：万一有哪条主题通知漏了，绘制时发现配色和成图不一致就补一帧。
    if (!d_ptr->image.isNull() && d_ptr->imagePaletteKey != paletteKey(pal))
        d_ptr->scheduleRender();

    const QRectF area = d_ptr->canvasRect();
    if (area.width() < 1.0 || area.height() < 1.0)
        return;

    // 只把上面两个角磨圆，底部与页脚相接，保持一条直线。
    QPainterPath canvasPath;
    canvasPath.setFillRule(Qt::WindingFill);
    canvasPath.addRoundedRect(area, kCardRadius - kBorderWidth, kCardRadius - kBorderWidth);
    canvasPath.addRect(QRectF(area.left(), area.bottom() - kCardRadius, area.width(), kCardRadius));

    painter.save();
    painter.setClipPath(canvasPath);
    painter.fillRect(area, pal.color(QPalette::AlternateBase));
    if (!d_ptr->image.isNull())
        painter.drawImage(area.translated(d_ptr->panOffset), d_ptr->image);
    painter.restore();
}

void FractalPage::wheelEvent(QWheelEvent *event)
{
    const double notches = event->angleDelta().y() / double(kWheelNotch);
    if (notches == 0.0 || !d_ptr->canvasRect().contains(event->position())) {
        event->ignore();
        return;
    }

    d_ptr->zoomAt(event->position(), std::pow(kZoomStep, -notches));
    event->accept();
}

void FractalPage::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton || !d_ptr->canvasRect().contains(event->position()))
        return;

    d_ptr->isDragging = true;
    d_ptr->lastPos = event->position().toPoint();
    setCursor(Qt::ClosedHandCursor);
    event->accept();
}

void FractalPage::mouseMoveEvent(QMouseEvent *event)
{
    if (!d_ptr->isDragging)
        return;

    const QPoint position = event->position().toPoint();
    const QPoint delta = position - d_ptr->lastPos;
    d_ptr->lastPos = position;
    if (delta.isNull())
        return;

    d_ptr->panBy(delta);
    // 新图还在路上，先把旧图平移过去，拖拽过程才不会像卡住了。
    update();
}

void FractalPage::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton || !d_ptr->isDragging)
        return;

    d_ptr->isDragging = false;
    setCursor(Qt::OpenHandCursor);
    d_ptr->scheduleRender();
}

void FractalPage::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    d_ptr->scheduleRender();
}

void FractalPage::changeEvent(QEvent *event)
{
    // 深浅色不在这里处理：换肤过程中 PaletteChange 会来好几次，
    // 那时调色板还没到位，跟着它渲染会拿到半套颜色。走 themeApplied。
    if (event->type() == QEvent::LanguageChange)
        retranslateUi();
    QWidget::changeEvent(event);
}

} // namespace Fractal
