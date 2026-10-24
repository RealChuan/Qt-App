#include "colorpickerwidget.hpp"

#include <shell/host.hpp>
#include <utils/uihelpers.hpp>
#include <utils/utils.hpp>

#include <QApplication>
#include <QClipboard>
#include <QCursor>
#include <QEvent>
#include <QFontMetrics>
#include <QFrame>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QMouseEvent>
#include <QScreen>
#include <QTimer>
#include <QVBoxLayout>

namespace ColorPicker {

namespace {

constexpr int kPatchHalfSize = 10; // 取 (2*10+1)^2 的像素
constexpr int kPatchSize = 2 * kPatchHalfSize + 1;
// 光标到窗口外沿的距离，必须大于 kPatchHalfSize，否则浮窗（连阴影）
// 会压进取样框，把自身的半透明像素取进颜色里。
constexpr int kOffset = 12;
constexpr int kPollIntervalMs = 60;
constexpr int kMagnifierZoom = 4;
// 卡片四周留给阴影的边距，也是传给阴影的模糊半径（helper 内部按 ×2 设置）。
constexpr int kShadowMargin = 12;

} // namespace

class ColorPickerWidget::ColorPickerWidgetPrivate
{
public:
    explicit ColorPickerWidgetPrivate(ColorPickerWidget *q)
        : q_ptr(q)
    {}

    void setupUi();
    void fitToContent();
    void refreshFromCursor();
    void reposition();

