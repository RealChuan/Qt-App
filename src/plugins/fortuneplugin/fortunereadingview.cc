#include "fortunereadingview.hpp"

#include "fortunecontent.hpp"
#include "fortunesharing.hpp"
#include "hexagramboard.hpp"

#include <shell/host.hpp>
#include <utils/singletonmanager.hpp>
#include <utils/theme.hpp>

#include <QClipboard>
#include <QDir>
#include <QEvent>
#include <QFileDialog>
#include <QFrame>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QHideEvent>
#include <QLabel>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QScrollArea>
#include <QSizePolicy>
#include <QStandardPaths>
#include <QVBoxLayout>

namespace Fortune {

namespace {

constexpr int kBoardWidth = 220;
constexpr int kBoardHeight = 240;
constexpr int kInfoMinWidth = 280;
constexpr int kRevealDurationMs = 1200;

} // namespace

class FortuneReadingView::FortuneReadingViewPrivate
{
public:
    HexagramBoard *board = nullptr;
    QLabel *nameLabel = nullptr;
    QLabel *trigramLabel = nullptr;
    QLabel *gradeLabel = nullptr;
    QLabel *judgmentLabel = nullptr;
    QLabel *idleLabel = nullptr;
    QLabel *readingLabel = nullptr;
    QLabel *doLabel = nullptr;
    QLabel *avoidLabel = nullptr;
    QLabel *luckyLabel = nullptr;
    QLabel *luckySwatch = nullptr;
    QLabel *birthLabel = nullptr;
    QPushButton *againButton = nullptr;
    QPushButton *skipButton = nullptr;
    QPushButton *copyButton = nullptr;
    QPushButton *saveButton = nullptr;
    QPushButton *resetButton = nullptr;
    QParallelAnimationGroup *revealGroup = nullptr;
    QPropertyAnimation *boardAnimation = nullptr;
    Reading reading;
    bool hasReading = false;
    bool isBusy = false;
};

FortuneReadingView::FortuneReadingView(QWidget *parent)
    : QWidget(parent)
    , d_ptr(std::make_unique<FortuneReadingViewPrivate>())
{
    setupUi();
    retranslateUi();
    setupConnections();
}

FortuneReadingView::~FortuneReadingView() = default;

void FortuneReadingView::showIdle(const BirthChart &chart, const Profile &profile)
{
    d_ptr->hasReading = false;
    d_ptr->copyButton->setVisible(false);
    d_ptr->saveButton->setVisible(false);
    finishReveal();

    const HexagramInfo &info = hexagram(chart.hexagram);
    d_ptr->board->setHexagram(info.upper, info.lower, 0, true);
    d_ptr->board->setRevealProgress(6.0);

    d_ptr->nameLabel->setText(
        tr("Hexagram %1: %2").arg(info.number).arg(hexagramName(chart.hexagram)));
    d_ptr->trigramLabel->setText(
        tr("%1 over %2").arg(trigramImage(info.upper), trigramImage(info.lower)));
    d_ptr->judgmentLabel->setText(hexagramJudgment(chart.hexagram));
    d_ptr->gradeLabel->setText(tr("Birth hexagram"));
    d_ptr->idleLabel->setVisible(true);
    d_ptr->readingLabel->setVisible(false);
    d_ptr->doLabel->setVisible(false);
    d_ptr->avoidLabel->setVisible(false);
    d_ptr->luckySwatch->setVisible(false);
    d_ptr->luckyLabel->setVisible(false);
    d_ptr->birthLabel->setText(tr("Born %1").arg(profile.birthDate.toString(Qt::ISODate)));
    d_ptr->birthLabel->setVisible(true);
}

void FortuneReadingView::showReading(const Reading &reading)
{
    d_ptr->reading = reading;
    d_ptr->hasReading = true;
    d_ptr->copyButton->setVisible(true);
    d_ptr->saveButton->setVisible(true);

    const HexagramInfo &info = hexagram(reading.hexagram);

    d_ptr->nameLabel->setText(
        tr("Hexagram %1: %2").arg(info.number).arg(hexagramName(reading.hexagram)));
    d_ptr->trigramLabel->setText(
        tr("%1 over %2").arg(trigramImage(info.upper), trigramImage(info.lower)));
    d_ptr->gradeLabel->setText(tr("%1 · %2").arg(gradeName(reading.grade), reading.headline));
    d_ptr->judgmentLabel->setText(reading.judgment);
    d_ptr->idleLabel->setVisible(false);
    d_ptr->readingLabel->setVisible(true);
    d_ptr->readingLabel->setText(reading.reading);
    d_ptr->doLabel->setVisible(true);
    d_ptr->doLabel->setText(tr("Do: %1").arg(reading.doList.join(QStringLiteral(" · "))));
    d_ptr->avoidLabel->setVisible(true);
    d_ptr->avoidLabel->setText(tr("Avoid: %1").arg(reading.avoidList.join(QStringLiteral(" · "))));
    d_ptr->luckySwatch->setVisible(true);
    d_ptr->luckySwatch->setStyleSheet(
        QStringLiteral("background: %1; border-radius: 6px;").arg(reading.luckyColor));
    d_ptr->luckyLabel->setVisible(true);
    d_ptr->luckyLabel->setText(
        tr("Element %1 · lucky number %2").arg(reading.luckyElement).arg(reading.luckyNumber));
    d_ptr->birthLabel->setVisible(true);
    d_ptr->birthLabel->setText(tr("Birth hexagram: %1 · Changed: %2")
                                   .arg(reading.birthHexagram)
                                   .arg(hexagramName(reading.changedHexagram)));

    d_ptr->board->setHexagram(info.upper, info.lower, reading.movingLine, false);
    startReveal();
}

void FortuneReadingView::clear()
{
    d_ptr->hasReading = false;
    d_ptr->copyButton->setVisible(false);
    d_ptr->saveButton->setVisible(false);
    finishReveal();
    d_ptr->board->clear();
}

void FortuneReadingView::finishReveal()
{
    d_ptr->revealGroup->stop();
    d_ptr->board->setRevealProgress(6.0);
    d_ptr->skipButton->setVisible(false);
    d_ptr->againButton->setEnabled(true);
    d_ptr->resetButton->setEnabled(true);
    d_ptr->copyButton->setEnabled(d_ptr->hasReading);
    d_ptr->saveButton->setEnabled(d_ptr->hasReading);
    d_ptr->isBusy = false;
}

void FortuneReadingView::focusCastAgain()
{
    d_ptr->againButton->setFocus();
}

void FortuneReadingView::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
        retranslateUi();

