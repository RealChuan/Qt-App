#include "crashwidgets.hpp"

#include <3rdparty/qtsingleapplication/qtsingleapplication.h>
#include <dump/breakpad.hpp>
#include <resource/resource.hpp>
#include <utils/appdata.hpp>
#include <utils/asynclog.hpp>
#include <utils/hostosinfo.h>
#include <utils/singletonmanager.hpp>
#include <utils/utils.hpp>

#include <QNetworkProxyFactory>
#include <QStyle>

namespace {

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

    setAppInfo();
    QDir::setCurrent(app.applicationDirPath());

    Dump::Breakpad breakpad(Utils::crashPath().toStdString());

    LANGUAGE_MANAGER->loadLanguage();

    // 日志配置：Debug → 控制台 + 文件 + DEBUG
    //           Release → 文件 + INFO（不写控制台，避免拖慢后端线程、污染用户终端）
    AsyncLog::Config logConfig;
    logConfig.logPath = Utils::logPath();
    logConfig.file = !logConfig.logPath.isEmpty();
#ifdef QT_DEBUG
    logConfig.console = true;
    logConfig.level = QtDebugMsg;
#else
    logConfig.console = false;
    logConfig.level = QtInfoMsg;
#endif
    AsyncLog::Logger::instance()->start(logConfig);

    initResource();
    qInfo().noquote() << "\n\n" + Utils::systemInfo() + "\n\n";
#ifdef Q_OS_MACOS
    Utils::loadFonts(QString("%1/../Resources/fonts").arg(app.applicationDirPath()));
#else
    Utils::loadFonts(QString("%1/resources/fonts").arg(app.applicationDirPath()));
#endif
    setQss();

    // Make sure we honor the system's proxy settings
    QNetworkProxyFactory::setUseSystemConfiguration(true);

    Crash::CrashWidgets w;
    app.setActivationWindow(&w);
    w.show();

    auto result = app.exec();
    AsyncLog::Logger::instance()->shutdown();
    return result;
}
