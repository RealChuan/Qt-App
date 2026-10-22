#pragma once

#include "utils_global.hpp"

#include <QSettings>

namespace Utils {

// 全局设置入口：userSettings 为用户级，installSettings 为系统级（安装级）。
// 由 main() 在启动早期调用 setupSettings() 注入，destroySettings() 负责释放。
UTILS_EXPORT QSettings &userSettings();
UTILS_EXPORT QSettings &installSettings();

UTILS_EXPORT void setupSettings(QSettings *user, QSettings *install);
UTILS_EXPORT void destroySettings();

// 仅在值偏离默认值时写入，保持配置文件干净。
template<typename T>
void setValueWithDefault(QSettings &settings, const QString &key, const T &value)
{
    if (value == T())
        settings.remove(key);
    else
        settings.setValue(key, value);
}

} // namespace Utils
