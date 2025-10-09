// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "stylehelper.h"

#include "algorithm.h"
#include "hostosinfo.h"
#include "qtcassert.h"

#include <QApplication>
#include <QCommonStyle>
#include <QFileInfo>
#include <QFontDatabase>
#include <QIcon>
#include <QPainter>
#include <QPainterPath>
#include <QPixmapCache>
#include <QStyleOption>
#include <QWindow>

#include <qmath.h>

// Clamps float color values within (0, 255)
static int clamp(float x)
{
    const int val = x > 255 ? 255 : static_cast<int>(x);
    return val < 0 ? 0 : val;
}

// Clamps float color values within (0, 255)
/*
static int range(float x, int min, int max)
{
    int val = x > max ? max : x;
    return val < min ? min : val;
}
*/

namespace Utils {

static StyleHelper::ToolbarStyle s_toolbarStyle = StyleHelper::ToolbarStyle::Compact;
// Invalid by default, setBaseColor needs to be called at least once
static QColor s_baseColor;
static QColor s_requestedBaseColor;

QColor StyleHelper::mergedColors(const QColor &colorA, const QColor &colorB, int factor)
{
    const int maxFactor = 100;
    QColor tmp = colorA;
    tmp.setRed((tmp.red() * factor) / maxFactor + (colorB.red() * (maxFactor - factor)) / maxFactor);
    tmp.setGreen((tmp.green() * factor) / maxFactor
                 + (colorB.green() * (maxFactor - factor)) / maxFactor);
    tmp.setBlue((tmp.blue() * factor) / maxFactor
                + (colorB.blue() * (maxFactor - factor)) / maxFactor);
    return tmp;
}

QColor StyleHelper::alphaBlendedColors(const QColor &colorA, const QColor &colorB)
{
    const int alpha = colorB.alpha();
    const int antiAlpha = 255 - alpha;

    return QColor((colorA.red() * antiAlpha + colorB.red() * alpha) / 255,
                  (colorA.green() * antiAlpha + colorB.green() * alpha) / 255,
                  (colorA.blue() * antiAlpha + colorB.blue() * alpha) / 255);
}

QColor StyleHelper::sidebarHighlight()
{
    return QColor(255, 255, 255, 40);
}

QColor StyleHelper::sidebarShadow()
{
    return QColor(0, 0, 0, 40);
}

QColor StyleHelper::toolBarDropShadowColor()
{
    return QColor(0, 0, 0, 70);
}

int StyleHelper::navigationWidgetHeight()
{
    return s_toolbarStyle == ToolbarStyle::Compact ? 24 : 30;
}

void StyleHelper::setToolbarStyle(ToolbarStyle style)
{
    s_toolbarStyle = style;
}

StyleHelper::ToolbarStyle StyleHelper::toolbarStyle()
{
    return s_toolbarStyle;
}

StyleHelper::ToolbarStyle StyleHelper::defaultToolbarStyle()
{
    return ToolbarStyle::Compact;
}

QColor StyleHelper::notTooBrightHighlightColor()
{
    QColor highlightColor = QApplication::palette().highlight().color();
    if (0.5 * highlightColor.saturationF() + 0.75 - highlightColor.valueF() < 0)
        highlightColor.setHsvF(highlightColor.hsvHueF(),
                               0.1 + highlightColor.saturationF() * 2.0,
                               highlightColor.valueF());
    return highlightColor;
}

QColor StyleHelper::panelTextColor(bool lightColored)
{
    if (!lightColored)
        return Qt::white;
    else
        return Qt::black;
}

QColor StyleHelper::baseColor(bool lightColored)
{
    static const QColor windowColor = QApplication::palette().color(QPalette::Window);
    static const bool windowColorAsBase = false;

    return (lightColored || windowColorAsBase) ? windowColor : s_baseColor;
}

QColor StyleHelper::requestedBaseColor()
{
    return s_requestedBaseColor;
}

QColor StyleHelper::toolbarBaseColor(bool lightColored)
{
    return StyleHelper::baseColor(lightColored);
}

QColor StyleHelper::highlightColor(bool lightColored)
{
    QColor result = baseColor(lightColored);
    if (!lightColored)
        result.setHsv(result.hue(), clamp(result.saturation()), clamp(result.value() * 1.16));
    else
        result.setHsv(result.hue(), clamp(result.saturation()), clamp(result.value() * 1.06));
    return result;
}

QColor StyleHelper::shadowColor(bool lightColored)
{
    QColor result = baseColor(lightColored);
    result.setHsv(result.hue(), clamp(result.saturation() * 1.1), clamp(result.value() * 0.70));
    return result;
}

QColor StyleHelper::borderColor(bool lightColored)
{
    QColor result = baseColor(lightColored);
    result.setHsv(result.hue(), result.saturation(), result.value() / 2);
    return result;
}

QColor StyleHelper::toolBarBorderColor()
{
    const QColor base = baseColor();
    return QColor::fromHsv(base.hue(), base.saturation(), clamp(base.value() * 0.80f));
}

static void verticalGradientHelper(QPainter *p,
                                   const QRect &spanRect,
                                   const QRect &rect,
                                   bool lightColored)
{
    QColor highlight = StyleHelper::highlightColor(lightColored);
    QColor shadow = StyleHelper::shadowColor(lightColored);
    QLinearGradient grad(spanRect.topRight(), spanRect.topLeft());
    grad.setColorAt(0, highlight.lighter(117));
    grad.setColorAt(1, shadow.darker(109));
    p->fillRect(rect, grad);

    QColor light(255, 255, 255, 80);
    p->setPen(light);
    p->drawLine(rect.topRight() - QPoint(1, 0), rect.bottomRight() - QPoint(1, 0));
    QColor dark(0, 0, 0, 90);
    p->setPen(dark);
    p->drawLine(rect.topLeft(), rect.bottomLeft());
}

void StyleHelper::verticalGradient(QPainter *painter,
                                   const QRect &spanRect,
                                   const QRect &clipRect,
                                   bool lightColored)
{
    if (StyleHelper::usePixmapCache()) {
        QColor keyColor = baseColor(lightColored);
        const QString key = QString::asprintf("mh_vertical %d %d %d %d %d",
                                              spanRect.width(),
                                              spanRect.height(),
                                              clipRect.width(),
                                              clipRect.height(),
                                              keyColor.rgb());

        QPixmap pixmap;
        if (!QPixmapCache::find(key, &pixmap)) {
            pixmap = QPixmap(clipRect.size());
            QPainter p(&pixmap);
            QRect rect(0, 0, clipRect.width(), clipRect.height());
            verticalGradientHelper(&p, spanRect, rect, lightColored);
            p.end();
            QPixmapCache::insert(key, pixmap);
        }

        painter->drawPixmap(clipRect.topLeft(), pixmap);
    } else {
        verticalGradientHelper(painter, spanRect, clipRect, lightColored);
    }
}

static void horizontalGradientHelper(QPainter *p,
                                     const QRect &spanRect,
                                     const QRect &rect,
                                     bool lightColored)
{
    if (lightColored) {
        QLinearGradient shadowGradient(rect.topLeft(), rect.bottomLeft());
        shadowGradient.setColorAt(0, 0xf0f0f0);
        shadowGradient.setColorAt(1, 0xcfcfcf);
        p->fillRect(rect, shadowGradient);
        return;
    }

    QColor base = StyleHelper::baseColor(lightColored);
    QColor highlight = StyleHelper::highlightColor(lightColored);
    QColor shadow = StyleHelper::shadowColor(lightColored);
    QLinearGradient grad(rect.topLeft(), rect.bottomLeft());
    grad.setColorAt(0, highlight.lighter(120));
    if (rect.height() == StyleHelper::navigationWidgetHeight()) {
        grad.setColorAt(0.4, highlight);
        grad.setColorAt(0.401, base);
    }
    grad.setColorAt(1, shadow);
    p->fillRect(rect, grad);

    QLinearGradient shadowGradient(spanRect.topLeft(), spanRect.topRight());
    shadowGradient.setColorAt(0, QColor(0, 0, 0, 30));
    QColor lighterHighlight;
    lighterHighlight = highlight.lighter(130);
    lighterHighlight.setAlpha(100);
    shadowGradient.setColorAt(0.7, lighterHighlight);
    shadowGradient.setColorAt(1, QColor(0, 0, 0, 40));
    p->fillRect(rect, shadowGradient);
}

void StyleHelper::horizontalGradient(QPainter *painter,
                                     const QRect &spanRect,
                                     const QRect &clipRect,
                                     bool lightColored)
{
    if (StyleHelper::usePixmapCache()) {
        QColor keyColor = baseColor(lightColored);
        const QString key = QString::asprintf("mh_horizontal %d %d %d %d %d %d",
                                              spanRect.width(),
                                              spanRect.height(),
                                              clipRect.width(),
                                              clipRect.height(),
                                              keyColor.rgb(),
                                              spanRect.x());

        QPixmap pixmap;
        if (!QPixmapCache::find(key, &pixmap)) {
            pixmap = QPixmap(clipRect.size());
            QPainter p(&pixmap);
            QRect rect = QRect(0, 0, clipRect.width(), clipRect.height());
            horizontalGradientHelper(&p, spanRect, rect, lightColored);
            p.end();
            QPixmapCache::insert(key, pixmap);
        }

        painter->drawPixmap(clipRect.topLeft(), pixmap);

    } else {
        horizontalGradientHelper(painter, spanRect, clipRect, lightColored);
    }
}

static void menuGradientHelper(QPainter *p, const QRect &spanRect, const QRect &rect)
{
    QLinearGradient grad(spanRect.topLeft(), spanRect.bottomLeft());
    QColor menuColor = StyleHelper::mergedColors(StyleHelper::baseColor(),
                                                 QColor(244, 244, 244),
                                                 25);
    grad.setColorAt(0, menuColor.lighter(112));
    grad.setColorAt(1, menuColor);
    p->fillRect(rect, grad);
}

void StyleHelper::drawPanelBgRect(QPainter *painter, const QRectF &rect, const QBrush &brush)
{
    if (toolbarStyle() == ToolbarStyle::Compact) {
        painter->fillRect(rect.toRect(), brush);
    } else {
        constexpr int margin = 2;
        constexpr int radius = 5;
        QPainterPath path;
        path.addRoundedRect(rect.adjusted(margin, margin, -margin, -margin), radius, radius);
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);
        painter->fillPath(path, brush);
        painter->restore();
    }
}

