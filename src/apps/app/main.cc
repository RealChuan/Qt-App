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
#include <utils/utils.hpp>
#include <widgets/waitwidget.hpp>

#include <QCryptographicHash>
#include <QMessageBox>
#include <QNetworkProxyFactory>
#include <QRunnable>
#include <QStyle>
#include <QThreadPool>

namespace {

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
    config.logPath = Utils::logPath();
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
        } else if (arg.startsWith(QLatin1String("--log-level="))) {
            config.level = parseLevel(arg.mid(12), config.level); // strlen("--log-level=") == 12
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

static inline QString msgCoreLoadFailure(const QString &why)
{
    return QCoreApplication::translate("Application", "Failed to load core: %1").arg(why);
}

static void displayError(const QString &t)
{
    if (Utils::HostOsInfo::isWindowsHost() && qApp) {
        QMessageBox::critical(nullptr, QLatin1String(Utils::appName), t);
    } else {
        qCritical("%s", qPrintable(t));
    }
}

// 把编译期身份写进 QCoreApplication（设置目录、窗口标题、收件箱都依赖它）。
void setupApplication()
{
    qApp->setApplicationVersion(Utils::version);
    qApp->setApplicationDisplayName(Utils::appName);
    qApp->setApplicationName(Utils::appName);
    qApp->setDesktopFileName(Utils::appName);
    qApp->setOrganizationDomain(Utils::organizationDomain);
    qApp->setOrganizationName(Utils::organizationName);
    qApp->setWindowIcon(QIcon(":/icon/icon/app.png"));
}

class Restarter
{
public:
    Restarter(int argc, char *argv[])
    {
        Q_UNUSED(argc)
        m_executable = QString::fromLocal8Bit(argv[0]);
        m_workingPath = QDir::currentPath();
    }

    void setArguments(const QStringList &args) { m_args = args; }

    QString executable() const { return m_executable; }
    QStringList arguments() const { return m_args; }
    QString workingPath() const { return m_workingPath; }

    int restartOrExit(int exitCode)
    {
        return qApp->property("restart").toBool() ? restart(exitCode) : exitCode;
    }

    int restart(int exitCode)
    {
        QProcess::startDetached(m_executable, m_args, m_workingPath);
        return exitCode;
    }

private:
    QString m_executable;
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
    Restarter restarter(argc, argv);
#if defined(Q_OS_WIN) && QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    if (!qEnvironmentVariableIsSet("QT_OPENGL")) {
        QCoreApplication::setAttribute(Qt::AA_UseOpenGLES);
    }
#else
    qputenv("QSG_RHI_BACKEND", "opengl");
#endif
    Utils::setupHighDpi();
    SharedTools::QtSingleApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
    SharedTools::QtSingleApplication app(Utils::appName, argc, argv);
    if (app.isRunning()) {
        qWarning() << "This is already running";
        if (app.sendMessage("raise_window_noop", 5000)) {
            return EXIT_SUCCESS;
        }
    }
    if (Utils::HostOsInfo::isWindowsHost()) {
        // The Windows 11 default style (Qt 6.7) has major issues, therefore
        // set the previous default style: "windowsvista"
        // FIXME: check newer Qt Versions
        QApplication::setStyle(QLatin1String("windowsvista"));

        // On scaling different than 100% or 200% use the "fusion" style
        qreal tmp;
        const bool fractionalDpi = !qFuzzyIsNull(std::modf(qApp->devicePixelRatio(), &tmp));
        if (fractionalDpi) {
            QApplication::setStyle(QLatin1String("fusion"));
        }
    }
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    app.setAttribute(Qt::AA_UseHighDpiPixmaps);
    app.setAttribute(Qt::AA_DisableWindowContextHelpButton);
#endif

    setupApplication();
    QDir::setCurrent(app.applicationDirPath());

    Crashpad::Client crashpadClient(
        Utils::crashPath().toStdString(), app.applicationDirPath().toStdString(), {}, true);

    Utils::languageManager()->loadLanguage();

    // 启动日志：Debug → 控制台 + 文件 + DEBUG
    //          Release → 文件 + INFO（--console 可临时开控制台）
    startLogger();

    initResource();
    qInfo().noquote() << "\n\n" + Utils::systemInfo() + "\n\n";
    Utils::setPixmapCacheLimit();
    Utils::loadFonts((Utils::appInfo().resources / "fonts").toUserOutput());
    // 样式表按当前深浅色方案生成；系统切换时 Theme 会自己重新应用。
    Utils::theme()->apply();

    // 上次异常退出的痕迹：crashpad 已经把 dmp 落盘，这里在插件加载前拦一次。
    // 放在插件之前是因为插件初始化阶段崩溃时，只有挡在启动流程里才能看到提示。
    if (const auto reports = crashpadClient.pendingReports(); !reports.empty()) {
        qInfo() << "pending crash reports:" << reports.size();
        Crash::CrashDialog dialog(reports, Utils::crashPath(), Utils::logPath());
        dialog.exec();
        // 先标记再重启：否则重启后的新进程会立刻再弹一次。
        crashpadClient.discardReports();
        if (dialog.action() == Crash::CrashDialog::Action::Restart) {
            app.setProperty("restart", true);
            AsyncLog::Logger::instance()->shutdown();
            return restarter.restartOrExit(0);
        }
    }

    // Make sure we honor the system's proxy settings
    QNetworkProxyFactory::setUseSystemConfiguration(true);

    // 等待界面
    auto waitWidget = std::make_unique<Widgets::WaitWidget>();
    waitWidget->show();
    app.processEvents();

    // 与插件加载并行跑，结果只进日志；QRunnable 交给线程池（autoDelete）接管。
    QThreadPool::globalInstance()->start(new HashBenchTask);

    auto *userSettings = new QSettings(Utils::configFilePath(), QSettings::IniFormat);
    auto *installSettings = new QSettings(
        QSettings::IniFormat,
        QSettings::SystemScope,
        QLatin1String(Utils::organizationName),
        QLatin1String(Utils::appName));
    Utils::setupSettings(userSettings, installSettings);

    ExtensionSystem::PluginManager pluginManager;
    ExtensionSystem::PluginManager::setPluginPaths({Utils::appInfo().plugins});

    if (const auto result = ExtensionSystem::PluginManager::loadPlugins(); !result) {
        displayError(msgCoreLoadFailure(result.error()));
        return 1;
    }

    // 外壳插件缺失或被禁用时没有主窗口，直接报错比留一个空界面更好。
    if (!Shell::host()) {
        displayError(
            QCoreApplication::translate("Application", "Shell plugin (coreplugin) is not available."));
        return 1;
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

    waitWidget->fullProgressBar();
    app.processEvents();
    waitWidget->close();

    // 主窗口不再在自己构造时抢着显示：等等待界面收掉再露出来，避免两个窗叠在一起。
    if (QWidget *mainWindow = Shell::host()->mainWindow())
        mainWindow->show();

    if (Utils::HostOsInfo::isWindowsHost()) {
        // Workaround for QTBUG-130696 and QTCREATORBUG-31890
        QApplication::setEffectEnabled(Qt::UI_FadeMenu, false);

        // Disable menu animation which just looks bad
        QApplication::setEffectEnabled(Qt::UI_AnimateMenu, false);
    }

    auto exitCode = restarter.restartOrExit(app.exec());
    AsyncLog::Logger::instance()->shutdown();
    return exitCode;
}
