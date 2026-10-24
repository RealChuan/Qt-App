#include "tetrispage.hpp"

#include "tetrisboard.hpp"
#include "tetrispreview.hpp"

#include <QElapsedTimer>
#include <QEvent>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QShowEvent>
#include <QTimer>
#include <QVBoxLayout>

#include <algorithm>

namespace Tetris {

namespace {

constexpr int kFrameIntervalMs = 16;
constexpr int kMaxFrameMs = 50; // 窗口被拖动时别一次补太多帧
constexpr int kDasMs = 170;
constexpr int kArrMs = 50;
constexpr int kSoftDropMs = 50;

constexpr int kLeftWidth = 160;
constexpr int kRightWidth = 184;
constexpr int kNextSlots = 3;
constexpr int kColumnSpacing = 36;
constexpr int kCardMargin = 32;
// 三栏（暂存 | 井 | 分数与队列）把卡片撑到和贪吃蛇齐平（680）：
// 32 + 160 + 36 + 200 + 36 + 184 + 32。井两侧因此是有内容的栏位，而不是死白。
constexpr int kCardWidth = kCardMargin * 2 + kLeftWidth + TetrisBoard::kPixelWidth + kRightWidth
                           + kColumnSpacing * 2;

auto makeDivider(QWidget *parent) -> QFrame *
{
    auto *divider = new QFrame(parent);
    divider->setObjectName("Divider");
    divider->setFrameShape(QFrame::NoFrame);
    return divider;
}

} // namespace

class TetrisPage::TetrisPagePrivate
{
public:
    explicit TetrisPagePrivate(TetrisPage *q)
        : q_ptr(q)
    {}

    void setupUi();
    void startRun();
    void finishRun();
    void syncButtons();
    void tick(int dtMs);
    void setHorizontal(int direction);
    void pressKey(int key);
    void releaseKey(int key);
    void resetInput();

    TetrisPage *q_ptr = nullptr;
    TetrisGame game;
    TetrisBoard *board = nullptr;
    TetrisPreview *nextView = nullptr;
    TetrisPreview *holdView = nullptr;
    QLabel *score = nullptr;
    QLabel *best = nullptr;
    QLabel *scoreCaption = nullptr;
    QLabel *nextCaption = nullptr;
    QLabel *holdCaption = nullptr;
    QLabel *levelLine = nullptr;
    QLabel *hint = nullptr;
    QPushButton *primary = nullptr;
    QPushButton *pause = nullptr;
    QTimer frameTimer;
    QElapsedTimer clock;

    int horizontal = 0;
    int dasMs = 0;
    int arrMs = 0;
    bool isLeftHeld = false;
    bool isRightHeld = false;
    bool isDownHeld = false;
    int softDropMs = 0;

