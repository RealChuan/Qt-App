#include "helloplugin.hpp"

#include "hellopage.hpp"

#include <extensionsystem/plugincontext.hpp>
#include <shell/functioncommand.hpp>
#include <shell/registry.hpp>

#include <QSettings>

namespace Hello {

namespace {
constexpr QLatin1StringView kCountKey("count");
}

class HelloPlugin::HelloPluginPrivate
{
public:
    QSettings *settings = nullptr;
    HelloPage *page = nullptr;
    std::unique_ptr<Shell::FunctionCommand> command;
};

HelloPlugin::HelloPlugin()
    : d_ptr(std::make_unique<HelloPluginPrivate>())
{}

HelloPlugin::~HelloPlugin() = default;

auto HelloPlugin::initialize(const ExtensionSystem::PluginContext &context) -> Utils::Result<>
{
    d_ptr->settings = context.settings();
    d_ptr->page = new HelloPage;
    d_ptr->page->setCount(d_ptr->settings->value(kCountKey, 0).toInt());

    // 命令对象由插件持有，生命周期与插件一致。
    d_ptr->command = std::make_unique<Shell::FunctionCommand>(
        u"helloplugin.increment"_s,
        tr("Hello: increment counter"),
        std::bind_front(&HelloPlugin::increment, this));
    d_ptr->command->withShortcut(QKeySequence(u"Ctrl+Alt+H"_s));

    connect(d_ptr->page, &HelloPage::countChanged, this, &HelloPlugin::onCountChanged);

    return Utils::ResultOk;
}

void HelloPlugin::extensionsInitialized()
{
    auto *registry = Shell::Registry::instance();
    registry->addPage(d_ptr->page);
    registry->addCommand(d_ptr->command.get());
}

void HelloPlugin::onCountChanged(int count)
{
    d_ptr->settings->setValue(kCountKey, count);
}

void HelloPlugin::increment()
{
    d_ptr->page->increment();
}

} // namespace Hello