void StyleHelper::drawCardBg(
    QPainter *painter, const QRectF &rect, const QBrush &fill, const QPen &pen, qreal rounding)
{
    const qreal strokeWidth = pen.style() == Qt::NoPen ? 0 : pen.widthF();
    const qreal strokeShrink = strokeWidth / 2;
    const QRectF itemRectAdjusted = rect.adjusted(strokeShrink,
                                                  strokeShrink,
                                                  -strokeShrink,
                                                  -strokeShrink);
    const qreal roundingAdjusted = rounding - strokeShrink;
    QPainterPath itemOutlinePath;
    itemOutlinePath.addRoundedRect(itemRectAdjusted, roundingAdjusted, roundingAdjusted);

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setBrush(fill);
    painter->setPen(pen);
    painter->drawPath(itemOutlinePath);
    painter->restore();
}

void StyleHelper::menuGradient(QPainter *painter, const QRect &spanRect, const QRect &clipRect)
{
    if (StyleHelper::usePixmapCache()) {
        const QString key = QString::asprintf("mh_menu %d %d %d %d %d",
                                              spanRect.width(),
                                              spanRect.height(),
                                              clipRect.width(),
                                              clipRect.height(),
                                              StyleHelper::baseColor().rgb());

        QPixmap pixmap;
        if (!QPixmapCache::find(key, &pixmap)) {
            pixmap = QPixmap(clipRect.size());
            QPainter p(&pixmap);
            QRect rect = QRect(0, 0, clipRect.width(), clipRect.height());
            menuGradientHelper(&p, spanRect, rect);
            p.end();
            QPixmapCache::insert(key, pixmap);
        }

        painter->drawPixmap(clipRect.topLeft(), pixmap);
    } else {
        menuGradientHelper(painter, spanRect, clipRect);
    }
}

