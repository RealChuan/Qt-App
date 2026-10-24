#include "snakepage.hpp"

#include "snakeboard.hpp"

#include <QComboBox>
#include <QEvent>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QListView>
#include <QPushButton>
#include <QShowEvent>
#include <QSignalBlocker>
#include <QTimer>
#include <QVBoxLayout>

#include <algorithm>

namespace Snake {

namespace {

constexpr int kColumns = 28;
constexpr int kRows = 20;
constexpr int kCardMinWidth = 560;
constexpr int kCardMaxWidth = 680;
constexpr int kBoardMinHeight = 300;
constexpr int kCardHorizontalMargin = 24;

} // namespace

class SnakePage::SnakePagePrivate
{
public:
    explicit SnakePagePrivate(SnakePage *q)
        : q_ptr(q)
        , game(kColumns, kRows)
    {}

    void setupUi();
    void startRun();
    void step();
    void togglePause();
    void finishRun();
    void syncButtons();
    void updateScore();

    SnakePage *q_ptr = nullptr;
    SnakeGame game;
    SnakeBoard *board = nullptr;
    QLabel *score = nullptr;
    QLabel *best = nullptr;
    QLabel *hint = nullptr;
    QPushButton *primary = nullptr;
    QPushButton *pause = nullptr;
    QComboBox *difficultyBox = nullptr;
    QTimer timer;

