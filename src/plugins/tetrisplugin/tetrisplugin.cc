#include "tetrisplugin.hpp"

#include "tetrispage.hpp"

#include <extensionsystem/plugincontext.hpp>
#include <shell/functioncommand.hpp>
#include <shell/host.hpp>
#include <shell/registry.hpp>

#include <QKeySequence>
#include <QSettings>

namespace Tetris {

namespace {

constexpr QLatin1StringView kBestScoreKey("bestScore");

} // namespace

class TetrisPlugin::TetrisPluginPrivate
{
public:
    TetrisPage *page = nullptr;
    QSettings *settings = nullptr;
    std::unique_ptr<Shell::FunctionCommand> command;
    int bestScore = 0;
};

TetrisPlugin::TetrisPlugin()
    : d_ptr(std::make_unique<TetrisPluginPrivate>())
{}

TetrisPlugin::~TetrisPlugin() = default;

auto TetrisPlugin::initialize(const ExtensionSystem::PluginContext &context) -> Utils::Result<>
{
    d_ptr->settings = context.settings();
    d_ptr->bestScore = d_ptr->settings->value(kBestScoreKey, 0).toInt();

    d_ptr->page = new TetrisPage;
    d_ptr->page->setBestScore(d_ptr->bestScore);

    d_ptr->command = std::make_unique<Shell::FunctionCommand>(
        QStringLiteral("tetrisplugin.play"),
        tr("Tetris: play a round"),
        std::bind_front(&TetrisPlugin::play, this));
    d_ptr->command->withShortcut(QKeySequence(QStringLiteral("Ctrl+Alt+T")));

    connect(d_ptr->page, &TetrisPage::gameFinished, this, &TetrisPlugin::onGameFinished);

    return Utils::ResultOk;
}

void TetrisPlugin::extensionsInitialized()
{
    auto *registry = Shell::Registry::instance();
    registry->addPage(d_ptr->page);
    registry->addCommand(d_ptr->command.get());
}

void TetrisPlugin::play()
{
    // 先切页面再开局：棋盘出现时已经在跑，不会先闪一帧空盘。
    if (auto *shellHost = Shell::host())
        shellHost->showPage(QStringLiteral("tetrisplugin.page"));

    d_ptr->page->startGame();
}

void TetrisPlugin::onGameFinished(int score)
{
    // 结算只在页面里发生一次，这里只在真正刷新纪录时写盘。
    if (score <= d_ptr->bestScore)
        return;

    d_ptr->bestScore = score;
    d_ptr->page->setBestScore(score);
    d_ptr->settings->setValue(kBestScoreKey, score);
}

} // namespace Tetris