bool StyleHelper::usePixmapCache()
{
    return true;
}

QPixmap StyleHelper::disabledSideBarIcon(const QPixmap &enabledicon)
{
    QImage im = enabledicon.toImage().convertToFormat(QImage::Format_ARGB32);
    for (int y = 0; y < im.height(); ++y) {
        auto scanLine = reinterpret_cast<QRgb *>(im.scanLine(y));
        for (int x = 0; x < im.width(); ++x) {
            QRgb pixel = *scanLine;
            char intensity = char(qGray(pixel));
            *scanLine = qRgba(intensity, intensity, intensity, qAlpha(pixel));
            ++scanLine;
        }
    }
    return QPixmap::fromImage(im);
}

// Draws a CSS-like border image where the defined borders are not stretched
// Unit for rect, left, top, right and bottom is user pixels
void StyleHelper::drawCornerImage(const QImage &img,
                                  QPainter *painter,
                                  const QRect &rect,
                                  int left,
                                  int top,
                                  int right,
                                  int bottom)
{
    // source rect for drawImage() calls needs to be specified in DIP unit of the image
    const qreal imagePixelRatio = img.devicePixelRatio();
    const qreal leftDIP = left * imagePixelRatio;
    const qreal topDIP = top * imagePixelRatio;
    const qreal rightDIP = right * imagePixelRatio;
    const qreal bottomDIP = bottom * imagePixelRatio;

    const QSize size = img.size();
    if (top > 0) { //top
        painter->drawImage(QRectF(rect.left() + left, rect.top(), rect.width() - right - left, top),
                           img,
                           QRectF(leftDIP, 0, size.width() - rightDIP - leftDIP, topDIP));
        if (left > 0) //top-left
            painter->drawImage(QRectF(rect.left(), rect.top(), left, top),
                               img,
                               QRectF(0, 0, leftDIP, topDIP));
        if (right > 0) //top-right
            painter->drawImage(QRectF(rect.left() + rect.width() - right, rect.top(), right, top),
                               img,
                               QRectF(size.width() - rightDIP, 0, rightDIP, topDIP));
    }
    //left
    if (left > 0)
        painter->drawImage(QRectF(rect.left(), rect.top() + top, left, rect.height() - top - bottom),
                           img,
                           QRectF(0, topDIP, leftDIP, size.height() - bottomDIP - topDIP));
    //center
    painter->drawImage(QRectF(rect.left() + left,
                              rect.top() + top,
                              rect.width() - right - left,
                              rect.height() - bottom - top),
                       img,
                       QRectF(leftDIP,
                              topDIP,
                              size.width() - rightDIP - leftDIP,
                              size.height() - bottomDIP - topDIP));
    if (right > 0) //right
        painter->drawImage(QRectF(rect.left() + rect.width() - right,
                                  rect.top() + top,
                                  right,
                                  rect.height() - top - bottom),
                           img,
                           QRectF(size.width() - rightDIP,
                                  topDIP,
                                  rightDIP,
                                  size.height() - bottomDIP - topDIP));
    if (bottom > 0) { //bottom
        painter->drawImage(QRectF(rect.left() + left,
                                  rect.top() + rect.height() - bottom,
                                  rect.width() - right - left,
                                  bottom),
                           img,
                           QRectF(leftDIP,
                                  size.height() - bottomDIP,
                                  size.width() - rightDIP - leftDIP,
                                  bottomDIP));
        if (left > 0) //bottom-left
            painter->drawImage(QRectF(rect.left(), rect.top() + rect.height() - bottom, left, bottom),
                               img,
                               QRectF(0, size.height() - bottomDIP, leftDIP, bottomDIP));
        if (right > 0) //bottom-right
            painter->drawImage(QRectF(rect.left() + rect.width() - right,
                                      rect.top() + rect.height() - bottom,
                                      right,
                                      bottom),
                               img,
                               QRectF(size.width() - rightDIP,
                                      size.height() - bottomDIP,
                                      rightDIP,
                                      bottomDIP));
    }
}

