#include "helloplugin.hpp"

#include "hellowidget.hpp"

#include <extensionsystem/plugincontext.hpp>
#include <shell/command.hpp>
#include <shell/registry.hpp>

#include <QSettings>

namespace Hello {

namespace {
constexpr auto kCountKey = "count";
}

// 命令面板里的一条命令。命令对象由插件持有，生命周期与插件一致。
class IncrementCommand final : public Shell::ICommand
{
public:
    explicit IncrementCommand(HelloPlugin *plugin)
        : m_plugin(plugin)
    {}

    auto id() const -> QString override { return QStringLiteral("helloplugin.increment"); }
    auto title() const -> QString override { return QStringLiteral("Hello: increment counter"); }
    auto subtitle() const -> QString override
    {
        return QStringLiteral("helloplugin.page");
    }
    auto shortcut() const -> QKeySequence override
    {
        return QKeySequence(QStringLiteral("Ctrl+Alt+H"));
    }
    void execute() override { m_plugin->increment(); }

private:
    HelloPlugin *m_plugin;
};

HelloPlugin::HelloPlugin() = default;

HelloPlugin::~HelloPlugin() = default;

auto HelloPlugin::initialize(const ExtensionSystem::PluginContext &context) -> Utils::Result<>
{
    m_settings = context.settings();
    m_widget = new HelloWidget;
    m_widget->setCount(m_settings->value(kCountKey, 0).toInt());
    m_command = std::make_unique<IncrementCommand>(this);

    connect(m_widget, &HelloWidget::countChanged, this, [this](int count) {
        m_settings->setValue(kCountKey, count);
    });

    return Utils::ResultOk;
}

void HelloPlugin::extensionsInitialized()
{
    auto *registry = Shell::Registry::instance();
    registry->addPage(m_widget);
    registry->addCommand(m_command.get());
}

void HelloPlugin::increment()
{
    m_widget->increment();
}

} // namespace Hello