    QString startText;
    QString playAgainText;
    QString pauseText;
    QString resumeText;
    int bestScore = 0;
};

void SnakePage::SnakePagePrivate::setupUi()
{
    board = new SnakeBoard(q_ptr);
    board->setMinimumHeight(kBoardMinHeight);

    score = new QLabel(q_ptr);
    score->setObjectName("HeroNumber");
    score->setAlignment(Qt::AlignLeft | Qt::AlignBottom);

    best = new QLabel(q_ptr);
    best->setObjectName("HeroMeta");
    best->setAlignment(Qt::AlignRight | Qt::AlignBottom);

    auto *scoreRow = new QHBoxLayout;
    scoreRow->setContentsMargins({});
    scoreRow->setSpacing(8);
    scoreRow->addWidget(score);
    scoreRow->addStretch();
    scoreRow->addWidget(best);

    hint = new QLabel(q_ptr);
    hint->setObjectName("FieldLabel");
    hint->setWordWrap(true);
    hint->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    primary = new QPushButton(q_ptr);
    primary->setObjectName("PrimaryButton");
    primary->setCursor(Qt::PointingHandCursor);

    pause = new QPushButton(q_ptr);
    pause->setCursor(Qt::PointingHandCursor);

    // 难度是"下一局"的设定：数据里存枚举，显示文字在 retranslateUi() 里补。
    difficultyBox = new QComboBox(q_ptr);
    difficultyBox->setView(new QListView(difficultyBox));
    for (const Difficulty difficulty : {Difficulty::Easy, Difficulty::Normal, Difficulty::Hard}) {
        difficultyBox->addItem(QString(), static_cast<int>(difficulty));
    }
    difficultyBox->setCurrentIndex(1); // 默认 Normal

    auto *controls = new QHBoxLayout;
    controls->setContentsMargins({});
    controls->setSpacing(8);
    controls->addWidget(primary);
    controls->addWidget(pause);
    controls->addWidget(difficultyBox);
    controls->addStretch();
    controls->addWidget(hint, 1);

    auto *divider = new QFrame(q_ptr);
    divider->setObjectName("Divider");
    divider->setFrameShape(QFrame::NoFrame);

    auto *card = new QFrame(q_ptr);
    card->setObjectName("Card");
    card->setMinimumWidth(kCardMinWidth);
    card->setMaximumWidth(kCardMaxWidth);
    auto *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(kCardHorizontalMargin, 20, kCardHorizontalMargin, 18);
    cardLayout->setSpacing(12);
    cardLayout->addLayout(scoreRow);
    cardLayout->addWidget(board, 1);
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

void SnakePage::SnakePagePrivate::startRun()
{
    game.start();
    timer.start(game.stepIntervalMs());
    board->update();
    updateScore();
    syncButtons();
    // 焦点回到棋盘，方向键立刻可用；按钮点完不要抢走它。
    board->setFocus();
}

void SnakePage::SnakePagePrivate::step()
{
    game.step();
    board->update();
    updateScore();

    if (game.state() == GameState::Over) {
        finishRun();
        return;
    }

    // 吃到食物后步进变快，定时器跟着换档。
    if (timer.interval() != game.stepIntervalMs())
        timer.setInterval(game.stepIntervalMs());
}

void SnakePage::SnakePagePrivate::togglePause()
{
    if (game.state() != GameState::Running && game.state() != GameState::Paused)
        return;

    game.togglePause();
    if (game.state() == GameState::Paused)
        timer.stop();
    else
        timer.start(game.stepIntervalMs());

    board->update();
    syncButtons();
    board->setFocus();
}

void SnakePage::SnakePagePrivate::finishRun()
{
    // 停表 + 状态已经是 Over，保证结算只发生这一次。
    timer.stop();
    syncButtons();
    board->setFocus();
    emit q_ptr->gameFinished(game.score());
}

void SnakePage::SnakePagePrivate::syncButtons()
{
    const GameState state = game.state();
    const bool isRunning = state == GameState::Running;
    const bool isPaused = state == GameState::Paused;

    primary->setText(state == GameState::Over ? playAgainText : startText);
    primary->setEnabled(!isRunning && !isPaused);
    pause->setText(isPaused ? resumeText : pauseText);
    pause->setEnabled(isRunning || isPaused);
    // 难度只在开局前可选：半途变速等于换了个游戏。
    difficultyBox->setEnabled(!isRunning && !isPaused);
}

void SnakePage::SnakePagePrivate::updateScore()
{
    score->setText(QString::number(game.score()));
}

SnakePage::SnakePage(QWidget *parent)
    : QWidget(parent)
    , d_ptr(std::make_unique<SnakePagePrivate>(this))
{
    d_ptr->setupUi();
    d_ptr->board->setGame(&d_ptr->game);

    d_ptr->timer.setTimerType(Qt::PreciseTimer);

    retranslateUi();
    setupConnections();

    d_ptr->syncButtons();
    d_ptr->updateScore();
}

SnakePage::~SnakePage() = default;

void SnakePage::retranslateUi()
{
    d_ptr->startText = tr("Start");
    d_ptr->playAgainText = tr("Play again");
    d_ptr->pauseText = tr("Pause");
    d_ptr->resumeText = tr("Resume");
    d_ptr->hint->setText(tr("Arrows or WASD · Space starts and pauses"));
    d_ptr->difficultyBox->setToolTip(tr("Starting speed for the next round"));

    for (int i = 0; i < d_ptr->difficultyBox->count(); ++i) {
        const auto difficulty = static_cast<Difficulty>(d_ptr->difficultyBox->itemData(i).toInt());
        d_ptr->difficultyBox->setItemText(i, difficultyLabel(difficulty));
    }

    updateBestLabel();
    d_ptr->syncButtons();
}

auto SnakePage::difficultyLabel(Difficulty difficulty) const -> QString
{
    switch (difficulty) {
    case Difficulty::Easy:
        return tr("Easy");
    case Difficulty::Normal:
        return tr("Normal");
    case Difficulty::Hard:
        return tr("Hard");
    }
    return {};
}

// tr() 只留在 QObject 类里：Private 只存数字，文案由页面负责。
void SnakePage::updateBestLabel()
{
    d_ptr->best->setText(tr("Best %1").arg(d_ptr->bestScore));
}

void SnakePage::setupConnections()
{
    connect(&d_ptr->timer, &QTimer::timeout, this, &SnakePage::onFrameTimeout);
    connect(d_ptr->board, &SnakeBoard::turnRequested, this, &SnakePage::onTurnRequested);
    connect(d_ptr->board, &SnakeBoard::pauseToggled, this, &SnakePage::onPauseToggled);
    connect(d_ptr->board, &SnakeBoard::primaryActionRequested, this, &SnakePage::onPrimaryAction);
    connect(d_ptr->primary, &QPushButton::clicked, this, &SnakePage::onPrimaryAction);
    connect(d_ptr->pause, &QPushButton::clicked, this, &SnakePage::onPauseToggled);
    connect(
        d_ptr->difficultyBox,
        &QComboBox::currentIndexChanged,
        this,
        &SnakePage::onDifficultyChanged);
}

void SnakePage::setBestScore(int score)
{
    d_ptr->bestScore = std::max(0, score);
    updateBestLabel();
}

void SnakePage::setDifficulty(Difficulty difficulty)
{
    // 回填存档时不要再走一遍"用户改了难度"的流程。
    const QSignalBlocker blocker(d_ptr->difficultyBox);
    const int index = d_ptr->difficultyBox->findData(static_cast<int>(difficulty));
    if (index >= 0)
        d_ptr->difficultyBox->setCurrentIndex(index);

    d_ptr->game.setDifficulty(difficulty);
}

void SnakePage::startGame()
{
    d_ptr->startRun();
}

void SnakePage::onFrameTimeout()
{
    d_ptr->step();
}

void SnakePage::onPrimaryAction()
{
    if (d_ptr->game.state() == GameState::Running || d_ptr->game.state() == GameState::Paused) {
        d_ptr->togglePause();
        return;
    }

    d_ptr->startRun();
}

void SnakePage::onPauseToggled()
{
    d_ptr->togglePause();
}

void SnakePage::onTurnRequested(Snake::Direction direction)
{
    d_ptr->game.turn(direction);
}

void SnakePage::onDifficultyChanged(int index)
{
    const auto difficulty = static_cast<Difficulty>(d_ptr->difficultyBox->itemData(index).toInt());
    d_ptr->game.setDifficulty(difficulty);
    // 选完难度键盘要立刻能玩，别把焦点留在下拉框上。
    d_ptr->board->setFocus();

    emit difficultyChanged(difficulty);
}

void SnakePage::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    d_ptr->board->setFocus();
}

void SnakePage::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
        retranslateUi();
    QWidget::changeEvent(event);
}

} // namespace Snake