// Tints an image with tintColor, while preserving alpha and lightness
void StyleHelper::tintImage(QImage &img, const QColor &tintColor)
{
    QPainter p(&img);
    p.setCompositionMode(QPainter::CompositionMode_Screen);

    for (int x = 0; x < img.width(); ++x) {
        for (int y = 0; y < img.height(); ++y) {
            QRgb rgbColor = img.pixel(x, y);
            int alpha = qAlpha(rgbColor);
            QColor c = QColor(rgbColor);

            if (alpha > 0) {
                c.toHsl();
                qreal l = c.lightnessF();
                QColor newColor = QColor::fromHslF(tintColor.hslHueF(),
                                                   tintColor.hslSaturationF(),
                                                   l);
                newColor.setAlpha(alpha);
                img.setPixel(x, y, newColor.rgba());
            }
        }
    }
}

QLinearGradient StyleHelper::statusBarGradient(const QRect &statusBarRect)
{
    QLinearGradient grad(statusBarRect.topLeft(),
                         QPoint(statusBarRect.center().x(), statusBarRect.bottom()));
    QColor startColor = shadowColor().darker(164);
    QColor endColor = baseColor().darker(130);
    grad.setColorAt(0, startColor);
    grad.setColorAt(1, endColor);
    return grad;
}