    QWidget::changeEvent(event);
}

void FortuneReadingView::hideEvent(QHideEvent *event)
{
    if (d_ptr->isBusy)
        finishReveal();

    QWidget::hideEvent(event);
}

void FortuneReadingView::onSkipClicked()
{
    finishReveal();
}

void FortuneReadingView::onCopyClicked()
{
    if (!d_ptr->hasReading)
        return;

    QGuiApplication::clipboard()->setText(shareText(d_ptr->reading));

    if (auto *shellHost = Shell::host())
        shellHost->showStatusMessage(tr("Reading copied."), 3000);
}

void FortuneReadingView::onSaveImageClicked()
{
    if (!d_ptr->hasReading)
        return;

    QString directory = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    if (directory.isEmpty())
        directory = QDir::homePath();

    const QString suggested = QDir(directory).filePath(
        QStringLiteral("cyber-fortune-%1.png")
            .arg(d_ptr->reading.castAt.toString(QStringLiteral("yyyyMMdd-HHmmss"))));
    const QString path = QFileDialog::getSaveFileName(
        this, tr("Save reading as image"), suggested, tr("PNG image (*.png)"));
    if (path.isEmpty())
        return;

    const QImage image = renderShareImage(d_ptr->reading);
    if (image.isNull() || !image.save(path, "PNG")) {
        if (auto *shellHost = Shell::host())
            shellHost->showStatusMessage(tr("Could not save the image."), 3000);
        return;
    }

    if (auto *shellHost = Shell::host())
        shellHost->showStatusMessage(tr("Reading saved."), 3000);
}

