// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "icon.h"
#include "algorithm.h"
#include "qtcassert.h"
#include "stylehelper.h"
#include "utilsicons.h"

#include <QApplication>
#include <QDebug>
#include <QIcon>
#include <QImage>
#include <QPainter>
#include <QWidget>

namespace Utils {

static const qreal PunchEdgeWidth = 0.5;
static const qreal PunchEdgeIntensity = 0.6;

static QPixmap maskToColorAndAlpha(const QPixmap &mask, const QColor &color)
{
    QImage result(mask.toImage().convertToFormat(QImage::Format_ARGB32));
    result.setDevicePixelRatio(mask.devicePixelRatio());
    auto bitsStart = reinterpret_cast<QRgb *>(result.bits());
    const QRgb *bitsEnd = bitsStart + result.width() * result.height();
    const QRgb tint = color.rgb() & 0x00ffffff;
    const auto alpha = QRgb(color.alpha());
    for (QRgb *pixel = bitsStart; pixel < bitsEnd; ++pixel) {
        QRgb pixelAlpha = (((~*pixel) & 0xff) * alpha) >> 8;
        *pixel = (pixelAlpha << 24) | tint;
    }
    return QPixmap::fromImage(result);
}

using MaskAndColor = QPair<QPixmap, QColor>;
using MasksAndColors = QList<MaskAndColor>;
static MasksAndColors masksAndColors(const QList<IconMaskAndColor> &icon, int dpr)
{
    MasksAndColors result;
    for (const IconMaskAndColor &i : icon) {
        const QString &fileName = i.first.toUrlishString();
        const QColor color(i.second);
        const QString dprFileName = StyleHelper::availableImageResolutions(i.first.toUrlishString())
                                            .contains(dpr)
                                        ? StyleHelper::imageFileWithResolution(fileName, dpr)
                                        : fileName;
        QPixmap pixmap;
        if (!pixmap.load(dprFileName)) {
            pixmap = QPixmap(1, 1);
            qWarning() << "Could not load image: " << dprFileName;
        }
        result.append({pixmap, color});
    }
    return result;
}

static void smearPixmap(QPainter *painter, const QPixmap &pixmap, qreal radius)
{
    const qreal nagative = -radius - 0.01; // Workaround for QPainter rounding behavior
    const qreal positive = radius;
    painter->drawPixmap(QPointF(nagative, nagative), pixmap);
    painter->drawPixmap(QPointF(0, nagative), pixmap);
    painter->drawPixmap(QPointF(positive, nagative), pixmap);
    painter->drawPixmap(QPointF(positive, 0), pixmap);
    painter->drawPixmap(QPointF(positive, positive), pixmap);
    painter->drawPixmap(QPointF(0, positive), pixmap);
    painter->drawPixmap(QPointF(nagative, positive), pixmap);
    painter->drawPixmap(QPointF(nagative, 0), pixmap);
}

static QPixmap combinedMask(const MasksAndColors &masks, Icon::IconStyleOptions style)
{
    if (masks.count() == 1)
        return masks.first().first;

    QPixmap result(masks.first().first);
    QPainter p(&result);
    p.setCompositionMode(QPainter::CompositionMode_Darken);
    auto maskImage = masks.constBegin();
    maskImage++;
    for (; maskImage != masks.constEnd(); ++maskImage) {
        if (style & Icon::PunchEdges) {
            p.save();
            p.setOpacity(PunchEdgeIntensity);
            p.setCompositionMode(QPainter::CompositionMode_Lighten);
            smearPixmap(&p, maskToColorAndAlpha((*maskImage).first, Qt::white), PunchEdgeWidth);
            p.restore();
        }
        p.drawPixmap(0, 0, (*maskImage).first);
    }
    p.end();
    return result;
}

static QPixmap masksToIcon(const MasksAndColors &masks,
                           const QPixmap &combinedMask,
                           Icon::IconStyleOptions style)
{
    QPixmap result(combinedMask.size());
    result.setDevicePixelRatio(combinedMask.devicePixelRatio());
    result.fill(Qt::transparent);
    QPainter p(&result);

    for (MasksAndColors::const_iterator maskImage = masks.constBegin();
         maskImage != masks.constEnd();
         ++maskImage) {
        if (style & Icon::PunchEdges && maskImage != masks.constBegin()) {
            // Punch a transparent outline around an overlay.
            p.save();
            p.setOpacity(PunchEdgeIntensity);
            p.setCompositionMode(QPainter::CompositionMode_DestinationOut);
            smearPixmap(&p, maskToColorAndAlpha((*maskImage).first, Qt::white), PunchEdgeWidth);
            p.restore();
        }
        p.drawPixmap(0, 0, maskToColorAndAlpha((*maskImage).first, (*maskImage).second));
    }

    p.end();

    return result;
}

Icon::Icon() = default;

Icon::Icon(const QList<IconMaskAndColor> &args, Icon::IconStyleOptions style)
    : m_iconSourceList(args)
    , m_style(style)
{}

Icon::Icon(const FilePath &imageFileName)
    : m_iconSourceList({{imageFileName, Qt::white}})
{}

using OptMasksAndColors = std::optional<MasksAndColors>;

QIcon Icon::icon() const
{
    if (m_iconSourceList.isEmpty())
        return QIcon();

    if (m_style == None)
        return QIcon(m_iconSourceList.constFirst().first.toUrlishString());

    const int maxDpr = qRound(qApp->devicePixelRatio());
    if (maxDpr == m_lastDevicePixelRatio)
        return m_lastIcon;

    m_lastDevicePixelRatio = maxDpr;
    m_lastIcon = QIcon();
    for (int dpr = 1; dpr <= maxDpr; dpr++) {
        const MasksAndColors masks = masksAndColors(m_iconSourceList, dpr);
        const QPixmap combinedMask = Utils::combinedMask(masks, m_style);
        m_lastIcon.addPixmap(masksToIcon(masks, combinedMask, m_style), QIcon::Normal, QIcon::Off);
        const QColor disabledColor("88a0a0a0");
        const QPixmap disabledIcon = maskToColorAndAlpha(combinedMask, disabledColor);
        m_lastIcon.addPixmap(disabledIcon, QIcon::Disabled);
    }
    return m_lastIcon;
}

QPixmap Icon::pixmap(QIcon::Mode iconMode) const
{
    if (m_iconSourceList.isEmpty()) {
        return QPixmap();
    } else if (m_style == None) {
        return QPixmap(StyleHelper::dpiSpecificImageFile(
            m_iconSourceList.constFirst().first.toUrlishString()));
    } else {
        const MasksAndColors masks = masksAndColors(m_iconSourceList,
                                                    qRound(qApp->devicePixelRatio()));
        const QPixmap combinedMask = Utils::combinedMask(masks, m_style);
        return iconMode == QIcon::Disabled ? maskToColorAndAlpha(combinedMask, "88a0a0a0")
                                           : masksToIcon(masks, combinedMask, m_style);
    }
}

FilePath Icon::imageFilePath() const
{
    QTC_ASSERT(m_iconSourceList.length() == 1, return {});
    return m_iconSourceList.first().first;
}

QIcon Icon::sideBarIcon(const Icon &classic, const Icon &flat)
{
    QIcon result;

    const QPixmap pixmap = classic.pixmap();
    result.addPixmap(pixmap);
    // Ensure that the icon contains a disabled state of that size, since
    // Since we have icons with mixed sizes (e.g. DEBUG_START), and want to
    // avoid that QIcon creates scaled versions of missing QIcon::Disabled
    // sizes.
    result.addPixmap(StyleHelper::disabledSideBarIcon(pixmap), QIcon::Disabled);
    return result;
}

QIcon Icon::combinedIcon(const QList<QIcon> &icons)
{
    QIcon result;
    const qreal devicePixelRatio = QApplication::allWidgets().constFirst()->devicePixelRatio();
    for (const QIcon &icon : icons)
        for (const QIcon::Mode mode : {QIcon::Disabled, QIcon::Normal})
            for (const QSize &size : icon.availableSizes(mode))
                result.addPixmap(icon.pixmap(size, devicePixelRatio, mode), mode);
    return result;
}

QIcon Icon::combinedIcon(const QList<Icon> &icons)
{
    const QList<QIcon> qIcons = transform(icons, &Icon::icon);
    return combinedIcon(qIcons);
}

QIcon Icon::fromTheme(const QString &name)
{
    static QHash<QString, QIcon> cache;

    auto found = cache.find(name);
    if (found != cache.end())
        return *found;

    QIcon icon = QIcon::fromTheme(name);
    if (name == "go-next") {
        cache.insert(name, !icon.isNull() ? icon : QIcon(":/utils/images/arrow.png"));
    } else if (name == "edit-clear") {
        cache.insert(name, !icon.isNull() ? icon : Icons::EDIT_CLEAR.icon());
    } else if (name == "edit-clear-locationbar-rtl") {
        // KDE has custom icons for this. If these icons are not available we use the freedesktop
        // standard name "edit-clear" before falling back to a bundled resource.
        cache.insert(name, !icon.isNull() ? icon : fromTheme("edit-clear"));
    } else if (name == "edit-clear-locationbar-ltr") {
        cache.insert(name, !icon.isNull() ? icon : fromTheme("edit-clear"));
    } else {
        cache.insert(name, icon);
    }

    return cache[name];
}

} // namespace Utils
