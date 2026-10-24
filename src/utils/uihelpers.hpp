#pragma once

#include "utils_global.hpp"

#include <QColor>
#include <QPoint>
#include <QString>

class QGraphicsDropShadowEffect;
class QPainter;
class QPalette;
class QRectF;
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

} // namespace Utils