void FortuneReadingView::onRevealFinished()
{
    finishReveal();
}

void FortuneReadingView::onThemeApplied()
{
    d_ptr->board->update();
}

void FortuneReadingView::setupUi()
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 8, 0, 0);
    root->setSpacing(12);

    d_ptr->board = new HexagramBoard(this);
    d_ptr->board->setFixedSize(kBoardWidth, kBoardHeight);

    auto *infoPanel = new QWidget(this);
    infoPanel->setObjectName(QStringLiteral("FortuneInfo"));
    infoPanel->setMinimumWidth(kInfoMinWidth);
    infoPanel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);

    d_ptr->nameLabel = new QLabel(infoPanel);
    d_ptr->nameLabel->setObjectName(QStringLiteral("HeroTitle"));
    d_ptr->nameLabel->setWordWrap(true);

    d_ptr->trigramLabel = new QLabel(infoPanel);
    d_ptr->trigramLabel->setObjectName(QStringLiteral("HeroMeta"));
    d_ptr->trigramLabel->setWordWrap(true);

    d_ptr->gradeLabel = new QLabel(infoPanel);
    d_ptr->gradeLabel->setObjectName(QStringLiteral("CardTitle"));
    d_ptr->gradeLabel->setWordWrap(true);

    d_ptr->judgmentLabel = new QLabel(infoPanel);
    d_ptr->judgmentLabel->setObjectName(QStringLiteral("CardHint"));
    d_ptr->judgmentLabel->setWordWrap(true);

    d_ptr->idleLabel = new QLabel(infoPanel);
    d_ptr->idleLabel->setObjectName(QStringLiteral("Placeholder"));
    d_ptr->idleLabel->setWordWrap(true);

    d_ptr->readingLabel = new QLabel(infoPanel);
    d_ptr->readingLabel->setObjectName(QStringLiteral("CardHint"));
    d_ptr->readingLabel->setWordWrap(true);

    d_ptr->doLabel = new QLabel(infoPanel);
    d_ptr->doLabel->setObjectName(QStringLiteral("CardHint"));
    d_ptr->doLabel->setWordWrap(true);

    d_ptr->avoidLabel = new QLabel(infoPanel);
    d_ptr->avoidLabel->setObjectName(QStringLiteral("CardHint"));
    d_ptr->avoidLabel->setWordWrap(true);

    d_ptr->luckySwatch = new QLabel(infoPanel);
    d_ptr->luckySwatch->setFixedSize(12, 12);

    d_ptr->luckyLabel = new QLabel(infoPanel);
    d_ptr->luckyLabel->setObjectName(QStringLiteral("CardHint"));
    d_ptr->luckyLabel->setWordWrap(true);

    auto *luckyRow = new QHBoxLayout;
    luckyRow->setContentsMargins(0, 0, 0, 0);
    luckyRow->setSpacing(6);
    luckyRow->addWidget(d_ptr->luckySwatch);
    luckyRow->addWidget(d_ptr->luckyLabel, 1);

    d_ptr->birthLabel = new QLabel(infoPanel);
    d_ptr->birthLabel->setObjectName(QStringLiteral("HeroMeta"));
    d_ptr->birthLabel->setWordWrap(true);

    auto *infoLayout = new QVBoxLayout(infoPanel);
    infoLayout->setContentsMargins(0, 0, 0, 0);
    infoLayout->setSpacing(5);
    infoLayout->addWidget(d_ptr->nameLabel);
    infoLayout->addWidget(d_ptr->trigramLabel);
    infoLayout->addWidget(d_ptr->gradeLabel);
    infoLayout->addWidget(d_ptr->judgmentLabel);
    infoLayout->addWidget(d_ptr->idleLabel);
    infoLayout->addWidget(d_ptr->readingLabel);
    infoLayout->addWidget(d_ptr->doLabel);
    infoLayout->addWidget(d_ptr->avoidLabel);
    infoLayout->addLayout(luckyRow);
    infoLayout->addWidget(d_ptr->birthLabel);
    infoLayout->addStretch();

    auto *textScroll = new QScrollArea(this);
    textScroll->setObjectName(QStringLiteral("FortuneTextScroll"));
    textScroll->setWidgetResizable(true);
    textScroll->setFrameShape(QFrame::NoFrame);
    textScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    textScroll->setStyleSheet(
        QStringLiteral("QScrollArea { background: transparent; border: none; }"));
    textScroll->viewport()->setAutoFillBackground(false);
    textScroll->setWidget(infoPanel);

    auto *topRow = new QHBoxLayout;
    topRow->setContentsMargins(0, 0, 0, 0);
    topRow->setSpacing(24);
    topRow->addWidget(d_ptr->board, 0, Qt::AlignTop);
    topRow->addWidget(textScroll, 1);

    d_ptr->againButton = new QPushButton(this);
    d_ptr->againButton->setObjectName(QStringLiteral("PrimaryButton"));

    d_ptr->skipButton = new QPushButton(this);
    d_ptr->skipButton->setVisible(false);

    d_ptr->copyButton = new QPushButton(this);
    d_ptr->copyButton->setVisible(false);

    d_ptr->saveButton = new QPushButton(this);
    d_ptr->saveButton->setVisible(false);

    d_ptr->resetButton = new QPushButton(this);

    auto *buttonRow = new QHBoxLayout;
    buttonRow->setContentsMargins(0, 0, 0, 0);
    buttonRow->setSpacing(8);
    buttonRow->addWidget(d_ptr->againButton);
    buttonRow->addWidget(d_ptr->skipButton);
    buttonRow->addWidget(d_ptr->copyButton);
    buttonRow->addWidget(d_ptr->saveButton);
    buttonRow->addStretch();
    buttonRow->addWidget(d_ptr->resetButton);

    root->addLayout(topRow, 1);
    root->addLayout(buttonRow);

    d_ptr->revealGroup = new QParallelAnimationGroup(this);

    d_ptr->boardAnimation
        = new QPropertyAnimation(d_ptr->board, "revealProgress", d_ptr->revealGroup);
    d_ptr->boardAnimation->setDuration(kRevealDurationMs);
    d_ptr->boardAnimation->setStartValue(0.0);
    d_ptr->boardAnimation->setEndValue(6.0);
    d_ptr->boardAnimation->setEasingCurve(QEasingCurve::OutCubic);
}

