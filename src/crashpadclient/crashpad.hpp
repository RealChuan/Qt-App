#pragma once

#include "crashpadclient_global.hpp"

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace Crashpad {

// 一条待处理（尚未上传、也还没被用户处理）的崩溃报告。
// 只保留界面需要的字段，crashpad 的类型不出这个模块。
struct CRASHPADCLIENT_EXPORT ReportInfo
{
    std::string uuid;           // 数据库内的唯一标识，标记已处理时用
    std::int64_t createdAt = 0; // 生成时间（Unix 秒）
    std::uint64_t size = 0;     // 报告与附件的总字节数
};

// Crashpad 客户端初始化器：构造时尝试拉起 handler，失败不会抛异常（崩溃上报
// 属于增强能力，不该阻止应用启动），用 isValid() 查询结果。
class CRASHPADCLIENT_EXPORT Client
{
    Q_DISABLE_COPY_MOVE(Client)
public:
    explicit Client(
        std::string_view dumpPath,
        std::string_view libexecPath,
        std::string_view reportUrl,
        bool crashReportingEnabled);

    ~Client();

    [[nodiscard]] auto isValid() const -> bool;
    [[nodiscard]] auto dumpPath() const -> std::string;
    [[nodiscard]] auto reportUrl() const -> std::string;
    [[nodiscard]] auto isReportingEnabled() const -> bool;

    // 上次运行遗留的崩溃报告，按生成时间升序。进程崩溃时自己已经退出，
    // 所以只能由下一次启动来询问用户。
    [[nodiscard]] auto pendingReports() const -> std::vector<ReportInfo>;

    // 把当前所有待处理报告移到 completed 状态，避免下次启动重复提示。
    void discardReports();

private:
    class ClientPrivate;
    std::unique_ptr<ClientPrivate> d_ptr;
};

} // namespace Crashpad
