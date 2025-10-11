// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#pragma once

#include "utils_global.h"

#include <QPen>
#include <QStyle>

QT_BEGIN_NAMESPACE
class QPalette;
class QPainter;
class QRect;
// Note, this is exported but in a private header as qtopengl depends on it.
// We should consider adding this as a public helper function.
void qt_blurImage(
    QPainter *p, QImage &blurImage, qreal radius, bool quality, bool alphaOnly, int transposed = 0);
QT_END_NAMESPACE

// Helper class holding all custom color values
namespace Utils::StyleHelper {

const unsigned int DEFAULT_BASE_COLOR = 0x666666;
const int progressFadeAnimationDuration = 600;
constexpr qreal defaultCardBgRounding = 3.75;

constexpr char C_ALIGN_ARROW[] = "alignarrow";
constexpr char C_DRAW_LEFT_BORDER[] = "drawleftborder";
constexpr char C_ELIDE_MODE[] = "elidemode";
constexpr char C_HIDE_BORDER[] = "hideborder";
constexpr char C_HIDE_ICON[] = "hideicon";
constexpr char C_HIGHLIGHT_WIDGET[] = "highlightWidget";
constexpr char C_LIGHT_COLORED[] = "lightColored";
constexpr char C_MINI_SPLITTER[] = "minisplitter";
constexpr char C_NOT_ELIDE_ASTERISK[] = "notelideasterisk";
constexpr char C_NO_ARROW[] = "noArrow";
constexpr char C_PANEL_WIDGET[] = "panelwidget";
constexpr char C_PANEL_WIDGET_SINGLE_ROW[] = "panelwidget_singlerow";
constexpr char C_SHOW_BORDER[] = "showborder";
constexpr char C_TOP_BORDER[] = "topBorder";
constexpr char C_TOOLBAR_ACTIONWIDGET[] = "toolbar_actionWidget";
constexpr char C_TABBAR_WHEELSCROLLING[] = "tabBarWheelScrolling";

constexpr char C_QT_SCALE_FACTOR_ROUNDING_POLICY[] = "QT_SCALE_FACTOR_ROUNDING_POLICY";

namespace SpacingTokens {
constexpr int PrimitiveXxs = 2;
constexpr int PrimitiveXs = 4;
constexpr int PrimitiveS = 6;
constexpr int PrimitiveM = 8;
constexpr int PrimitiveL = 12;
constexpr int PrimitiveXl = 16;
constexpr int PrimitiveXxl = 24;

// Top and bottom padding within the component
constexpr int PaddingVXxs = PrimitiveXxs;
constexpr int PaddingVXs = PrimitiveXs;
constexpr int PaddingVS = PrimitiveS;
constexpr int PaddingVM = PrimitiveM;
constexpr int PaddingVL = PrimitiveL;
constexpr int PaddingVXl = PrimitiveXl;
constexpr int PaddingVXxl = PrimitiveXxl;

// Left and right padding within the component
constexpr int PaddingHXxs = PrimitiveXxs;
constexpr int PaddingHXs = PrimitiveXs;
constexpr int PaddingHS = PrimitiveS;
constexpr int PaddingHM = PrimitiveM;
constexpr int PaddingHL = PrimitiveL;
constexpr int PaddingHXl = PrimitiveXl;
constexpr int PaddingHXxl = PrimitiveXxl;

// Gap between vertically (on top of each other) positioned elements
constexpr int GapVXxs = PrimitiveXxs;
constexpr int GapVXs = PrimitiveXs;
constexpr int GapVS = PrimitiveS;
constexpr int GapVM = PrimitiveM;
constexpr int GapVL = PrimitiveL;
constexpr int GapVXl = PrimitiveXl;
constexpr int GapVXxl = PrimitiveXxl;

// Gap between horizontally (from left to right) positioned elements
constexpr int GapHXxs = PrimitiveXxs;
constexpr int GapHXs = PrimitiveXs;
constexpr int GapHS = PrimitiveS;
constexpr int GapHM = PrimitiveM;
constexpr int GapHL = PrimitiveL;
constexpr int GapHXl = PrimitiveXl;
constexpr int GapHXxl = PrimitiveXxl;
}; // namespace SpacingTokens

constexpr int HighlightThickness = SpacingTokens::PrimitiveXxs;

enum class ToolbarStyle {
    Compact,
    Relaxed,
};

// Keep in sync with:
// SyleHelper::uiFontMetrics, ICore::uiConfigInformation, tst_manual_widgets_uifonts::main
enum UiElement {
    UiElementH1,
    UiElementH2,
    UiElementH3,
    UiElementH4,
    UiElementH5,
    UiElementH6,
    UiElementH6Capital,
    UiElementBody1,
    UiElementBody2,
    UiElementButtonMedium,
    UiElementButtonSmall,
    UiElementLabelMedium,
    UiElementLabelSmall,
    UiElementCaptionStrong,
    UiElementCaption,
    UiElementIconStandard,
    UiElementIconActive,
};

// Height of the project explorer navigation bar
UTILS_EXPORT int navigationWidgetHeight();
UTILS_EXPORT void setToolbarStyle(ToolbarStyle style);
UTILS_EXPORT ToolbarStyle toolbarStyle();
UTILS_EXPORT ToolbarStyle defaultToolbarStyle();

// This is our color table, all colors derive from baseColor
UTILS_EXPORT QColor requestedBaseColor();
UTILS_EXPORT QColor baseColor(bool lightColored = false);
UTILS_EXPORT QColor toolbarBaseColor(bool lightColored = false);
UTILS_EXPORT QColor panelTextColor(bool lightColored = false);
UTILS_EXPORT QColor highlightColor(bool lightColored = false);
UTILS_EXPORT QColor shadowColor(bool lightColored = false);
UTILS_EXPORT QColor borderColor(bool lightColored = false);
UTILS_EXPORT QColor toolBarBorderColor();
UTILS_EXPORT QColor mergedColors(const QColor &colorA, const QColor &colorB, int factor = 50);
UTILS_EXPORT QColor alphaBlendedColors(const QColor &colorA, const QColor &colorB);

UTILS_EXPORT QColor sidebarHighlight();
UTILS_EXPORT QColor sidebarShadow();
UTILS_EXPORT QColor toolBarDropShadowColor();
UTILS_EXPORT QColor notTooBrightHighlightColor();

UTILS_EXPORT QFont uiFont(UiElement element);
UTILS_EXPORT int uiFontLineHeight(UiElement element);
UTILS_EXPORT QString fontToCssProperties(const QFont &font);

UTILS_EXPORT void drawPanelBgRect(QPainter *painter, const QRectF &rect, const QBrush &brush);
UTILS_EXPORT void drawCardBg(QPainter *painter,
                             const QRectF &rect,
                             const QBrush &fill,
                             const QPen &pen = QPen(Qt::NoPen),
                             qreal rounding = defaultCardBgRounding);

// Gradients used for panels
UTILS_EXPORT void horizontalGradient(QPainter *painter,
                                     const QRect &spanRect,
                                     const QRect &clipRect,
                                     bool lightColored = false);
UTILS_EXPORT void verticalGradient(QPainter *painter,
                                   const QRect &spanRect,
                                   const QRect &clipRect,
                                   bool lightColored = false);
UTILS_EXPORT void menuGradient(QPainter *painter, const QRect &spanRect, const QRect &clipRect);
UTILS_EXPORT bool usePixmapCache();

UTILS_EXPORT QPixmap disabledSideBarIcon(const QPixmap &enabledicon);
UTILS_EXPORT void drawCornerImage(const QImage &img,
                                  QPainter *painter,
                                  const QRect &rect,
                                  int left = 0,
                                  int top = 0,
                                  int right = 0,
                                  int bottom = 0);

UTILS_EXPORT void tintImage(QImage &img, const QColor &tintColor);
UTILS_EXPORT QLinearGradient statusBarGradient(const QRect &statusBarRect);
UTILS_EXPORT void setPanelWidget(QWidget *widget, bool value = true);
UTILS_EXPORT void setPanelWidgetSingleRow(QWidget *widget, bool value = true);

UTILS_EXPORT
Qt::HighDpiScaleFactorRoundingPolicy defaultHighDpiScaleFactorRoundingPolicy();

class IconFontHelper
{
public:
    IconFontHelper(const QString &iconSymbol,
                   const QColor &color,
                   const QSize &size,
                   QIcon::Mode mode = QIcon::Normal,
                   QIcon::State state = QIcon::Off)
        : m_iconSymbol(iconSymbol)
        , m_color(color)
        , m_size(size)
        , m_mode(mode)
        , m_state(state)
    {}

