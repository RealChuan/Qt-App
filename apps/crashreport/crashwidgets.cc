#include "crashwidgets.hpp"

#include <3rdparty/breakpad.hpp>
#include <utils/utils.h>

#include <QtWidgets>

namespace Crash {

void openDir(const QString &path)
{
    QDir dir;
    if (dir.exists(path)) {
        qInfo() << QDesktopServices::openUrl(QUrl(path, QUrl::TolerantMode));
    }
}

class CrashWidgets::CrashWidgetsPrivate
{
public:
    CrashWidgetsPrivate(CrashWidgets *q)
        : q_ptr(q)
    {
        auto appArgs = qApp->arguments();
        Q_ASSERT(appArgs.size() > 4);
        crashPath = appArgs.at(1);
        logPath = appArgs.at(2);
        appPath = appArgs.at(3);
        args = {appArgs.begin() + 4, appArgs.end()};
    }

    CrashWidgets *q_ptr;

    QString crashPath;
    QString logPath;
    QString appPath;
    QStringList args;
};

CrashWidgets::CrashWidgets(QWidget *parent)
    : GUI::CommonWidget(parent)
    , d_ptr(new CrashWidgetsPrivate(this))
{
    setupUI();
    resize(450, 450);
}

CrashWidgets::~CrashWidgets() {}

void CrashWidgets::onOpenCrashPath()
{
    openDir(d_ptr->crashPath);
    openDir(d_ptr->logPath);
}

void CrashWidgets::onRestart()
{
    QProcess::startDetached(d_ptr->appPath, d_ptr->args);
}

void CrashWidgets::onQuit()
{
    QMetaObject::invokeMethod(qApp, &QApplication::quit, Qt::QueuedConnection);
}

void CrashWidgets::setupUI()
{
    auto crashButton = new QPushButton(tr("Path of Crash File"), this);
    auto restartButton = new QPushButton(tr("Restart"), this);
    auto closeButton = new QPushButton(tr("Close"), this);
    crashButton->setObjectName("BlueButton");
    restartButton->setObjectName("BlueButton");
    closeButton->setObjectName("BlueButton");
    connect(crashButton, &QPushButton::clicked, this, &CrashWidgets::onOpenCrashPath);
    connect(restartButton, &QPushButton::clicked, this, &CrashWidgets::onRestart);
    connect(closeButton, &QPushButton::clicked, this, &CrashWidgets::onQuit);

    auto crashLabel = new QLabel(this);
    crashLabel->setObjectName("CrashLabel");
    crashLabel->setWordWrap(true);
    crashLabel->setAlignment(Qt::AlignCenter);
    crashLabel->setText(tr("Sorry, the application crashed abnormally. \n"
                           "please click [Path of Crash File] \n"
                           "and send us all the files in this path. \n"
                           "This can help us fix bugs. \n"
                           "Thank you! \n"
                           "Contact Me - Email: \n"
                           "1070753498@qq.com"));

    auto widget = new QWidget(this);
    auto layout = new QVBoxLayout(widget);
    layout->addStretch();
    layout->addWidget(crashLabel);
    layout->addStretch();
    layout->addWidget(crashButton);
    layout->addWidget(restartButton);
    layout->addWidget(closeButton);
    setCentralWidget(widget);
}

} // namespace Crash
