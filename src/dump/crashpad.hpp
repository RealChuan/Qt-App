#pragma once

#include "dump_global.hpp"

#include <memory>
#include <string>
#include <string_view>

namespace Dump {

// Crashpad 初始化器：构造时尝试拉起 handler，失败不会抛异常（崩溃上报
// 属于增强能力，不该阻止应用启动），用 isValid() 查询结果。
class DUMP_EXPORT Crashpad
{
    Q_DISABLE_COPY_MOVE(Crashpad)
public:
    explicit Crashpad(std::string_view dumpPath,
                      std::string_view libexecPath,
                      std::string_view reportUrl,
                      bool crashReportingEnabled);

    ~Crashpad();

    [[nodiscard]] auto isValid() const -> bool;
    [[nodiscard]] auto dumpPath() const -> std::string;
    [[nodiscard]] auto reportUrl() const -> std::string;
    [[nodiscard]] auto isReportingEnabled() const -> bool;

private:
    class CrashpadPrivate;
    std::unique_ptr<CrashpadPrivate> d_ptr;
};

} // namespace Dump