void StyleHelper::setPanelWidget(QWidget *widget, bool value)
{
    widget->setProperty(C_PANEL_WIDGET, value);
}

void StyleHelper::setPanelWidgetSingleRow(QWidget *widget, bool value)
{
    widget->setProperty(C_PANEL_WIDGET_SINGLE_ROW, value);
}

Qt::HighDpiScaleFactorRoundingPolicy StyleHelper::defaultHighDpiScaleFactorRoundingPolicy()
{
    return HostOsInfo::isMacHost() ? Qt::HighDpiScaleFactorRoundingPolicy::Unset
                                   : Qt::HighDpiScaleFactorRoundingPolicy::Round;
}

QIcon StyleHelper::getIconFromIconFont(const QString &fontName,
                                       const QList<IconFontHelper> &parameters)
{
    QTC_ASSERT(QFontDatabase::hasFamily(fontName), {});

    QIcon icon;

    for (const IconFontHelper &p : parameters) {
        const int maxDpr = qRound(qApp->devicePixelRatio());
        for (int dpr = 1; dpr <= maxDpr; dpr++) {
            QPixmap pixmap(p.size() * dpr);
            pixmap.setDevicePixelRatio(dpr);
            pixmap.fill(Qt::transparent);

            QFont font(fontName);
            font.setPixelSize(p.size().height());

            QPainter painter(&pixmap);
            painter.save();
            painter.setPen(p.color());
            painter.setFont(font);
            painter.drawText(QRectF(QPoint(0, 0), p.size()), p.iconSymbol());
            painter.restore();

            icon.addPixmap(pixmap, p.mode(), p.state());
        }
    }

    return icon;
}

