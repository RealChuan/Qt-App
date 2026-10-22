#include "danmakuservice.hpp"

#include "danmakuoverlay.hpp"

#include <QPointer>

namespace Danmaku {

class DanmakuService::DanmakuServicePrivate
{
public:
    QPointer<DanmakuOverlay> overlay;
};

DanmakuService::DanmakuService(QObject *parent)
    : QObject(parent)
    , d_ptr(std::make_unique<DanmakuServicePrivate>())
{}

DanmakuService::~DanmakuService() = default;

void DanmakuService::setOverlay(DanmakuOverlay *overlay)
{
    d_ptr->overlay = overlay;
}

void DanmakuService::push(QString text, QColor color)
{
    // 插件全部运行在主线程，直接调用即可；QPointer 保证浮层被销毁后不再访问。
    if (!d_ptr->overlay || text.isEmpty())
        return;

    d_ptr->overlay->push(text, color);
}

} // namespace Danmaku
