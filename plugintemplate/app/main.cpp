#include <controls/waitwidget.h>
#include <crashhandler/breakpad.hpp>
#include <extensionsystem/iplugin.h>
#include <extensionsystem/pluginmanager.h>
#include <extensionsystem/pluginspec.h>
#include <useraccountsystem/loginwidget.h>
#include <utils/languageconfig.hpp>
#include <utils/logasync.h>
#include <utils/utils.h>
#include <qtsingleapplication/qtsingleapplication.h>

#include <QDebug>
#include <QDir>
#include <QFont>
#include <QNetworkProxyFactory>
#include <QSettings>
#include <QStyle>

using namespace ExtensionSystem;

void setAppInfo()
{
    qApp->setApplicationVersion("0.0.1");
    qApp->setApplicationDisplayName(QObject::tr("AppPlugin"));
    qApp->setApplicationName(QObject::tr("AppPlugin"));
    qApp->setDesktopFileName(QObject::tr("AppPlugin"));
    qApp->setOrganizationDomain(QObject::tr("Youth"));
    qApp->setOrganizationName(QObject::tr("Youth"));
}

int main(int argc, char *argv[])
{
#if defined(Q_OS_WIN) && QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    if (!qEnvironmentVariableIsSet("QT_OPENGL"))
        QCoreApplication::setAttribute(Qt::AA_UseOpenGLES);
#else
    qputenv("QSG_RHI_BACKEND", "opengl");
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(
        Qt::HighDpiScaleFactorRoundingPolicy::Round);
#endif
    Utils::setHighDpiEnvironmentVariable();
    SharedTools::QtSingleApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
    SharedTools::QtSingleApplication app("Plugin-Template", argc, argv);
    if (app.isRunning()) {
        qWarning() << "This is already running";
        if (app.sendMessage("raise_window_noop", 5000)) {
            return EXIT_SUCCESS;
        }
    }

#ifdef Q_OS_WIN
    if (!qFuzzyCompare(qApp->devicePixelRatio(), 1.0)
        && QApplication::style()->objectName().startsWith(QLatin1String("windows"),
                                                          Qt::CaseInsensitive)) {
        QApplication::setStyle(QLatin1String("fusion"));
    }
#endif
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    app.setAttribute(Qt::AA_UseHighDpiPixmaps);
    app.setAttribute(Qt::AA_DisableWindowContextHelpButton);
#endif

    Utils::BreakPad breakPad;
    QDir::setCurrent(app.applicationDirPath());
    Utils::LanguageConfig::instance()->loadLanguage();

    // 异步日志
    Utils::LogAsync *log = Utils::LogAsync::instance();
    log->setOrientation(Utils::LogAsync::Orientation::StdAndFile);
    log->setLogLevel(QtDebugMsg);
    log->startWork();

    Utils::printBuildInfo();
    //Utils::setUTF8Code();
    Utils::loadFonts();
    Utils::setQSS("plugintemplate_qss_files");
    Utils::setGlobalThreadPoolMaxSize();

    // Make sure we honor the system's proxy settings
    QNetworkProxyFactory::setUseSystemConfiguration(true);

    // 等待界面
    Control::WaitWidget waitWidget;
    waitWidget.show();
    app.processEvents();

    setAppInfo();

    QSettings *setting = new QSettings(Utils::getConfigPath() + "/config/config.ini",
                                       QSettings::IniFormat);
    PluginManager pluginManager;

    PluginManager::setSettings(setting);
    PluginManager::setPluginIID(QLatin1String("Youth.Qt.plugin"));
    const QStringList pluginPaths{app.applicationDirPath() + "/plugins",
                                  app.applicationDirPath() + "/test"};
    PluginManager::setPluginPaths(pluginPaths);
    PluginManager::loadPlugins();

    // Shutdown plugin manager on the exit
    QObject::connect(&app, SIGNAL(aboutToQuit()), &pluginManager, SLOT(shutdown()));

    const QVector<PluginSpec *> plugins = PluginManager::plugins();
    PluginSpec *coreSpec = nullptr;
    for (PluginSpec *spec : plugins) {
        if (spec->name() == QLatin1String("CorePlugin")) {
            coreSpec = spec;
            break;
        }
    }

    if (coreSpec) {
        QObject::connect(&app, &SharedTools::QtSingleApplication::messageReceived, [=] {
            coreSpec->plugin()->remoteCommand(QStringList(), QString(), QStringList());
        });
        waitWidget.fullProgressBar();
        coreSpec->plugin()->remoteCommand(QStringList(), QString(), QStringList());
        waitWidget.close();
    } else {
        pluginManager.shutdown();
        app.quit();
        return -1;
    }

    int result = app.exec();
    log->stop();
    return result;
}