QIcon StyleHelper::getIconFromIconFont(
    const QString &fontName, const QString &iconSymbol, int fontSize, int iconSize, QColor color)
{
    QTC_ASSERT(QFontDatabase::hasFamily(fontName), {});

    QIcon icon;
    QSize size(iconSize, iconSize);

    const int maxDpr = qRound(qApp->devicePixelRatio());
    for (int dpr = 1; dpr <= maxDpr; dpr++) {
        QPixmap pixmap(size * dpr);
        pixmap.setDevicePixelRatio(dpr);
        pixmap.fill(Qt::transparent);

        QFont font(fontName);
        font.setPixelSize(fontSize);

        QPainter painter(&pixmap);
        painter.save();
        painter.setPen(color);
        painter.setFont(font);
        painter.drawText(QRectF(QPoint(0, 0), size), Qt::AlignCenter, iconSymbol);
        painter.restore();

        icon.addPixmap(pixmap);
    }

    return icon;
}

QIcon StyleHelper::getIconFromIconFont(const QString &fontName,
                                       const QString &iconSymbol,
                                       int fontSize,
                                       int iconSize)
{
    QColor penColor = QApplication::palette("QWidget").color(QPalette::Normal, QPalette::ButtonText);
    return getIconFromIconFont(fontName, iconSymbol, fontSize, iconSize, penColor);
}

QIcon StyleHelper::getCursorFromIconFont(const QString &fontName,
                                         const QString &cursorFill,
                                         const QString &cursorOutline,
                                         int fontSize,
                                         int iconSize)
{
    QTC_ASSERT(QFontDatabase::hasFamily(fontName), {});

    const QColor outlineColor = Qt::black;
    const QColor fillColor = Qt::white;

    QIcon icon;
    QSize size(iconSize, iconSize);

    const int maxDpr = qRound(qApp->devicePixelRatio());
    for (int dpr = 1; dpr <= maxDpr; dpr++) {
        QPixmap pixmap(size * dpr);
        pixmap.setDevicePixelRatio(dpr);
        pixmap.fill(Qt::transparent);

        QFont font(fontName);
        font.setPixelSize(fontSize);

        QPainter painter(&pixmap);
        painter.save();
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setRenderHint(QPainter::TextAntialiasing, true);
        painter.setRenderHint(QPainter::LosslessImageRendering, true);
        painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

        painter.setFont(font);
        painter.setPen(outlineColor);
        painter.drawText(QRectF(QPointF(0.0, 0.0), size), Qt::AlignCenter, cursorOutline);

        painter.setPen(fillColor);
        painter.drawText(QRectF(QPointF(0.0, 0.0), size), Qt::AlignCenter, cursorFill);

        painter.restore();

        icon.addPixmap(pixmap);
    }

    return icon;
}

QString StyleHelper::dpiSpecificImageFile(const QString &fileName)
{
    // See QIcon::addFile()
    if (qApp->devicePixelRatio() > 1.0) {
        const QString atDprfileName = imageFileWithResolution(fileName,
                                                              qRound(qApp->devicePixelRatio()));
        if (QFileInfo::exists(atDprfileName))
            return atDprfileName;
    }
    return fileName;
}

QString StyleHelper::imageFileWithResolution(const QString &fileName, int dpr)
{
    return qt_findAtNxFile(fileName, dpr);
}

QList<int> StyleHelper::availableImageResolutions(const QString &fileName)
{
    QList<int> result;
    const int maxResolutions = qApp->devicePixelRatio();
    for (int i = 1; i <= maxResolutions; ++i)
        if (QFileInfo::exists(imageFileWithResolution(fileName, i)))
            result.append(i);
    return result;
}

double StyleHelper::luminance(const QColor &color)
{
    // calculate the luminance based on
    // https://www.w3.org/TR/2008/REC-WCAG20-20081211/#relativeluminancedef
    auto val = [](const double &colorVal) {
        return colorVal < 0.03928 ? colorVal / 12.92 : std::pow((colorVal + 0.055) / 1.055, 2.4);
    };

    static QHash<QRgb, double> cache;
    QHash<QRgb, double>::iterator it = cache.find(color.rgb());
    if (it == cache.end()) {
        it = cache.insert(color.rgb(),
                          0.2126 * val(color.redF()) + 0.7152 * val(color.greenF())
                              + 0.0722 * val(color.blueF()));
    }
    return it.value();
}

