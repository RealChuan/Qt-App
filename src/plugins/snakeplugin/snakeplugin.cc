#include "snakeplugin.hpp"

#include "snakepage.hpp"

#include <extensionsystem/plugincontext.hpp>
#include <shell/functioncommand.hpp>
#include <shell/host.hpp>
#include <shell/registry.hpp>

#include <QKeySequence>
#include <QSettings>

namespace Snake {

namespace {

constexpr QLatin1StringView kDifficultyKey("difficulty");
constexpr QLatin1StringView kBestScoreGroup("bestScore");
constexpr QLatin1StringView kEasyKey("easy");
constexpr QLatin1StringView kNormalKey("normal");
constexpr QLatin1StringView kHardKey("hard");

// 存档里用短字符串而不是枚举值：以后加减难度不会把旧配置读成别的档。
auto difficultyKey(Difficulty difficulty) -> QString
{
    switch (difficulty) {
    case Difficulty::Easy:
        return QString(kEasyKey);
    case Difficulty::Normal:
        return QString(kNormalKey);
    case Difficulty::Hard:
        return QString(kHardKey);
    }
    return QString(kNormalKey);
}

auto difficultyFromKey(const QString &key) -> Difficulty
{
    if (key == kEasyKey)
        return Difficulty::Easy;
    if (key == kHardKey)
        return Difficulty::Hard;
    return Difficulty::Normal;
}

// 最高分按难度分开存：否则轻松档迟早把纪录刷到挑战档追不上。
auto bestScoreKey(Difficulty difficulty) -> QString
{
    return QString(kBestScoreGroup) + QLatin1Char('/') + difficultyKey(difficulty);
}

} // namespace

class SnakePlugin::SnakePluginPrivate
{
public:
    [[nodiscard]] auto bestScore(Difficulty difficulty) const -> int
    {
        return settings->value(bestScoreKey(difficulty), 0).toInt();
    }

    SnakePage *page = nullptr;
    QSettings *settings = nullptr;
    std::unique_ptr<Shell::FunctionCommand> command;
    Difficulty difficulty = Difficulty::Normal;
};

SnakePlugin::SnakePlugin()
    : d_ptr(std::make_unique<SnakePluginPrivate>())
{}

SnakePlugin::~SnakePlugin() = default;

auto SnakePlugin::initialize(const ExtensionSystem::PluginContext &context) -> Utils::Result<>
{
    d_ptr->settings = context.settings();
    d_ptr->difficulty = difficultyFromKey(
        d_ptr->settings->value(kDifficultyKey, QString(kNormalKey)).toString());

    d_ptr->page = new SnakePage;
    d_ptr->page->setDifficulty(d_ptr->difficulty);
    d_ptr->page->setBestScore(d_ptr->bestScore(d_ptr->difficulty));

    d_ptr->command = std::make_unique<Shell::FunctionCommand>(
        QStringLiteral("snakeplugin.play"),
        tr("Snake: play a round"),
        std::bind_front(&SnakePlugin::play, this));
    d_ptr->command->withShortcut(QKeySequence(QStringLiteral("Ctrl+Alt+S")));

    connect(d_ptr->page, &SnakePage::gameFinished, this, &SnakePlugin::onGameFinished);
    connect(d_ptr->page, &SnakePage::difficultyChanged, this, &SnakePlugin::onDifficultyChanged);

    return Utils::ResultOk;
}

void SnakePlugin::extensionsInitialized()
{
    auto *registry = Shell::Registry::instance();
    registry->addPage(d_ptr->page);
    registry->addCommand(d_ptr->command.get());
}

void SnakePlugin::play()
{
    // 先切页面再开局：棋盘出现时已经进入 Running，不会先闪一帧空盘。
    if (auto *shellHost = Shell::host())
        shellHost->showPage(QStringLiteral("snakeplugin.page"));

    d_ptr->page->startGame();
}

void SnakePlugin::onGameFinished(int score)
{
    // 结算只在页面里发生一次，这里也只在真正刷新纪录时写盘。
    if (score <= d_ptr->bestScore(d_ptr->difficulty))
        return;

    d_ptr->settings->setValue(bestScoreKey(d_ptr->difficulty), score);
    d_ptr->page->setBestScore(score);
}

void SnakePlugin::onDifficultyChanged(Snake::Difficulty difficulty)
{
    d_ptr->difficulty = difficulty;
    d_ptr->settings->setValue(kDifficultyKey, difficultyKey(difficulty));
    // 换档就换纪录：显示的是当前难度的最高分。
    d_ptr->page->setBestScore(d_ptr->bestScore(difficulty));
}

} // namespace Snake
