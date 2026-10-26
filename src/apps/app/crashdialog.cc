#include "crashdialog.hpp"

#include <QDateTime>
#include <QDesktopServices>
#include <QEvent>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QPushButton>
#include <QUrl>
#include <QVBoxLayout>

#include <utility>

using namespace Qt::StringLiterals;

namespace Crash {

class CrashDialog::CrashDialogPrivate
{
public:
    CrashDialogPrivate(std::vector<Crashpad::ReportInfo> reports, QString crashPath, QString logPath)
        : reports(std::move(reports))
        , crashPath(std::move(crashPath))
        , logPath(std::move(logPath))
    {}

    void setupUi(CrashDialog *q);
    void retranslateUi();

    std::vector<Crashpad::ReportInfo> reports;
    QString crashPath;
    QString logPath;
    Action action = Action::Dismiss;

    QLabel *iconLabel = nullptr;
    QLabel *titleLabel = nullptr;
    QLabel *metaLabel = nullptr;
    QLabel *bodyLabel = nullptr;
    QLabel *pathLabel = nullptr;
    QPushButton *openButton = nullptr;
    QPushButton *restartButton = nullptr;
    QPushButton *closeButton = nullptr;
};

void CrashDialog::CrashDialogPrivate::setupUi(CrashDialog *q)
{
    iconLabel = new QLabel;
    iconLabel->setFixedSize(44, 44);
    iconLabel->setPixmap(QIcon(":/icon/crash.png").pixmap(44, 44));

    titleLabel = new QLabel;
    titleLabel->setObjectName("CrashTitle");
    metaLabel = new QLabel;
    metaLabel->setObjectName("CrashMeta");

    auto *heading = new QVBoxLayout;
    heading->setContentsMargins({});
    heading->setSpacing(4);
    heading->addWidget(titleLabel);
    heading->addWidget(metaLabel);

    auto *headerRow = new QHBoxLayout;
    headerRow->setContentsMargins({});
    headerRow->setSpacing(16);
    headerRow->addWidget(iconLabel, 0, Qt::AlignTop);
    headerRow->addLayout(heading, 1);

    bodyLabel = new QLabel;
    bodyLabel->setObjectName("CrashBody");
    bodyLabel->setWordWrap(true);

    pathLabel = new QLabel;
    pathLabel->setObjectName("CrashPath");
    pathLabel->setWordWrap(true);
    pathLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);

    openButton = new QPushButton;
    openButton->setObjectName("PrimaryButton");
    openButton->setCursor(Qt::PointingHandCursor);
    restartButton = new QPushButton;
    closeButton = new QPushButton;
    closeButton->setObjectName("GrayButton");

    auto *buttonRow = new QHBoxLayout;
    buttonRow->setContentsMargins({});
    buttonRow->setSpacing(8);
    buttonRow->addWidget(openButton);
    buttonRow->addStretch();
    buttonRow->addWidget(restartButton);
    buttonRow->addWidget(closeButton);

    auto *widget = new QWidget;
    auto *layout = new QVBoxLayout(widget);
    layout->setContentsMargins(28, 24, 28, 22);
    layout->setSpacing(18);
    layout->addLayout(headerRow);
    layout->addWidget(bodyLabel);
    layout->addWidget(pathLabel);
    layout->addStretch();
    layout->addLayout(buttonRow);

    q->setCentralWidget(widget);
}

void CrashDialog::CrashDialogPrivate::retranslateUi()
{
    titleLabel->setText(CrashDialog::tr("The application did not exit normally"));
    bodyLabel->setText(
        CrashDialog::tr(
            "Crash dumps are stored in the folders below. Sending them together "
            "with the logs helps us track down the problem."));
    openButton->setText(CrashDialog::tr("Open folders"));
    restartButton->setText(CrashDialog::tr("Restart"));
    closeButton->setText(CrashDialog::tr("Close"));
    pathLabel->setText(CrashDialog::tr("Dumps: %1\nLogs: %2").arg(crashPath, logPath));

    // 报告按时间升序返回，最后一条即最近一次崩溃。
    const QString time = reports.empty() ? QString()
                                         : QDateTime::fromSecsSinceEpoch(reports.back().createdAt)
                                               .toString(u"yyyy-MM-dd HH:mm"_s);
    const int count = int(reports.size());
    const QString countText = count == 1 ? CrashDialog::tr("1 crash report")
                                         : CrashDialog::tr("%1 crash reports").arg(count);
    metaLabel->setText(CrashDialog::tr("Last crash %1 · %2").arg(time, countText));
}

CrashDialog::CrashDialog(
    std::vector<Crashpad::ReportInfo> reports, QString crashPath, QString logPath, QWidget *parent)
    : Widgets::Dialog(parent)
    , d_ptr(
          std::make_unique<CrashDialogPrivate>(
              std::move(reports), std::move(crashPath), std::move(logPath)))
{
    d_ptr->setupUi(this);
    retranslateUi();
    setupConnections();
    resize(440, 320);
}

CrashDialog::~CrashDialog() = default;

auto CrashDialog::action() const -> Action
{
    return d_ptr->action;
}

void CrashDialog::onOpenFolders()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(d_ptr->crashPath));
    QDesktopServices::openUrl(QUrl::fromLocalFile(d_ptr->logPath));
}

void CrashDialog::onRestartClicked()
{
    d_ptr->action = Action::Restart;
    accept();
}

void CrashDialog::retranslateUi()
{
    setWindowTitle(tr("Crash detected"));
    d_ptr->retranslateUi();
}

void CrashDialog::setupConnections()
{
    connect(d_ptr->openButton, &QPushButton::clicked, this, &CrashDialog::onOpenFolders);
    connect(d_ptr->restartButton, &QPushButton::clicked, this, &CrashDialog::onRestartClicked);
    connect(d_ptr->closeButton, &QPushButton::clicked, this, &CrashDialog::reject);
}

void CrashDialog::changeEvent(QEvent *event)
{
    Widgets::Dialog::changeEvent(event);
    if (event->type() == QEvent::LanguageChange)
        retranslateUi();
}

} // namespace Crash