static double contrastRatio(const QColor &color1, const QColor &color2)
{
    // calculate the contrast ratio based on
    // https://www.w3.org/TR/2008/REC-WCAG20-20081211/#contrast-ratiodef
    auto contrast = (StyleHelper::luminance(color1) + .05) / (StyleHelper::luminance(color2) + .05);
    if (contrast < 1)
        return 1 / contrast;
    return contrast;
}

bool StyleHelper::isReadableOn(const QColor &background, const QColor &foreground)
{
    // following the W3C Recommendation on contrast for large Text
    // https://www.w3.org/TR/2008/REC-WCAG20-20081211/#contrast-ratiodef
    return contrastRatio(background, foreground) > 3;
}

QColor StyleHelper::ensureReadableOn(const QColor &background, const QColor &desiredForeground)
{
    if (isReadableOn(background, desiredForeground))
        return desiredForeground;

    int h, s, v;
    QColor foreground = desiredForeground;
    foreground.getHsv(&h, &s, &v);
    // adjust the color value to ensure better readability
    if (luminance(background) < .5)
        v = v + 64;
    else if (v >= 64)
        v = v - 64;
    v %= 256;

    foreground.setHsv(h, s, v);
    if (!isReadableOn(background, foreground)) {
        s = (s + 128) % 256; // adjust the saturation to ensure better readability
        foreground.setHsv(h, s, v);
        if (!isReadableOn(background, foreground)) // we failed to create some better foreground
            return desiredForeground;
    }
    return foreground;
}

static const QStringList &applicationFontFamilies()
{
    const static QStringList families = [] {
        // Font is either installed in the system, or was loaded from share/qtcreator/fonts/
        const QStringList candidates = {"Inter", "Inter Variable"};
        const QString family = Utils::findOrDefault(candidates, &QFontDatabase::hasFamily);
        return family.isEmpty() ? QStringList() : QStringList(family);
    }();
    return families;
}

static const QStringList &brandFontFamilies()
{
    const static QStringList families = [] {
        const int id = QFontDatabase::addApplicationFont(":/studiofonts/TitilliumWeb-Regular.ttf");
        return id >= 0 ? QFontDatabase::applicationFontFamilies(id) : QStringList();
    }();
    return families;
}

struct UiFontMetrics
{
    // Original "text token" values are defined in pixels
    const int pixelSize = -1;
    const int lineHeight = -1;
    const QFont::Weight weight = QFont::Normal;
};

static const UiFontMetrics &uiFontMetrics(StyleHelper::UiElement element)
{
    static const std::map<StyleHelper::UiElement, UiFontMetrics> metrics{
        {StyleHelper::UiElementH1, {36, 54, QFont::DemiBold}},
        {StyleHelper::UiElementH2, {28, 44, QFont::DemiBold}},
        {StyleHelper::UiElementH3, {16, 20, QFont::Bold}},
        {StyleHelper::UiElementH4, {16, 20, QFont::Bold}},
        {StyleHelper::UiElementH5, {14, 16, QFont::DemiBold}},
        {StyleHelper::UiElementH6, {12, 14, QFont::DemiBold}},
        {StyleHelper::UiElementH6Capital, {12, 14, QFont::DemiBold}},
        {StyleHelper::UiElementBody1, {14, 20, QFont::Light}},
        {StyleHelper::UiElementBody2, {12, 20, QFont::Light}},
        {StyleHelper::UiElementButtonMedium, {12, 16, QFont::Bold}},
        {StyleHelper::UiElementButtonSmall, {10, 12, QFont::Bold}},
        {StyleHelper::UiElementLabelMedium, {12, 16, QFont::DemiBold}},
        {StyleHelper::UiElementLabelSmall, {10, 12, QFont::DemiBold}},
        {StyleHelper::UiElementCaptionStrong, {10, 12, QFont::DemiBold}},
        {StyleHelper::UiElementCaption, {10, 12, QFont::Normal}},
        {StyleHelper::UiElementIconStandard, {12, 16, QFont::Medium}},
        {StyleHelper::UiElementIconActive, {12, 16, QFont::DemiBold}},
    };
    QTC_ASSERT(metrics.count(element) > 0, return metrics.at(StyleHelper::UiElementCaptionStrong));
    return metrics.at(element);
}

