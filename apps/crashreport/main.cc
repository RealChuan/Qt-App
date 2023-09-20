#include "crashwidgets.hpp"

#include <3rdparty/breakpad.hpp>
#include <3rdparty/qtsingleapplication/qtsingleapplication.h>
#include <resource/resource.hpp>
#include <utils/languageconfig.hpp>
#include <utils/logasync.h>
#include <utils/utils.h>

#include <QNetworkProxyFactory>
#include <QStyle>

#define AppName "CrashReport"

void initResource()
{
    Resource r; // 这样才可以使用qrc
#ifndef Q_OS_WIN
    Q_INIT_RESOURCE(resource);
#endif
}

void setAppInfo()
{
    qApp->setApplicationVersion("0.0.1");
    qApp->setApplicationDisplayName(AppName);
    qApp->setApplicationName(AppName);
    qApp->setDesktopFileName(AppName);
    qApp->setOrganizationDomain("Youth");
    qApp->setOrganizationName("Youth");
    qApp->setWindowIcon(QIcon(":/icon/icon/crash.png"));
}

void setQss()
{
    Utils::setQSS(
        {":/qss/qss/common.css", ":/qss/qss/commonwidget.css", ":/qss/qss/carshdialog.css"});
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
    SharedTools::QtSingleApplication app(AppName, argc, argv);
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
    Utils::BreakPad::instance();
    QDir::setCurrent(app.applicationDirPath());
    Utils::LanguageConfig::instance()->loadLanguage();

    // 异步日志
    auto log = Utils::LogAsync::instance();
    log->setOrientation(Utils::LogAsync::Orientation::StdAndFile);
    log->setLogLevel(QtDebugMsg);
    log->startWork();

    initResource();
    Utils::printBuildInfo();
    //Utils::setUTF8Code();
    Utils::setGlobalThreadPoolMaxSize();
    Utils::loadFonts(app.applicationDirPath() + "/fonts");
    setQss();

    // Make sure we honor the system's proxy settings
    QNetworkProxyFactory::setUseSystemConfiguration(true);

    setAppInfo();

    Crash::CrashWidgets w;
    w.show();

    auto result = app.exec();
    log->stop();
    return result;
}
