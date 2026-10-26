#include "uihelpers.hpp"

#include <QApplication>
#include <QComboBox>
#include <QDir>
#include <QFileInfo>
#include <QFont>
#include <QFontDatabase>
#include <QGraphicsDropShadowEffect>
#include <QGuiApplication>
#include <QPainter>
#include <QPalette>
#include <QPixmapCache>
#include <QScreen>
#include <QStyle>
#include <QStyleFactory>
#include <QWidget>

#include <algorithm>

using namespace Qt::StringLiterals;

namespace Utils {

namespace {

// core 插件启动时注入，供各处的对话框取默认父窗口。
QWidget *(*s_dialogParentGetter)() = nullptr;

// Qt 6 的 QColor 浮点接口用 float，常量也保持 float，避免 double→float 截断。
constexpr float kScrimAlpha = 0.80F;
// 遮罩上的字是游戏里最该被看见的东西，字号按"一眼能读"给，不跟着正文的 13px 走。
constexpr int kTitlePixelSize = 26;
constexpr int kNumberPixelSize = 46;
constexpr int kHintPixelSize = 15;
// 开局提示比正文大一档，字重跟标题一致：它是"该按键了"的号召，不是占位符。
constexpr int kIdlePixelSize = 16;
// 棋盘外框两个游戏必须一样，否则切换页面时边框会"跳"一下。
constexpr int kBoardBorderWidth = 1;
constexpr qreal kBoardRadius = 10.0;

auto withPixelSize(const QFont &font, int pixelSize, QFont::Weight weight) -> QFont
{
    QFont result = font;
    result.setPixelSize(pixelSize);
    result.setWeight(weight);
    return result;
}

} // namespace

void loadFonts(const QString &fontPath)
{
    const QDir dir(fontPath);
    if (!dir.exists())
        return;

    const QFileInfoList fonts = dir.entryInfoList({u"*.ttf"_s}, QDir::Files);
    for (const QFileInfo &info : fonts) {
        const int fontId = QFontDatabase::addApplicationFont(info.absoluteFilePath());
        if (fontId == -1) {
            qWarning().noquote() << u"Loading Fonts file: %1 Failed."_s.arg(info.fileName());
            continue;
        }
        qInfo().noquote() << u"Loading Fonts file: %1."_s.arg(info.fileName())
                          << QFontDatabase::applicationFontFamilies(fontId);
    }
}

void centerInParent(QWidget *child, QWidget *parent)
{
    const QRect target
        = QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, child->size(), parent->rect());
    child->move(target.topLeft());
}

void centerOnScreen(QWidget *window)
{
    const QScreen *screen = window->screen() ? window->screen() : QGuiApplication::primaryScreen();
    if (!screen)
        return;

    const QRect target = QStyle::alignedRect(
        Qt::LeftToRight, Qt::AlignCenter, window->size(), screen->availableGeometry());
    window->move(target.topLeft());
}

void setDialogParentGetter(QWidget *(*getter)())
{
    s_dialogParentGetter = getter;
}

auto dialogParent() -> QWidget *
{
    return s_dialogParentGetter ? s_dialogParentGetter() : nullptr;
}

void restoreAndActivate(QWidget *window)
{
    if (window->isMinimized())
        window->setWindowState(window->windowState() & ~Qt::WindowMinimized);

    window->show();
    window->raise();
    window->activateWindow();
}

auto addDropShadow(QWidget *widget, const DropShadow &style) -> QGraphicsDropShadowEffect *
{
    // 用半透明黑而不是纯灰：深色主题下灰边会亮得像一圈描边。
    auto *effect = new QGraphicsDropShadowEffect(widget);
    effect->setOffset(style.offset);
    effect->setColor(style.color);
    // Qt 的阴影大约向外扩 blurRadius / 2，取边距的 2 倍正好铺满预留的那圈空白。
    effect->setBlurRadius(style.margin * 2.0);
    widget->setGraphicsEffect(effect);
    return effect;
}