    QString iconSymbol() const { return m_iconSymbol; }
    QColor color() const { return m_color; }
    QSize size() const { return m_size; }
    QIcon::Mode mode() const { return m_mode; }
    QIcon::State state() const { return m_state; }

private:
    QString m_iconSymbol;
    QColor m_color;
    QSize m_size;
    QIcon::Mode m_mode;
    QIcon::State m_state;
};

UTILS_EXPORT QIcon getIconFromIconFont(const QString &fontName,
                                       const QList<IconFontHelper> &parameters);
UTILS_EXPORT QIcon getIconFromIconFont(
    const QString &fontName, const QString &iconSymbol, int fontSize, int iconSize, QColor color);
UTILS_EXPORT QIcon getIconFromIconFont(const QString &fontName,
                                       const QString &iconSymbol,
                                       int fontSize,
                                       int iconSize);
UTILS_EXPORT QIcon getCursorFromIconFont(const QString &fontname,
                                         const QString &cursorFill,
                                         const QString &cursorOutline,
                                         int fontSize,
                                         int iconSize);

UTILS_EXPORT QString dpiSpecificImageFile(const QString &fileName);
UTILS_EXPORT QString imageFileWithResolution(const QString &fileName, int dpr);
UTILS_EXPORT QList<int> availableImageResolutions(const QString &fileName);

UTILS_EXPORT double luminance(const QColor &color);
UTILS_EXPORT bool isReadableOn(const QColor &background, const QColor &foreground);
// returns a foreground color readable on background (desiredForeground if already readable or adaption fails)
UTILS_EXPORT QColor ensureReadableOn(const QColor &background, const QColor &desiredForeground);
// modifies widget's palette QPalette::Base to color, leaves other colors of palette untouched
UTILS_EXPORT void modifyPaletteBase(QWidget *widget, const QColor &color);
} // namespace Utils::StyleHelper
