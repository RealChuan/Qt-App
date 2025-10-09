#include <3rdparty/qtsingleapplication/qtsingleapplication.h>
#include <dump/crashpad.hpp>
#include <extensionsystem/iplugin.h>
#include <extensionsystem/pluginmanager.h>
#include <extensionsystem/pluginspec.h>
#include <resource/resource.hpp>
#include <utils/algorithm.h>
#include <utils/appdata.hpp>
#include <utils/appinfo.h>
#include <utils/hostosinfo.h>
#include <utils/logasync.h>
#include <utils/singletonmanager.hpp>
#include <utils/utils.hpp>
#include <widgets/waitwidget.h>

#include <QMessageBox>
#include <QNetworkProxyFactory>
#include <QStyle>

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
#ifdef Q_OS_MACOS
    info.plugins = (appDirPath / "../PlugIns" / QString(Utils::appName).toLower()).cleanPath();
    info.resources = (appDirPath / "../Resources").cleanPath();
#else
    info.plugins = (appDirPath / "plugins" / QString(Utils::appName).toLower()).cleanPath();
    info.resources = (appDirPath / "resources").cleanPath();
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
    LANGUAGE_MANAGER->loadLanguage();

    auto *log = Utils::LogAsync::instance();
    log->setLogPath(Utils::logPath());
    log->setAutoDelFile(true);
    log->setAutoDelFileDays(7);
    log->setOrientation(Utils::LogAsync::Orientation::StandardAndFile);
    log->setLogLevel(QtDebugMsg);
    log->startWork();

    initResource();
    qInfo().noquote() << "\n\n" + Utils::systemInfo() + "\n\n";
    Utils::setPixmapCacheLimit();
#ifdef Q_OS_MACOS
    Utils::loadFonts(QString("%1/../Resources/fonts").arg(app.applicationDirPath()));
#else
    Utils::loadFonts(QString("%1/fonts").arg(app.applicationDirPath()));
#endif
    setQss();

    // Make sure we honor the system's proxy settings
    QNetworkProxyFactory::setUseSystemConfiguration(true);

    // 等待界面
    QScopedPointer<Widgets::WaitWidget> waitWidgetPtr(new Widgets::WaitWidget);
    waitWidgetPtr->show();
    app.processEvents();

    auto *setting = new Utils::QtcSettings(Utils::configFilePath(), QSettings::IniFormat);
    auto *installSettings = new Utils::QtcSettings(QSettings::IniFormat,
                                                   QSettings::SystemScope,
                                                   QLatin1String(Utils::organzationName),
                                                   QLatin1String(Utils::appName));
    ExtensionSystem::PluginManager pluginManager;
    ExtensionSystem::PluginManager::setPluginIID(QLatin1String("Youth.Qt.plugin"));
    ExtensionSystem::PluginManager::setInstallSettings(installSettings);
    ExtensionSystem::PluginManager::setSettings(setting);
    ExtensionSystem::PluginManager::startProfiling();
    ExtensionSystem::PluginManager::removePluginsAfterRestart();
    // We need to install plugins before we scan for them.
    ExtensionSystem::PluginManager::installPluginsAfterRestart();

    ExtensionSystem::PluginManager::setPluginPaths({Utils::appInfo().plugins});

    // Shutdown plugin manager on the exit
    QObject::connect(&app,
                     &SharedTools::QtSingleApplication::aboutToQuit,
                     &pluginManager,
                     &ExtensionSystem::PluginManager::shutdown);

    const auto plugins = ExtensionSystem::PluginManager::plugins();

    auto *coreplugin = ExtensionSystem::PluginManager::specById(QLatin1String("CorePlugin"));
    if (!coreplugin) {
        const QString pluginPath
            = ExtensionSystem::PluginManager::pluginPaths().first().toUserOutput();
        QString nativePaths = QDir::toNativeSeparators(pluginPath);
        const QString reason = QCoreApplication::translate("Application",
                                                           "Could not find Core plugin in %1")
                                   .arg(nativePaths);
        displayError(msgCoreLoadFailure(reason));
        return 1;
    }
    if (!coreplugin->isEffectivelyEnabled()) {
        const QString reason = QCoreApplication::translate("Application",
                                                           "Core plugin is disabled.");
        displayError(msgCoreLoadFailure(reason));
        return 1;
    }
    if (coreplugin->hasError()) {
        displayError(msgCoreLoadFailure(coreplugin->errorString()));
        return 1;
    }

    ExtensionSystem::PluginManager::checkForProblematicPlugins();
    ExtensionSystem::PluginManager::loadPlugins();

    if (coreplugin->hasError()) {
        displayError(msgCoreLoadFailure(coreplugin->errorString()));
        return 1;
    }

    // Set up remote arguments.
    QObject::connect(&app,
                     &SharedTools::QtSingleApplication::messageReceived,
                     &pluginManager,
                     &ExtensionSystem::PluginManager::remoteArguments);

    QObject::connect(&app,
                     &SharedTools::QtSingleApplication::fileOpenRequest,
                     coreplugin->plugin(),
                     [coreplugin](const QString &file) {
                         coreplugin->plugin()->remoteCommand({}, {}, {file});
                     });

    // shutdown plugin manager on the exit
    QObject::connect(&app,
                     &QCoreApplication::aboutToQuit,
                     &pluginManager,
                     &ExtensionSystem::PluginManager::shutdown);

    waitWidgetPtr->fullProgressBar();
    app.processEvents();
    waitWidgetPtr->close();

    auto exitCode = restarter.restartOrExit(app.exec());
    log->stop();
    return exitCode;
}
