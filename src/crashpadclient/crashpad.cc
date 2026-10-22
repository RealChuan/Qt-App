#include "crashpad.hpp"

#include <crashpad/client/crash_report_database.h>
#include <crashpad/client/crashpad_client.h>
#include <crashpad/client/settings.h>
#include <crashpad/util/misc/metrics.h>

#include <QLoggingCategory>
#include <QString>

#include <algorithm>
#include <filesystem>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#endif

Q_LOGGING_CATEGORY(lcCrashpad, "qtapp.crashpad", QtInfoMsg)

namespace Crashpad {

namespace {

#ifdef _WIN32
auto toWide(std::string_view text) -> std::wstring
{
    if (text.empty())
        return {};

    const int size
        = MultiByteToWideChar(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), nullptr, 0);
    if (size <= 0)
        return {};

    std::wstring wide(static_cast<std::size_t>(size), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), wide.data(), size);
    return wide;
}

auto toBaseFilePath(std::string_view utf8) -> base::FilePath
{
    return base::FilePath(toWide(utf8));
}
#else
auto toBaseFilePath(std::string_view utf8) -> base::FilePath
{
    return base::FilePath(std::string(utf8));
}
#endif

} // namespace

class Client::ClientPrivate
{
public:
    ClientPrivate(
        std::string_view dumpDir,
        std::string_view libexecDir,
        std::string_view url,
        bool reportingEnabled)
        : dumpPath(dumpDir)
        , libexecPath(libexecDir)
        , reportUrl(url)
        , crashReportingEnabled(reportingEnabled)
    {
        valid = initialize();
    }

    auto initialize() -> bool
    {
        std::error_code error;
        std::filesystem::create_directories(dumpPath, error);
        if (error) {
            qCWarning(lcCrashpad).noquote()
                << "cannot create dump directory" << QString::fromStdString(dumpPath)
                << QString::fromStdString(error.message());
            return false;
        }

        std::filesystem::path handlerPath(libexecPath);
        handlerPath /= "crashpad_handler";
#ifdef _WIN32
        handlerPath += ".exe";
#endif

        const base::FilePath database = toBaseFilePath(dumpPath);
        const base::FilePath handler = toBaseFilePath(handlerPath.string());

        databasePtr = crashpad::CrashReportDatabase::Initialize(database);
        if (!databasePtr) {
            qCWarning(lcCrashpad) << "failed to initialize CrashReportDatabase";
            return false;
        }
        if (auto *settings = databasePtr->GetSettings())
            settings->SetUploadsEnabled(crashReportingEnabled);

#ifdef __linux__
        constexpr bool asynchronousStart = false;
#else
        constexpr bool asynchronousStart = true;
#endif

        clientPtr = std::make_unique<crashpad::CrashpadClient>();
        const bool started = clientPtr->StartHandler(
            handler,
            database,
            database,
            reportUrl,
            {},                  // annotations
            {"--no-rate-limit"}, // arguments
            true,                // restartable
            asynchronousStart);
        if (!started) {
            qCWarning(lcCrashpad) << "failed to start crashpad handler at"
                                  << QString::fromStdString(handlerPath.string());
            return false;
        }

        qCInfo(lcCrashpad).noquote()
            << "crashpad ready: dumps ->" << QString::fromStdString(dumpPath)
            << "| reporting:" << (crashReportingEnabled ? "on" : "off");
        return true;
    }

    std::string dumpPath;
    std::string libexecPath;
    std::string reportUrl;
    bool crashReportingEnabled = false;
    bool valid = false;
    std::unique_ptr<crashpad::CrashpadClient> clientPtr;
    std::unique_ptr<crashpad::CrashReportDatabase> databasePtr;
};

Client::Client(
    std::string_view dumpPath,
    std::string_view libexecPath,
    std::string_view reportUrl,
    bool crashReportingEnabled)
    : d_ptr(std::make_unique<ClientPrivate>(dumpPath, libexecPath, reportUrl, crashReportingEnabled))
{}

Client::~Client() = default;

auto Client::isValid() const -> bool
{
    return d_ptr->valid;
}

auto Client::dumpPath() const -> std::string
{
    return d_ptr->dumpPath;
}

auto Client::reportUrl() const -> std::string
{
    return d_ptr->reportUrl;
}

auto Client::isReportingEnabled() const -> bool
{
    return d_ptr->crashReportingEnabled;
}

auto Client::pendingReports() const -> std::vector<ReportInfo>
{
    std::vector<ReportInfo> result;
    if (!d_ptr->databasePtr)
        return result;

    std::vector<crashpad::CrashReportDatabase::Report> reports;
    if (d_ptr->databasePtr->GetPendingReports(&reports) != crashpad::CrashReportDatabase::kNoError) {
        qCWarning(lcCrashpad) << "failed to read pending crash reports";
        return result;
    }

    result.reserve(reports.size());
    for (const auto &report : reports) {
        ReportInfo info;
        info.uuid = report.uuid.ToString();
        info.createdAt = static_cast<std::int64_t>(report.creation_time);
        info.size = report.total_size;
        result.push_back(std::move(info));
    }

    std::ranges::sort(result, {}, &ReportInfo::createdAt);
    return result;
}

void Client::discardReports()
{
    if (!d_ptr->databasePtr)
        return;

    std::vector<crashpad::CrashReportDatabase::Report> reports;
    if (d_ptr->databasePtr->GetPendingReports(&reports) != crashpad::CrashReportDatabase::kNoError) {
        return;
    }

    for (const auto &report : reports) {
        if (d_ptr->databasePtr->SkipReportUpload(
                report.uuid, crashpad::Metrics::CrashSkippedReason::kUploadsDisabled)
            != crashpad::CrashReportDatabase::kNoError) {
            qCWarning(lcCrashpad) << "failed to discard a crash report";
        }
    }
}

} // namespace Crashpad