QFont StyleHelper::uiFont(UiElement element)
{
    QFont font;

    switch (element) {
    case UiElementH1:
        font.setFamilies(brandFontFamilies());
        font.setWordSpacing(2);
        break;
    case UiElementH2: font.setFamilies(brandFontFamilies()); break;
    case UiElementH3:
    case UiElementH6Capital: font.setCapitalization(QFont::AllUppercase); [[fallthrough]];
    default:
        if (!applicationFontFamilies().isEmpty())
            font.setFamilies(applicationFontFamilies());
        break;
    }

    const UiFontMetrics &metrics = uiFontMetrics(element);

    // On macOS, by default 72 dpi are assumed for conversion between point and pixel size.
    // For non-macOS, it is 96 dpi.
    constexpr qreal defaultDpi = HostOsInfo::isMacHost() ? 72.0 : 96.0;
    constexpr qreal pixelsToPointSizeFactor = 72.0 / defaultDpi;
    const qreal qrealPointSize = metrics.pixelSize * pixelsToPointSizeFactor;
    font.setPointSizeF(qrealPointSize);

    // Intermediate font weights can produce blurry rendering and are harder to read.
    // For "non-retina" screens, apply the weight only for some fonts.
    static const bool isHighDpi = qApp->devicePixelRatio() >= 2;
    const bool setWeight = isHighDpi || element == UiElementCaptionStrong || element <= UiElementH4;
    if (setWeight)
        font.setWeight(metrics.weight);

    return font;
}

int StyleHelper::uiFontLineHeight(UiElement element)
{
    const UiFontMetrics &metrics = uiFontMetrics(element);
    const qreal lineHeightToPixelSizeRatio = qreal(metrics.lineHeight) / metrics.pixelSize;
    const QFontInfo fontInfo(uiFont(element));
    return qCeil(fontInfo.pixelSize() * lineHeightToPixelSizeRatio);
}

QString StyleHelper::fontToCssProperties(const QFont &font)
{
    const QString fontSize = font.pixelSize() != -1 ? QString::number(font.pixelSize()) + "px"
                                                    : QString::number(font.pointSizeF()) + "pt";
    const QString fontStyle = QLatin1String(font.style() == QFont::StyleNormal   ? "normal"
                                            : font.style() == QFont::StyleItalic ? "italic"
                                                                                 : "oblique");
    const QString fontShorthand = fontStyle + " " + QString::number(font.weight()) + " " + fontSize
                                  + " '" + font.family() + "'";
    const QString textDecoration = QLatin1String(font.underline() ? "underline" : "none");
    const QString textTransform = QLatin1String(
        font.capitalization() == QFont::AllUppercase   ? "uppercase"
        : font.capitalization() == QFont::AllLowercase ? "lowercase"
                                                       : "none");
    const QString propertyTemplate = "%1: %2";
    const QStringList cssProperties = {
        propertyTemplate.arg("font").arg(fontShorthand),
        propertyTemplate.arg("text-decoration").arg(textDecoration),
        propertyTemplate.arg("text-transform").arg(textTransform),
        propertyTemplate.arg("word-spacing").arg(font.wordSpacing()),
    };
    const QString fontCssStyle = cssProperties.join("; ");
    return fontCssStyle;
}

} // namespace Utils
