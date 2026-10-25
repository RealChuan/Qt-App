#include "crashdialog.hpp"

#include <3rdparty/qtsingleapplication/qtsingleapplication.hpp>
#include <crashpadclient/crashpad.hpp>
#include <extensionsystem/iplugin.hpp>
#include <extensionsystem/pluginmanager.hpp>
#include <resource/resource.hpp>
#include <shell/host.hpp>
#include <utils/algorithm.hpp>
#include <utils/appinfo.hpp>
#include <utils/asynclog.hpp>
#include <utils/hostosinfo.hpp>
#include <utils/settings.hpp>
#include <utils/singletonmanager.hpp>
#include <utils/theme.hpp>
#include <utils/uihelpers.hpp>
#include <utils/utils.hpp>
#include <widgets/startupsplash.hpp>

#include <QCryptographicHash>
#include <QMessageBox>
#include <QNetworkProxyFactory>
#include <QProcess>
#include <QRunnable>
#include <QStyle>
#include <QThreadPool>

namespace {

constexpr auto kLogLevelPrefix = "--log-level=";

// 把 --log-level=<name> 的值映射成 QtMsgType；无法识别时返回 fallback
QtMsgType parseLevel(const QString &name, QtMsgType fallback)
{
    const auto v = name.toLower();
    if (v == QLatin1String("debug"))
        return QtDebugMsg;
    if (v == QLatin1String("info"))
        return QtInfoMsg;
    if (v == QLatin1String("warning"))
        return QtWarningMsg;
    if (v == QLatin1String("critical"))
        return QtCriticalMsg;
    if (v == QLatin1String("fatal"))
        return QtFatalMsg;
    return fallback;
}

// 根据构建类型生成日志配置并启动 AsyncLog::Logger 单例。
//   Debug   构建 → 控制台开、级别 DEBUG
//   Release 构建 → 控制台关、级别 INFO
// 运行期可覆盖（现场排查用）：
//   --console                                     强制打开控制台输出
//   --log-level=debug|info|warning|critical|fatal 覆盖级别
//
// 要求 QApplication/QCoreApplication 已构造（内部读 QCoreApplication::arguments）。
void startLogger()
{
    AsyncLog::Config config;
    config.logPath = Utils::AppInfo::logPath().path();
    config.file = !config.logPath.isEmpty();

#ifdef QT_DEBUG
    config.console = true;
    config.level = QtDebugMsg;
#else
    config.console = false;
    config.level = QtInfoMsg;
#endif

    // 手写扫描命令行，避免与 QtSingleApplication / 主程序自身的参数解析冲突
    const auto args = QCoreApplication::arguments();
    for (int i = 1; i < args.size(); ++i) {
        const auto &arg = args[i];
        if (arg == QLatin1String("--console")) {
            config.console = true;
        } else if (arg.startsWith(QLatin1String(kLogLevelPrefix))) {
            config.level = parseLevel(arg.mid(QLatin1String(kLogLevelPrefix).size()), config.level);
        }
    }

    AsyncLog::Logger::instance()->start(config);
}

// 机器体检：单线程 MD5 吞吐。放在全局线程池里跑，不挡启动，结束后只写一条日志。
constexpr int kHashBenchIterations = 3;
constexpr int kHashBenchDurationMs = 500;
constexpr int kHashBenchDataSize = 1024 * 1024;

class HashBenchTask final : public QRunnable
{
public:
    void run() override
    {
        const double mibPerSecond = Utils::hashBench(
            kHashBenchIterations, kHashBenchDurationMs, kHashBenchDataSize, QCryptographicHash::Md5);
        qInfo().noquote() << QStringLiteral("MD5 throughput: %1 MiB/s (single thread, 1 MiB blocks)")
                                 .arg(mibPerSecond, 0, 'f', 1);
    }
};

QString coreLoadFailureMessage(const QString &why)
{
    return QCoreApplication::translate("Application", "Failed to load core: %1").arg(why);
}

// 启动失败时故意用原生消息框：此刻出问题的可能正是外壳/插件这一层，
// 不该再依赖自绘窗口那套东西来报错。
void displayError(const QString &t)
{
    if (Utils::HostOsInfo::isWindowsHost() && qApp) {
        QMessageBox::critical(nullptr, QLatin1String(Utils::AppInfo::appName), t);
    } else {
        qCritical("%s", qPrintable(t));
    }
}

// 启动中途中止：日志必须落盘（丢掉的往往正是失败原因），设置也按正常流程销毁。
// 这条路径上 app.exec() 还没跑，aboutToQuit 不会触发，只能手动收尾。
int abortStartup(int exitCode)
{
    AsyncLog::Logger::instance()->shutdown();
    Utils::destroySettings();
    return exitCode;
}

// 把编译期身份写进 QCoreApplication（设置目录、窗口标题、收件箱都依赖它）。
void setupApplication()
{
    qApp->setApplicationVersion(Utils::AppInfo::version);
    qApp->setApplicationDisplayName(Utils::AppInfo::appName);
    qApp->setApplicationName(Utils::AppInfo::appName);
    qApp->setDesktopFileName(Utils::AppInfo::appName);
    qApp->setOrganizationDomain(Utils::AppInfo::organizationDomain);
    qApp->setOrganizationName(Utils::AppInfo::organizationName);
    qApp->setWindowIcon(QIcon(":/icon/app.png"));
}

class Restarter
{
public:
    // 启动目录要在这里抓住：main() 稍后会把 CWD 切到程序目录。
    Restarter()
        : m_workingPath(QDir::currentPath())
    {}

