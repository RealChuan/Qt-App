#pragma once

#include "utils_global.hpp"

#include <QColor>
#include <QPainterPath>
#include <QPalette>
#include <QPoint>
#include <QRect>
#include <QRectF>
#include <QString>

class QGraphicsDropShadowEffect;
class QPainter;
class QWidget;

namespace Utils {

// 这里只放"要碰 QWidget / QPainter"的自由函数。它们和 utils.hpp 里的文件、进程、
// 基准工具分开：只做数据处理的模块，不必为了一个居中函数把控件头文件拖进来。

UTILS_EXPORT void restoreAndActivate(QWidget *window);

// 阴影样式。margin 是调用方在控件四周预留的空白，模糊半径按它的 2 倍取，
// 阴影才不会越出预留范围被父控件裁掉；另外两项只在换风格时覆盖。
struct DropShadow
{
    int margin = 10;
    QPoint offset{0, 2};
    QColor color{0, 0, 0, 70};
};

// 返回创建的 effect：个别调用方要在运行期改它（比如最大化时关掉）。
UTILS_EXPORT auto addDropShadow(QWidget *widget, const DropShadow &style = {})
    -> QGraphicsDropShadowEffect *;

// 对话框默认父窗口：由 core 插件在启动时注入。
[[nodiscard]] UTILS_EXPORT auto dialogParent() -> QWidget *;
UTILS_EXPORT void setDialogParentGetter(QWidget *(*getter)());

// 只设 Windows 的取整策略；Qt 6 的高 DPI 缩放本来就常开，名字别让人误会成开关。
UTILS_EXPORT void setupHighDpiRounding();
UTILS_EXPORT void loadFonts(const QString &fontPath);
// 两种居中语义不同，所以不共用一个名字：一个对父控件，一个对屏幕。
UTILS_EXPORT void centerInParent(QWidget *child, QWidget *parent);
UTILS_EXPORT void centerOnScreen(QWidget *window);
UTILS_EXPORT void setMacComboBoxStyle(QWidget *parent);
// 不设具体数值，而是按设备像素比的平方放大现有上限。
UTILS_EXPORT void scalePixmapCacheForDpr();

// 在自绘区域上盖一层随主题变化的遮罩，并居中排三行：标题、数字（可空）、提示（可空）。
// 字号对齐 shell 的排版语汇（HeroTitle 17 / 数字 40 / Placeholder 13），
// 让贪吃蛇、俄罗斯方块这类游戏页的"暂停 / 结束"长得一模一样。
UTILS_EXPORT void paintOverlay(
    QPainter &painter,
    const QRectF &area,
    const QPalette &palette,
    const QString &title,
    const QString &number = {},
    const QString &hint = {});

// 棋盘几何：rows×columns 个正方形格子居中摆进控件，格子尺寸取宽高各自除下来更小的那个。
// 棋盘类的绘制都从这里取坐标，免得每个游戏各写一遍同样的三条公式。
struct GridMetrics
{
    int cell = 0;
    int columns = 0;
    int rows = 0;
    QRect rect;

    [[nodiscard]] auto cellRect(const QPoint &position) const -> QRectF
    {
        return QRectF(rect.left() + position.x() * cell, rect.top() + position.y() * cell, cell, cell);
    }

    // 整行，给消行闪烁这类"按行盖一层"的画法用。
    [[nodiscard]] auto rowRect(int row) const -> QRectF
    {
        return QRectF(rect.left(), rect.top() + row * cell, rect.width(), cell);
    }
};

// padding 是棋盘四周的留白。任一方向除下来 <= 0 时 cell 为 0，调用方据此跳过绘制。
[[nodiscard]] UTILS_EXPORT auto fitGrid(
    const QSize &available, int columns, int rows, int padding = 0) -> GridMetrics;

// 棋盘底：1px 圆角描边 + AlternateBase 填充，返回路径供调用方 setClipPath。
// borderRole 默认 Mid；井这类要和卡片底色拉开的地方传 Dark。
UTILS_EXPORT auto paintBoardFrame(
    QPainter &painter,
    const QRectF &area,
    const QPalette &palette,
    QPalette::ColorRole borderRole = QPalette::Mid) -> QPainterPath;

// 圆角下限：小格子上按比例算出的圆角会小到看不出来，实心块和描边块共用这个底。
constexpr qreal kCellMinRadius = 2.0;

// 一颗圆角格子。inset 是格子内缩，radiusRatio 是圆角占格宽的比例。
struct CellStyle
{
    qreal inset = 1.0;
    qreal radiusRatio = 0.2;
};

UTILS_EXPORT void paintCell(
    QPainter &painter, const QRectF &cell, const QColor &color, const CellStyle &style = {});

// Idle 的"按 X 开始"：字号字重与 paintOverlay 同源，两个游戏才不会各写一份还写歪。
UTILS_EXPORT void paintIdleText(
    QPainter &painter, const QRectF &area, const QPalette &palette, const QString &text);

} // namespace Utils