void setupHighDpiRounding()
{
#ifdef Q_OS_WIN
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(
        Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif
}

void setMacComboBoxStyle(QWidget *parent)
{
#ifdef Q_OS_MACOS
    const auto comboBoxes = parent->findChildren<QComboBox *>();
    for (QComboBox *comboBox : comboBoxes)
        comboBox->setStyle(QStyleFactory::create(u"Fusion"_s));
#else
    Q_UNUSED(parent)
#endif
}

void scalePixmapCacheForDpr()
{
    const int originalLimit = QPixmapCache::cacheLimit();
    const qreal dpr = qApp->devicePixelRatio();
    const qreal multiplier = std::clamp(dpr * dpr, 1.0, 4.0);
    QPixmapCache::setCacheLimit(qRound(originalLimit * multiplier));
}

void paintOverlay(
    QPainter &painter,
    const QRectF &area,
    const QPalette &palette,
    const QString &title,
    const QString &number,
    const QString &hint)
{
    // 遮罩用 Window 而不是黑色：深色方案下才是"压暗"，浅色方案下是"蒙白"。
    QColor scrim = palette.color(QPalette::Window);
    scrim.setAlphaF(kScrimAlpha);
    painter.setPen(Qt::NoPen);
    painter.setBrush(scrim);
    painter.drawRect(area);

    const qreal titleHeight = kTitlePixelSize * 1.8;
    const qreal numberHeight = number.isEmpty() ? 0.0 : kNumberPixelSize * 1.35;
    const qreal hintHeight = hint.isEmpty() ? 0.0 : kHintPixelSize * 2.0;
    qreal top = area.center().y() - (titleHeight + numberHeight + hintHeight) / 2.0;

    painter.setFont(withPixelSize(painter.font(), kTitlePixelSize, QFont::DemiBold));
    painter.setPen(palette.color(QPalette::Text));
    painter.drawText(QRectF(area.left(), top, area.width(), titleHeight), Qt::AlignCenter, title);
    top += titleHeight;

    if (!number.isEmpty()) {
        painter.setFont(withPixelSize(painter.font(), kNumberPixelSize, QFont::DemiBold));
        painter
            .drawText(QRectF(area.left(), top, area.width(), numberHeight), Qt::AlignCenter, number);
        top += numberHeight;
    }

    if (!hint.isEmpty()) {
        painter.setFont(withPixelSize(painter.font(), kHintPixelSize, QFont::Normal));
        // 提示不用 PlaceholderText：那档颜色在遮罩上几乎读不出来。
        QColor hintColor = palette.color(QPalette::Text);
        hintColor.setAlphaF(0.85F);
        painter.setPen(hintColor);
        painter.drawText(QRectF(area.left(), top, area.width(), hintHeight), Qt::AlignCenter, hint);
    }
}

auto fitGrid(const QSize &available, int columns, int rows, int padding) -> GridMetrics
{
    GridMetrics grid;
    if (columns <= 0 || rows <= 0)
        return grid;

    // 留白从可用尺寸里先扣掉，格子才是"棋盘那一块"里的正方形，而不是控件里的。
    const int width = available.width() - 2 * padding;
    const int height = available.height() - 2 * padding;
    if (width <= 0 || height <= 0)
        return grid;

    grid.cell = std::min(width / columns, height / rows);
    if (grid.cell <= 0)
        return grid;

    grid.columns = columns;
    grid.rows = rows;

    const int boardWidth = grid.cell * columns;
    const int boardHeight = grid.cell * rows;
    grid.rect = QRect(
        (available.width() - boardWidth) / 2,
        (available.height() - boardHeight) / 2,
        boardWidth,
        boardHeight);
    return grid;
}

auto paintBoardFrame(
    QPainter &painter, const QRectF &area, const QPalette &palette, QPalette::ColorRole borderRole)
    -> QPainterPath
{
    // 线宽居中于路径：内缩半个线宽，1px 的边才不会有一半被控件边界切掉。
    const qreal inset = kBoardBorderWidth / 2.0;
    QPainterPath shape;
    shape.addRoundedRect(area.adjusted(inset, inset, -inset, -inset), kBoardRadius, kBoardRadius);

    painter.setPen(QPen(palette.color(borderRole), kBoardBorderWidth));
    painter.setBrush(palette.color(QPalette::AlternateBase));
    painter.drawPath(shape);
    return shape;
}

void paintCell(QPainter &painter, const QRectF &cell, const QColor &color, const CellStyle &style)
{
    const qreal radius = std::max(kCellMinRadius, cell.width() * style.radiusRatio);

    painter.setPen(Qt::NoPen);
    painter.setBrush(color);
    painter.drawRoundedRect(
        cell.adjusted(style.inset, style.inset, -style.inset, -style.inset), radius, radius);
}

void paintIdleText(
    QPainter &painter, const QRectF &area, const QPalette &palette, const QString &text)
{
    painter.setFont(withPixelSize(painter.font(), kIdlePixelSize, QFont::DemiBold));
    // 用正文色而不是 PlaceholderText：这句是要人照做的，不是灰掉的占位说明。
    painter.setPen(palette.color(QPalette::Text));
    painter.drawText(area, Qt::AlignCenter, text);
}

} // namespace Utils