    // 参数必须在 QApplication 构造之后再取：Qt 会整理引号、编码这些细节。
    void setArguments(const QStringList &args) { m_args = args; }

    // 调用方必须先收好日志：新进程一起来就会开自己的日志文件，
    // 老进程还在缓冲里的行会晚一步落盘，排查时两边的时序会对不上。
    void restartIfRequested()
    {
        if (Utils::restartRequested())
            QProcess::startDetached(QCoreApplication::applicationFilePath(), m_args, m_workingPath);
    }

private:
    QStringList m_args;
    QString m_workingPath;
};

} // namespace

void initResource()
{
    Resource r; // 这样才可以使用qrc
#ifndef Q_OS_WIN
    Q_INIT_RESOURCE(resource);
#endif
}

auto main(int argc, char *argv[]) -> int
{
    Restarter restarter;
    qputenv("QSG_RHI_BACKEND", "opengl");
    Utils::setupHighDpiRounding();
    SharedTools::QtSingleApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
    SharedTools::QtSingleApplication app(Utils::AppInfo::appName, argc, argv);
    // Qt 会整理引号与编码，重启时用它解析过的参数比原始的 argv 更可靠。
    restarter.setArguments(QCoreApplication::arguments().mid(1));
    if (app.isRunning()) {
        qWarning() << "This is already running";
        // 已有实例收不到消息（卡死或正在退出）时继续启动，宁可出现两个窗口，
        // 也不要让用户点了图标却什么都没有。
        if (app.sendMessage("raise_window_noop", 5000)) {
            return EXIT_SUCCESS;
        }
    }
    setupApplication();
    QDir::setCurrent(app.applicationDirPath());

    Crashpad::Client crashpadClient(
        Utils::AppInfo::crashPath().path().toStdString(),
        app.applicationDirPath().toStdString(),
        {},
        true);

    Utils::languageManager()->loadLanguage();

    // 启动日志：Debug → 控制台 + 文件 + DEBUG
    //          Release → 文件 + INFO（--console 可临时开控制台）
    startLogger();

    initResource();
    qInfo().noquote() << "\n\n" + Utils::systemInfo() + "\n\n";
    Utils::scalePixmapCacheForDpr();
    Utils::loadFonts((Utils::AppInfo::resources() / "fonts").toUserOutput());
    // 样式表按当前深浅色方案生成；系统切换时 Theme 会自己重新应用。
    Utils::theme()->apply();

    // 上次异常退出的痕迹：crashpad 已经把 dmp 落盘，这里在插件加载前拦一次。
    // 放在插件之前是因为插件初始化阶段崩溃时，只有挡在启动流程里才能看到提示。
    if (const auto reports = crashpadClient.pendingReports(); !reports.empty()) {
        qInfo() << "pending crash reports:" << reports.size();
        Crash::CrashDialog
            dialog(reports, Utils::AppInfo::crashPath().path(), Utils::AppInfo::logPath().path());
        dialog.exec();
        // 先标记再重启：否则重启后的新进程会立刻再弹一次。
        crashpadClient.discardReports();
        if (dialog.action() == Crash::CrashDialog::Action::Restart) {
            Utils::requestRestart();
            AsyncLog::Logger::instance()->shutdown();
            restarter.restartIfRequested();
            return 0;
        }
    }

    // Make sure we honor the system's proxy settings
    QNetworkProxyFactory::setUseSystemConfiguration(true);

    // 启动画面：插件加载期间主线程被占住，这里只负责别让用户对着空桌面。
    auto splash = std::make_unique<Widgets::StartupSplash>();
    splash->show();
    app.processEvents();

    // 与插件加载并行跑，结果只进日志；QRunnable 交给线程池（autoDelete）接管。
    QThreadPool::globalInstance()->start(new HashBenchTask);

    auto *userSettings
        = new QSettings(Utils::AppInfo::configFilePath().path(), QSettings::IniFormat);
    auto *installSettings = new QSettings(
        QSettings::IniFormat,
        QSettings::SystemScope,
        QLatin1String(Utils::AppInfo::organizationName),
        QLatin1String(Utils::AppInfo::appName));
    Utils::setupSettings(userSettings, installSettings);

    ExtensionSystem::PluginManager pluginManager;
    ExtensionSystem::PluginManager::setPluginPaths({Utils::AppInfo::plugins()});

    if (const auto result = ExtensionSystem::PluginManager::loadPlugins(); !result) {
        displayError(coreLoadFailureMessage(result.error()));
        return abortStartup(1);
    }

    // 外壳插件缺失或被禁用时没有主窗口，直接报错比留一个空界面更好。
    if (!Shell::host()) {
        displayError(
            QCoreApplication::translate("Application", "Shell plugin (coreplugin) is not available."));
        return abortStartup(1);
    }

    // 单实例：第二个进程把已有窗口拉到前台。
    QObject::connect(&app, &SharedTools::QtSingleApplication::messageReceived, &pluginManager, [] {
        if (Shell::host())
            Utils::restoreAndActivate(Shell::host()->mainWindow());
    });

    // shutdown plugin manager on the exit
    QObject::connect(&app, &QCoreApplication::aboutToQuit, &pluginManager, [] {
        ExtensionSystem::PluginManager::shutdown();
        Utils::destroySettings();
    });

    splash->close();

    // 主窗口不再在自己构造时抢着显示：等等待界面收掉再露出来，避免两个窗叠在一起。
    if (QWidget *mainWindow = Shell::host()->mainWindow())
        mainWindow->show();

    if (Utils::HostOsInfo::isWindowsHost()) {
        // Workaround for QTBUG-130696 and QTCREATORBUG-31890
        QApplication::setEffectEnabled(Qt::UI_FadeMenu, false);

        // Disable menu animation which just looks bad
        QApplication::setEffectEnabled(Qt::UI_AnimateMenu, false);
    }

    const int exitCode = app.exec();
    // 先排空日志，再拉起新进程：两条退出路径都保持这个顺序。
    AsyncLog::Logger::instance()->shutdown();
    restarter.restartIfRequested();
    return exitCode;
}