void FortuneReadingView::retranslateUi()
{
    d_ptr->againButton->setText(tr("Cast again"));
    d_ptr->skipButton->setText(tr("Skip"));
    d_ptr->copyButton->setText(tr("Copy"));
    d_ptr->saveButton->setText(tr("Save image"));
    d_ptr->resetButton->setText(tr("Reset profile"));
    d_ptr->idleLabel->setText(tr("Press Cast to ask about today."));
}

void FortuneReadingView::setupConnections()
{
    connect(d_ptr->againButton, &QPushButton::clicked, this, &FortuneReadingView::castAgainRequested);
    connect(d_ptr->resetButton, &QPushButton::clicked, this, &FortuneReadingView::resetRequested);
    connect(d_ptr->skipButton, &QPushButton::clicked, this, &FortuneReadingView::onSkipClicked);
    connect(d_ptr->copyButton, &QPushButton::clicked, this, &FortuneReadingView::onCopyClicked);
    connect(d_ptr->saveButton, &QPushButton::clicked, this, &FortuneReadingView::onSaveImageClicked);
    connect(
        d_ptr->revealGroup,
        &QParallelAnimationGroup::finished,
        this,
        &FortuneReadingView::onRevealFinished);
    connect(Utils::theme(), &Utils::Theme::themeApplied, this, &FortuneReadingView::onThemeApplied);
}

void FortuneReadingView::startReveal()
{
    d_ptr->board->setRevealProgress(0.0);
    d_ptr->skipButton->setVisible(true);
    d_ptr->againButton->setEnabled(false);
    d_ptr->resetButton->setEnabled(false);
    d_ptr->copyButton->setEnabled(false);
    d_ptr->saveButton->setEnabled(false);
    d_ptr->isBusy = true;
    d_ptr->revealGroup->start();
}

} // namespace Fortune
