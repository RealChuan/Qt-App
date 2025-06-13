#include <3rdparty/qtsingleapplication/qtsingleapplication.h>
#include <dump/crashpad.hpp>
#include <extensionsystem/iplugin.h>
#include <extensionsystem/pluginmanager.h>
#include <extensionsystem/pluginspec.h>
#include <resource/resource.hpp>
#include <utils/algorithm.h>
#include <utils/appdata.hpp>
#include <utils/appinfo.h>
#include <utils/languageconfig.hpp>
#include <utils/logasync.h>
#include <utils/utils.h>
#include <widgets/waitwidget.h>

#include <QNetworkProxyFactory>
#include <QStyle>

void initResource()
{
    Resource r; // 这样才可以使用qrc
#ifndef Q_OS_WIN
    Q_INIT_RESOURCE(resource);
    Q_INIT_RESOURCE(utils);
#endif
}

void setAppInfo()
{
    qApp->setApplicationVersion(Utils::version);
    qApp->setApplicationDisplayName(Utils::appName);
    qApp->setApplicationName(Utils::appName);
    qApp->setDesktopFileName(Utils::appName);
    qApp->setOrganizationDomain(Utils::organizationDomain);
    qApp->setOrganizationName(Utils::organzationName);
    qApp->setWindowIcon(QIcon(":/icon/icon/app.png"));

    const Utils::FilePath appDirPath = Utils::FilePath::fromUserInput(qApp->applicationDirPath());
    Utils::AppInfo info;
    info.author = Utils::author;
    info.copyright = Utils::copyright;
    info.displayVersion = Utils::displayVersion;
    info.id = Utils::id;
    info.plugins = (appDirPath / "plugins").cleanPath();
#ifndef Q_OS_MACOS
    info.resources = (appDirPath / "resources").cleanPath();
#else
    info.resources = (appDirPath / "../Resources").cleanPath();
#endif
    // sync with src\tools\qmlpuppet\qmlpuppet\qmlpuppet.cpp -> QString crashReportsPath()
    info.crashReports = Utils::FilePath::fromString(Utils::crashPath());
    Utils::Internal::setAppInfo(info);
}

void setQss()
{
    Utils::setQSS({":/qss/qss/common.css",
                   ":/qss/qss/mainwidget.css",
                   ":/qss/qss/sidebarbutton.css",
                   ":/qss/qss/specific.css"});
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
    Utils::setHighDpiEnvironmentVariable();
    SharedTools::QtSingleApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
    SharedTools::QtSingleApplication app(Utils::appName, argc, argv);
    if (app.isRunning()) {
        qWarning() << "This is already running";
        if (app.sendMessage("raise_window_noop", 5000)) {
            return EXIT_SUCCESS;
        }
    }
#ifdef Q_OS_WIN
    if (!qFuzzyCompare(app.devicePixelRatio(), 1.0)
        && QApplication::style()->objectName().startsWith(QLatin1String("windows"),
                                                          Qt::CaseInsensitive)) {
        QApplication::setStyle(QLatin1String("fusion"));
    }
#endif
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    app.setAttribute(Qt::AA_UseHighDpiPixmaps);
    app.setAttribute(Qt::AA_DisableWindowContextHelpButton);
#endif

    setAppInfo();

    Dump::CrashPad crashpad(Utils::crashPath(), app.applicationDirPath(), "", true);
    // auto *breakPad = Dump::BreakPad::instance();
    // breakPad->setDumpPath(Utils::crashPath());
    // QObject::connect(breakPad, &Dump::BreakPad::crash, [] { Dump::openCrashReporter(); });

    QDir::setCurrent(app.applicationDirPath());
    Utils::LanguageConfig::instance()->loadLanguage();

    auto *log = Utils::LogAsync::instance();
    log->setLogPath(Utils::logPath());
    log->setAutoDelFile(true);
    log->setAutoDelFileDays(7);
    log->setOrientation(Utils::LogAsync::Orientation::StandardAndFile);
    log->setLogLevel(QtDebugMsg);
    log->startWork();

    initResource();
    qInfo().noquote() << "\n\n" + Utils::systemInfo() + "\n\n";
    Utils::loadFonts(app.applicationDirPath() + "/fonts");
    setQss();

    // Make sure we honor the system's proxy settings
    QNetworkProxyFactory::setUseSystemConfiguration(true);

    // 等待界面
    QScopedPointer<Widgets::WaitWidget> waitWidgetPtr(new Widgets::WaitWidget);
    waitWidgetPtr->show();
    app.processEvents();

    auto *setting = new Utils::QtcSettings(Utils::configFilePath(), QSettings::IniFormat);
    ExtensionSystem::PluginManager pluginManager;
    ExtensionSystem::PluginManager::setSettings(setting);
    ExtensionSystem::PluginManager::setPluginIID(QLatin1String("Youth.Qt.plugin"));
    const QStringList pluginPaths{app.applicationDirPath() + "/plugins"};
    ExtensionSystem::PluginManager::setPluginPaths(
        Utils::transform(pluginPaths, &Utils::FilePath::fromUserInput));
    ExtensionSystem::PluginManager::loadPlugins();

    // Shutdown plugin manager on the exit
    QObject::connect(&app,
                     &SharedTools::QtSingleApplication::aboutToQuit,
                     &pluginManager,
                     &ExtensionSystem::PluginManager::shutdown);

    const auto plugins = ExtensionSystem::PluginManager::plugins();
    ExtensionSystem::PluginSpec *coreSpec = nullptr;
    for (auto *spec : plugins) {
        if (spec->name() == QLatin1String("CorePlugin")) {
            coreSpec = spec;
            break;
        }
    }

    if (coreSpec != nullptr) {
        QObject::connect(&app, &SharedTools::QtSingleApplication::messageReceived, [coreSpec] {
            coreSpec->plugin()->remoteCommand({}, {}, {});
        });
        waitWidgetPtr->fullProgressBar();
        coreSpec->plugin()->remoteCommand({}, {}, {});
        waitWidgetPtr->close();
    } else {
        pluginManager.shutdown();
        app.quit();
        return -1;
    }

    auto exitCode = restarter.restartOrExit(app.exec());
    log->stop();
    return exitCode;
}
