#pragma once

#include <QColor>
#include <QString>
#include <QtPlugin>

// 跨插件接口约定：只放头文件，不产生链接依赖。
// 提供方（danmakuplugin）实现它并注册到 PluginManager 的对象池，
// 消费方（pomodoroplugin）用 PluginManager::object<Danmaku::IDanmaku>() 取用。
namespace Danmaku {

class IDanmaku
{
public:
    virtual ~IDanmaku() = default;

    // text 会从主窗口右侧飘到左侧；color 无效时使用默认色。
    virtual void push(QString text, QColor color = {}) = 0;
};

} // namespace Danmaku

Q_DECLARE_INTERFACE(Danmaku::IDanmaku, "Youth.Qt.Danmaku/1.0")