    ColorPickerWidget *q_ptr = nullptr;
    QTimer *timer = nullptr;
    QFrame *card = nullptr;
    QLabel *magnifier = nullptr;
    QLabel *swatch = nullptr;
    QLabel *info = nullptr;
    QLabel *hint = nullptr;
    QColor color;
    QPixmap patch;
};

// 只建控件与布局；配色交给 pages.css，只有被取到的颜色本身是运行时值。
void ColorPickerWidget::ColorPickerWidgetPrivate::setupUi()
{
    // 底色和描边只能落在卡片上：顶层窗口带 WA_TranslucentBackground，
    // 同时带 WA_NoSystemBackground，自己画不出背景（命令面板同理）。
    card = new QFrame(q_ptr);
    card->setObjectName("ColorPickerCard");
    // 阴影把浮窗和后面分开，kShadowMargin 那圈是它的位置。
    Utils::addDropShadow(card, {.margin = kShadowMargin});

    magnifier = new QLabel(card);
    magnifier->setFixedSize(kPatchSize * kMagnifierZoom, kPatchSize * kMagnifierZoom);
    magnifier->setAlignment(Qt::AlignCenter);

    swatch = new QLabel(card);
    swatch->setFixedSize(18, 18);

    info = new QLabel(card);
    info->setObjectName("ColorPickerValue");
    info->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    hint = new QLabel(card);
    hint->setObjectName("ColorPickerHint");
    hint->setAlignment(Qt::AlignCenter);

    auto *valueRow = new QHBoxLayout;
    valueRow->setContentsMargins({});
    valueRow->setSpacing(8);
    valueRow->addStretch();
    valueRow->addWidget(swatch);
    valueRow->addWidget(info);
    valueRow->addStretch();

    auto *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(12, 12, 12, 10);
    cardLayout->setSpacing(8);
    cardLayout->addWidget(magnifier, 0, Qt::AlignCenter);
    cardLayout->addLayout(valueRow);
    cardLayout->addWidget(hint);

    auto *layout = new QVBoxLayout(q_ptr);
    layout->setContentsMargins(kShadowMargin, kShadowMargin, kShadowMargin, kShadowMargin);
    layout->setSpacing(0);
    layout->addWidget(card);

    timer = new QTimer(q_ptr);
    timer->setInterval(kPollIntervalMs);
}

// 窗口宽度由内容决定（写死会把 rgb 值裁掉）。色值位数会变，先用最宽的一串
// 把信息栏钉住，否则窗口会跟着鼠标左右抽动。
void ColorPickerWidget::ColorPickerWidgetPrivate::fitToContent()
{
    info->ensurePolished();
    const QFontMetrics metrics(info->font());
    // 必须用 setFixedWidth：setMinimumWidth 不改 sizeHint，adjustSize() 仍会压窄。
    info->setFixedWidth(metrics.horizontalAdvance(QStringLiteral("#FFFFFF  rgb(255, 255, 255)")));
    q_ptr->adjustSize();
}

void ColorPickerWidget::ColorPickerWidgetPrivate::refreshFromCursor()
{
    const QPoint cursor = QCursor::pos();
    QScreen *screen = QGuiApplication::screenAt(cursor);
    if (!screen)
        screen = QGuiApplication::primaryScreen();

    const QRect
        area(cursor.x() - kPatchHalfSize, cursor.y() - kPatchHalfSize, kPatchSize, kPatchSize);
    patch = screen->grabWindow(0, area.x(), area.y(), area.width(), area.height());

    if (!patch.isNull()) {
        const QImage image = patch.toImage();
        // 取图像正中：分数缩放下 grabWindow() 回来的不是 21x21 设备像素，
        // 按 kPatchHalfSize 索引会偏。
        color = image.pixelColor(image.width() / 2, image.height() / 2);

        // 缩放要按设备像素算：pixmap 带屏幕 DPR，缩到 magnifier->size()
        // 只能铺满标签的 2/3（1.5 倍屏上是 56/84），四周留出一圈空。
        const qreal dpr = screen->devicePixelRatio();
        QPixmap scaled = patch.scaled(
            (QSizeF(magnifier->size()) * dpr).toSize(),
            Qt::IgnoreAspectRatio,
            Qt::FastTransformation);
        scaled.setDevicePixelRatio(dpr);
        magnifier->setPixmap(scaled);
    }

    info->setText(QStringLiteral("%1  rgb(%2, %3, %4)")
                      .arg(color.name(QColor::HexRgb).toUpper())
                      .arg(color.red())
                      .arg(color.green())
                      .arg(color.blue()));
    // 色块的颜色只能来自运行时，其余外观都由样式表决定。
    swatch->setStyleSheet(QStringLiteral("background: %1; border-radius: 5px;").arg(color.name()));

    reposition();
}

void ColorPickerWidget::ColorPickerWidgetPrivate::reposition()
{
    const QPoint cursor = QCursor::pos();
    QScreen *screen = QGuiApplication::screenAt(cursor);
    if (!screen)
        return;

    const QRect available = screen->availableGeometry();
    QPoint target = cursor + QPoint(kOffset, kOffset);
    if (target.x() + q_ptr->width() > available.right())
        target.setX(cursor.x() - q_ptr->width() - kOffset);
    if (target.y() + q_ptr->height() > available.bottom())
        target.setY(cursor.y() - q_ptr->height() - kOffset);

    // 翻到光标左侧/上方时仍可能顶出屏幕，最后夹一次，保证整窗都露在外面。
    target.setX(qMax(target.x(), available.left()));
    target.setY(qMax(target.y(), available.top()));

    q_ptr->move(target);
}

ColorPickerWidget::ColorPickerWidget(QWidget *parent)
    : QWidget(parent, Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint)
    , d_ptr(std::make_unique<ColorPickerWidgetPrivate>(this))
{
    setObjectName("ColorPickerWindow");
    setWindowTitle(tr("Color Picker"));
    setAttribute(Qt::WA_TranslucentBackground);
    d_ptr->setupUi();
    retranslateUi();
    setupConnections();
}

void ColorPickerWidget::onPollTimeout()
{
    d_ptr->refreshFromCursor();
}

ColorPickerWidget::~ColorPickerWidget() = default;

void ColorPickerWidget::setupConnections()
{
    connect(d_ptr->timer, &QTimer::timeout, this, &ColorPickerWidget::onPollTimeout);
}

void ColorPickerWidget::retranslateUi()
{
    d_ptr->hint->setText(tr("Click to copy  •  Esc to close"));
}

void ColorPickerWidget::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
        retranslateUi();
    QWidget::changeEvent(event);
}

void ColorPickerWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    // 先把尺寸定下来：reposition() 要按真实大小决定往光标哪一侧翻转。
    d_ptr->fitToContent();
    d_ptr->timer->start();
    d_ptr->refreshFromCursor();
    setFocus();
}

void ColorPickerWidget::hideEvent(QHideEvent *event)
{
    d_ptr->timer->stop();
    QWidget::hideEvent(event);
}

void ColorPickerWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton)
        return;

    const QString name = d_ptr->color.name(QColor::HexRgb).toUpper();
    QApplication::clipboard()->setText(name);
    if (Shell::host())
        Shell::host()->showStatusMessage(tr("Copied %1").arg(name));
    hide();
}

void ColorPickerWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        hide();
        return;
    }
    QWidget::keyPressEvent(event);
}

} // namespace ColorPicker
