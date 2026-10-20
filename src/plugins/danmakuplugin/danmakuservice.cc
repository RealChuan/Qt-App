#include "danmakuservice.hpp"

#include "danmakuoverlay.hpp"

#include <QMetaObject>

namespace Danmaku {

DanmakuService::DanmakuService(QObject *parent)
    : QObject(parent)
{}

void DanmakuService::setOverlay(DanmakuOverlay *overlay)
{
    m_overlay = overlay;
}

void DanmakuService::push(QString text, QColor color)
{
    if (!m_overlay || text.isEmpty())
        return;

    // 调用方可能来自工作线程（比如番茄钟的 worker），统一切回浮层所在线程。
    QMetaObject::invokeMethod(
        m_overlay,
        [overlay = m_overlay.data(), text = std::move(text), color] {
            overlay->push(text, color);
        },
        Qt::QueuedConnection);
}

} // namespace Danmaku
