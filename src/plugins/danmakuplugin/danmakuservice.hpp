#pragma once

#include <api/danmaku/idanmaku.hpp>

#include <QObject>
#include <QPointer>

namespace Danmaku {

class DanmakuOverlay;

// 服务的实现类：QObject + Q_INTERFACES，才能被 qobject_cast 到 IDanmaku。
class DanmakuService final : public QObject, public IDanmaku
{
    Q_OBJECT
    Q_INTERFACES(Danmaku::IDanmaku)

public:
    explicit DanmakuService(QObject *parent = nullptr);

    void setOverlay(DanmakuOverlay *overlay);

    void push(QString text, QColor color = {}) override;

private:
    QPointer<DanmakuOverlay> m_overlay;
};

} // namespace Danmaku
