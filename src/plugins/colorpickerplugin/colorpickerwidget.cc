#include "colorpickerwidget.hpp"

#include <shell/host.hpp>

#include <QApplication>
#include <QClipboard>
#include <QCursor>
#include <QEvent>
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
constexpr int kOffset = 18;
constexpr int kPollIntervalMs = 60;
constexpr int kWidgetWidth = 200;
constexpr int kMagnifierZoom = 4;

} // namespace

class ColorPickerWidget::ColorPickerWidgetPrivate
{
public:
    explicit ColorPickerWidgetPrivate(ColorPickerWidget *q)
        : q_ptr(q)
    {}

    void setupUi();
    void refreshFromCursor();
    void reposition();

    ColorPickerWidget *q_ptr = nullptr;
    QTimer *timer = nullptr;
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
    magnifier = new QLabel(q_ptr);
    magnifier->setFixedSize(kPatchSize * kMagnifierZoom, kPatchSize * kMagnifierZoom);
    magnifier->setAlignment(Qt::AlignCenter);

    swatch = new QLabel(q_ptr);
    swatch->setFixedSize(18, 18);

    info = new QLabel(q_ptr);
    info->setObjectName("ColorPickerValue");
    info->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    hint = new QLabel(q_ptr);
    hint->setObjectName("ColorPickerHint");
    hint->setAlignment(Qt::AlignCenter);

    auto *valueRow = new QHBoxLayout;
    valueRow->setContentsMargins({});
    valueRow->setSpacing(8);
    valueRow->addStretch();
    valueRow->addWidget(swatch);
    valueRow->addWidget(info);
    valueRow->addStretch();

    auto *layout = new QVBoxLayout(q_ptr);
    layout->setContentsMargins(12, 12, 12, 10);
    layout->setSpacing(8);
    layout->addWidget(magnifier, 0, Qt::AlignCenter);
    layout->addLayout(valueRow);
    layout->addWidget(hint);

    timer = new QTimer(q_ptr);
    timer->setInterval(kPollIntervalMs);
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
        color = image.pixelColor(kPatchHalfSize, kPatchHalfSize);
        magnifier->setPixmap(
            patch.scaled(magnifier->size(), Qt::IgnoreAspectRatio, Qt::FastTransformation));
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

    q_ptr->move(target);
}

ColorPickerWidget::ColorPickerWidget(QWidget *parent)
    : QWidget(parent, Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint)
    , d_ptr(std::make_unique<ColorPickerWidgetPrivate>(this))
{
    setObjectName("ColorPickerWindow");
    setWindowTitle(tr("Color Picker"));
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedWidth(kWidgetWidth);
    d_ptr->setupUi();
    retranslateUi();

    connect(d_ptr->timer, &QTimer::timeout, this, &ColorPickerWidget::onPollTimeout);
}

void ColorPickerWidget::onPollTimeout()
{
    d_ptr->refreshFromCursor();
}

ColorPickerWidget::~ColorPickerWidget() = default;

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