    QString startText;
    QString playAgainText;
    QString pauseText;
    QString resumeText;
    int bestScore = 0;
};

void TetrisPage::TetrisPagePrivate::setupUi()
{
    board = new TetrisBoard(q_ptr);
    board->setFixedSize(TetrisBoard::kPixelWidth, TetrisBoard::kPixelHeight);

    score = new QLabel(q_ptr);
    score->setObjectName("HeroNumber");
    score->setAlignment(Qt::AlignLeft | Qt::AlignBottom);

    best = new QLabel(q_ptr);
    best->setObjectName("HeroMeta");
    best->setAlignment(Qt::AlignRight | Qt::AlignBottom);

    scoreCaption = new QLabel(q_ptr);
    scoreCaption->setObjectName("FieldLabel");
    nextCaption = new QLabel(q_ptr);
    nextCaption->setObjectName("FieldLabel");
    holdCaption = new QLabel(q_ptr);
    holdCaption->setObjectName("FieldLabel");
    levelLine = new QLabel(q_ptr);
    levelLine->setObjectName("FieldLabel");
    levelLine->setWordWrap(true);

    nextView = new TetrisPreview(q_ptr);
    nextView->setSlots(kNextSlots);
    holdView = new TetrisPreview(q_ptr);
    holdView->setSlots(1);

    // 左栏：暂存与等级。
    auto *leftColumn = new QWidget(q_ptr);
    leftColumn->setFixedWidth(kLeftWidth);
    auto *leftLayout = new QVBoxLayout(leftColumn);
    leftLayout->setContentsMargins({});
    leftLayout->setSpacing(6);
    leftLayout->addWidget(holdCaption);
    leftLayout->addWidget(holdView);
    leftLayout->addStretch();
    leftLayout->addWidget(levelLine);

    // 右栏：最高分、分数、Next 队列。
    auto *rightColumn = new QWidget(q_ptr);
    rightColumn->setFixedWidth(kRightWidth);
    auto *rightLayout = new QVBoxLayout(rightColumn);
    rightLayout->setContentsMargins({});
    rightLayout->setSpacing(6);
    rightLayout->addWidget(best);
    rightLayout->addWidget(score);
    rightLayout->addWidget(scoreCaption);
    rightLayout->addWidget(makeDivider(rightColumn));
    rightLayout->addWidget(nextCaption);
    rightLayout->addWidget(nextView);
    rightLayout->addStretch();

    primary = new QPushButton(q_ptr);
    primary->setObjectName("PrimaryButton");
    primary->setCursor(Qt::PointingHandCursor);
    pause = new QPushButton(q_ptr);
    pause->setCursor(Qt::PointingHandCursor);
    hint = new QLabel(q_ptr);
    hint->setObjectName("FieldLabel");
    hint->setWordWrap(true);
    hint->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    auto *controls = new QHBoxLayout;
    controls->setContentsMargins({});
    controls->setSpacing(8);
    controls->addWidget(primary);
    controls->addWidget(pause);
    controls->addStretch();
    controls->addWidget(hint, 1);

    auto *body = new QHBoxLayout;
    body->setContentsMargins({});
    body->setSpacing(kColumnSpacing);
    body->addWidget(leftColumn);
    body->addWidget(board, 0, Qt::AlignVCenter);
    body->addWidget(rightColumn);

    auto *divider = makeDivider(q_ptr);

    auto *card = new QFrame(q_ptr);
    card->setObjectName("Card");
    card->setFixedWidth(kCardWidth);
    auto *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(kCardMargin, 18, kCardMargin, 16);
    cardLayout->setSpacing(12);
    cardLayout->addLayout(body, 1);
    cardLayout->addWidget(divider);
    cardLayout->addLayout(controls);

    auto *centerRow = new QHBoxLayout;
    centerRow->setContentsMargins({});
    centerRow->addStretch();
    centerRow->addWidget(card, 1);
    centerRow->addStretch();

    auto *layout = new QVBoxLayout(q_ptr);
    layout->setContentsMargins({});
    layout->addLayout(centerRow, 1);
}

void TetrisPage::TetrisPagePrivate::startRun()
{
    game.start();
    resetInput();
    clock.restart();
    frameTimer.start(kFrameIntervalMs);

    board->update();
    syncButtons();
    board->setFocus();
}

void TetrisPage::TetrisPagePrivate::finishRun()
{
    frameTimer.stop();
    resetInput();
    syncButtons();
    board->update();
    board->setFocus();

    emit q_ptr->gameFinished(game.score());
}

void TetrisPage::TetrisPagePrivate::syncButtons()
{
    const GameState state = game.state();
    const bool isRunning = state == GameState::Running;
    const bool isPaused = state == GameState::Paused;

    primary->setText(state == GameState::Over ? playAgainText : startText);
    primary->setEnabled(!isRunning && !isPaused);
    pause->setText(isPaused ? resumeText : pauseText);
    pause->setEnabled(isRunning || isPaused);
}

void TetrisPage::TetrisPagePrivate::tick(int dtMs)
{
    // 左右连发：先等 DAS，再按 ARR 一格一格走。
    if (horizontal != 0) {
        dasMs += dtMs;
        if (dasMs >= kDasMs) {
            arrMs += dtMs;
            while (arrMs >= kArrMs) {
                arrMs -= kArrMs;
                if (!game.moveBy(horizontal))
                    break;
            }
        }
    }

    if (isDownHeld) {
        softDropMs += dtMs;
        while (softDropMs >= kSoftDropMs) {
            softDropMs -= kSoftDropMs;
            game.softDrop();
        }
    }

    game.advance(dtMs);
}

void TetrisPage::TetrisPagePrivate::setHorizontal(int direction)
{
    if (horizontal == direction)
        return;

    horizontal = direction;
    dasMs = 0;
    arrMs = 0;

    if (direction != 0)
        game.moveBy(direction);
}

void TetrisPage::TetrisPagePrivate::pressKey(int key)
{
    switch (key) {
    case Qt::Key_Left:
    case Qt::Key_A:
        isLeftHeld = true;
        setHorizontal(-1);
        return;
    case Qt::Key_Right:
    case Qt::Key_D:
        isRightHeld = true;
        setHorizontal(1);
        return;
    case Qt::Key_Down:
    case Qt::Key_S:
        isDownHeld = true;
        softDropMs = 0;
        game.softDrop();
        return;
    case Qt::Key_Up:
    case Qt::Key_W:
    case Qt::Key_X:
        game.rotate(true);
        return;
    case Qt::Key_Z:
        game.rotate(false);
        return;
    case Qt::Key_Space:
        game.hardDrop();
        resetInput();
        return;
    case Qt::Key_C:
        game.hold();
        return;
    case Qt::Key_P:
    case Qt::Key_Escape:
        q_ptr->onPauseToggled();
        return;
    case Qt::Key_Return:
    case Qt::Key_Enter:
        q_ptr->onPrimaryAction();
        return;
    default:
        return;
    }
}

void TetrisPage::TetrisPagePrivate::releaseKey(int key)
{
    switch (key) {
    case Qt::Key_Left:
    case Qt::Key_A:
        isLeftHeld = false;
        if (horizontal < 0)
            setHorizontal(isRightHeld ? 1 : 0);
        return;
    case Qt::Key_Right:
    case Qt::Key_D:
        isRightHeld = false;
        if (horizontal > 0)
            setHorizontal(isLeftHeld ? -1 : 0);
        return;
    case Qt::Key_Down:
    case Qt::Key_S:
        isDownHeld = false;
        return;
    default:
        return;
    }
}

void TetrisPage::TetrisPagePrivate::resetInput()
{
    horizontal = 0;
    dasMs = 0;
    arrMs = 0;
    isLeftHeld = false;
    isRightHeld = false;
    isDownHeld = false;
    softDropMs = 0;
}

TetrisPage::TetrisPage(QWidget *parent)
    : QWidget(parent)
    , d_ptr(std::make_unique<TetrisPagePrivate>(this))
{
    d_ptr->setupUi();
    d_ptr->board->setGame(&d_ptr->game);

    d_ptr->frameTimer.setTimerType(Qt::PreciseTimer);

    retranslateUi();
    setupConnections();

    d_ptr->syncButtons();
    updateLabels();
}

TetrisPage::~TetrisPage() = default;

void TetrisPage::retranslateUi()
{
    d_ptr->startText = tr("Start");
    d_ptr->playAgainText = tr("Play again");
    d_ptr->pauseText = tr("Pause");
    d_ptr->resumeText = tr("Resume");
    d_ptr->scoreCaption->setText(tr("Score"));
    d_ptr->nextCaption->setText(tr("Next"));
    d_ptr->holdCaption->setText(tr("Hold"));
    d_ptr->hint->setText(tr("Arrows move · Up spins · Space drops · C holds"));
    updateBestLabel();
    updateLabels();
    d_ptr->syncButtons();
}

void TetrisPage::setupConnections()
{
    connect(&d_ptr->frameTimer, &QTimer::timeout, this, &TetrisPage::onFrame);
    connect(d_ptr->board, &TetrisBoard::keyPressed, this, &TetrisPage::onKeyPressed);
    connect(d_ptr->board, &TetrisBoard::keyReleased, this, &TetrisPage::onKeyReleased);
    connect(d_ptr->primary, &QPushButton::clicked, this, &TetrisPage::onPrimaryAction);
    connect(d_ptr->pause, &QPushButton::clicked, this, &TetrisPage::onPauseToggled);
}

void TetrisPage::updateBestLabel()
{
    d_ptr->best->setText(tr("Best %1").arg(d_ptr->bestScore));
}

void TetrisPage::updateLabels()
{
    d_ptr->score->setText(QString::number(d_ptr->game.score()));
    d_ptr->levelLine->setText(
        tr("Level %1 · %2 lines").arg(d_ptr->game.level()).arg(d_ptr->game.lines()));

    d_ptr->nextView->setPieces(d_ptr->game.nextQueue(kNextSlots));

    const std::optional<PieceType> held = d_ptr->game.heldType();
    d_ptr->holdView->setPieces(held.has_value() ? QVector<PieceType>{*held} : QVector<PieceType>{});
}

void TetrisPage::setBestScore(int score)
{
    d_ptr->bestScore = std::max(0, score);
    updateBestLabel();
}

void TetrisPage::startGame()
{
    d_ptr->startRun();
}

void TetrisPage::onFrame()
{
    const int dt = std::clamp(static_cast<int>(d_ptr->clock.restart()), 1, kMaxFrameMs);

    d_ptr->tick(dt);
    d_ptr->board->update();
    updateLabels();

    if (d_ptr->game.state() == GameState::Over)
        d_ptr->finishRun();
}

void TetrisPage::onPrimaryAction()
{
    if (d_ptr->game.state() == GameState::Running || d_ptr->game.state() == GameState::Paused) {
        onPauseToggled();
        return;
    }

    d_ptr->startRun();
}

void TetrisPage::onPauseToggled()
{
    if (d_ptr->game.state() != GameState::Running && d_ptr->game.state() != GameState::Paused)
        return;

    d_ptr->game.togglePause();
    d_ptr->resetInput();

    if (d_ptr->game.state() == GameState::Paused) {
        d_ptr->frameTimer.stop();
    } else {
        d_ptr->clock.restart();
        d_ptr->frameTimer.start(kFrameIntervalMs);
    }

    d_ptr->board->update();
    d_ptr->syncButtons();
    d_ptr->board->setFocus();
}

void TetrisPage::onKeyPressed(int key)
{
    d_ptr->pressKey(key);
    d_ptr->board->update();
}

void TetrisPage::onKeyReleased(int key)
{
    d_ptr->releaseKey(key);
}

void TetrisPage::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    d_ptr->board->setFocus();
}

void TetrisPage::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
        retranslateUi();
    QWidget::changeEvent(event);
}

} // namespace Tetris
