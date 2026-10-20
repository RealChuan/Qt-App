#include "colorpickerwidget.hpp"

#include <shell/host.hpp>

#include <QApplication>
#include <QClipboard>
#include <QCursor>
#include <QGuiApplication>
#include <QKeyEvent>
#include <QLabel>
#include <QMouseEvent>
#include <QScreen>
#include <QTimer>
#include <QVBoxLayout>

namespace ColorPicker {

namespace {
constexpr int kPatchHalfSize = 10; // 取 (2*10+1)^2 的像素
constexpr int kOffset = 18;
constexpr int kPollIntervalMs = 60;
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
    QLabel *info = nullptr;
    QColor color;
    QPixmap patch;
};

void ColorPickerWidget::ColorPickerWidgetPrivate::setupUi()
{
    magnifier = new QLabel(q_ptr);
    magnifier->setFixedSize((2 * kPatchHalfSize + 1) * 4, (2 * kPatchHalfSize + 1) * 4);
    magnifier->setAlignment(Qt::AlignCenter);

    info = new QLabel(q_ptr);
    info->setAlignment(Qt::AlignCenter);

    auto *hint = new QLabel(QObject::tr("Click to copy  •  Esc to close"), q_ptr);
    hint->setAlignment(Qt::AlignCenter);
    hint->setStyleSheet(QStringLiteral("color: #888;"));

    auto *layout = new QVBoxLayout(q_ptr);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->addWidget(magnifier, 0, Qt::AlignCenter);
    layout->addWidget(info);
    layout->addWidget(hint);

    q_ptr->setStyleSheet(QStringLiteral("ColorPickerWidget { background: #22242b;"
                                        " border-radius: 8px; }"
                                        "QLabel { color: #eee;"
                                        " font-family: Consolas, monospace; }"));

    timer = new QTimer(q_ptr);
    timer->setInterval(kPollIntervalMs);
    QObject::connect(timer, &QTimer::timeout, q_ptr, [this] { refreshFromCursor(); });
}

void ColorPickerWidget::ColorPickerWidgetPrivate::refreshFromCursor()
{
    const QPoint cursor = QCursor::pos();
    QScreen *screen = QGuiApplication::screenAt(cursor);
    if (!screen)
        screen = QGuiApplication::primaryScreen();

    const QRect area(cursor.x() - kPatchHalfSize,
                     cursor.y() - kPatchHalfSize,
                     2 * kPatchHalfSize + 1,
                     2 * kPatchHalfSize + 1);
    patch = screen->grabWindow(0, area.x(), area.y(), area.width(), area.height());

    if (!patch.isNull()) {
        const QImage image = patch.toImage();
        color = image.pixelColor(kPatchHalfSize, kPatchHalfSize);
        magnifier->setPixmap(patch.scaled(magnifier->size(),
                                          Qt::IgnoreAspectRatio,
                                          Qt::FastTransformation));
    }

    info->setText(QStringLiteral("%1\nrgb(%2, %3, %4)")
                      .arg(color.name(QColor::HexRgb).toUpper())
                      .arg(color.red())
                      .arg(color.green())
                      .arg(color.blue()));
    info->setStyleSheet(QStringLiteral("color: %1; font-size: 15px; font-weight: 600;")
                            .arg(color.lightness() > 128 ? "#111" : "#fff"));

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
    setWindowTitle(tr("Color Picker"));
    setFixedWidth(200);
    d_ptr->setupUi();
}

ColorPickerWidget::~ColorPickerWidget() = default;

void ColorPickerWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    d_ptr->timer->start();
    d_ptr->refreshFromCursor();
    activateWindow();
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
