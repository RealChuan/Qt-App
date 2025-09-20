#include "crashwidgets.hpp"

#include <3rdparty/qtsingleapplication/qtsingleapplication.h>
#include <dump/breakpad.hpp>
#include <resource/resource.hpp>
#include <utils/appdata.hpp>
#include <utils/logasync.h>
#include <utils/singletonmanager.hpp>
#include <utils/utils.h>

#include <QNetworkProxyFactory>
#include <QStyle>

void initResource()
{
    Resource r; // 这样才可以使用qrc
#ifndef Q_OS_WIN
    Q_INIT_RESOURCE(resource);
#endif
}

void setAppInfo()
{
    qApp->setApplicationVersion(Utils::version);
    qApp->setApplicationDisplayName(Utils::crashName);
    qApp->setApplicationName(Utils::crashName);
    qApp->setDesktopFileName(Utils::crashName);
    qApp->setOrganizationDomain(Utils::organizationDomain);
    qApp->setOrganizationName(Utils::organzationName);
    qApp->setWindowIcon(QIcon(":/icon/icon/crash.png"));
}

void setQss()
{
    Utils::setQSS({":/qss/qss/common.css",
                   ":/qss/qss/mainwidget.css",
                   ":/qss/qss/sidebarbutton.css",
                   ":/qss/qss/specific.css"});
}

auto main(int argc, char *argv[]) -> int
{
#if defined(Q_OS_WIN) && QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    if (!qEnvironmentVariableIsSet("QT_OPENGL")) {
        QCoreApplication::setAttribute(Qt::AA_UseOpenGLES);
    }
#else
    qputenv("QSG_RHI_BACKEND", "opengl");
#endif
    Utils::setHighDpiEnvironmentVariable();
    SharedTools::QtSingleApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
    SharedTools::QtSingleApplication app(Utils::crashName, argc, argv);
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
    Dump::BreakPad::instance()->setDumpPath(Utils::crashPath());
    QDir::setCurrent(app.applicationDirPath());
    LANGUAGE_MANAGER->loadLanguage();

    // 异步日志
    auto *log = Utils::LogAsync::instance();
    log->setLogPath(Utils::logPath());
    log->setAutoDelFile(true);
    log->setAutoDelFileDays(7);
    log->setOrientation(Utils::LogAsync::Orientation::StandardAndFile);
    log->setLogLevel(QtDebugMsg);
    log->startWork();

    initResource();
    qInfo().noquote() << "\n\n" + Utils::systemInfo() + "\n\n";
#ifdef Q_OS_MACOS
    Utils::loadFonts(QString("%1/../Resources/fonts").arg(app.applicationDirPath()));
#else
    Utils::loadFonts(QString("%1/fonts").arg(app.applicationDirPath()));
#endif
    setQss();

    // Make sure we honor the system's proxy settings
    QNetworkProxyFactory::setUseSystemConfiguration(true);

    Crash::CrashWidgets w;
    app.setActivationWindow(&w);
    w.show();

    auto result = app.exec();
    log->stop();
    return result;
}
