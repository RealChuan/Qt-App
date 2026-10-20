#include "pluginpage.hpp"

#include <extensionsystem/pluginmanager.hpp>
#include <extensionsystem/pluginspec.hpp>
#include <shell/host.hpp>

#include <QHeaderView>
#include <QLabel>
#include <QTableWidget>
#include <QVBoxLayout>

namespace Core {

namespace {
constexpr int kEnabledColumn = 4;
}

class PluginPage::PluginPagePrivate
{
public:
    explicit PluginPagePrivate(PluginPage *q)
        : q_ptr(q)
    {}

    void refresh();
    void updateDetails();
    void onItemChanged(QTableWidgetItem *item);

    PluginPage *q_ptr = nullptr;
    QTableWidget *table = nullptr;
    QLabel *details = nullptr;
    bool updating = false;
};

void PluginPage::PluginPagePrivate::refresh()
{
    updating = true;
    const QList<ExtensionSystem::PluginSpec *> plugins = ExtensionSystem::PluginManager::plugins();
    table->setRowCount(plugins.size());

    for (int row = 0; row < plugins.size(); ++row) {
        ExtensionSystem::PluginSpec *spec = plugins.at(row);

        auto *name = new QTableWidgetItem(spec->displayName());
        name->setData(Qt::UserRole, QVariant::fromValue<quintptr>(reinterpret_cast<quintptr>(spec)));
        table->setItem(row, 0, name);
        table->setItem(row, 1, new QTableWidgetItem(spec->version()));
        table->setItem(row, 2, new QTableWidgetItem(spec->stateText()));
        table->setItem(row, 3, new QTableWidgetItem(QString::number(spec->loadTimeMs())));

        auto *enabled = new QTableWidgetItem;
        if (spec->isRequired()) {
            enabled->setText(QObject::tr("required"));
            enabled->setFlags(Qt::ItemIsEnabled);
        } else {
            enabled->setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
            enabled->setCheckState(spec->isEnabledBySettings() ? Qt::Checked : Qt::Unchecked);
        }
        table->setItem(row, kEnabledColumn, enabled);
    }

    updating = false;
    updateDetails();
}

void PluginPage::PluginPagePrivate::updateDetails()
{
    const int row = table->currentRow();
    auto *item = row >= 0 ? table->item(row, 0) : nullptr;
    if (!item) {
        details->clear();
        return;
    }

    auto *spec = reinterpret_cast<ExtensionSystem::PluginSpec *>(
        item->data(Qt::UserRole).value<quintptr>());
    if (!spec) {
        details->clear();
        return;
    }

    QStringList text;
    text << QStringLiteral("<b>%1</b> (%2 %3)").arg(spec->id(), spec->vendor(), spec->version());
    if (!spec->longDescription().isEmpty())
        text << spec->longDescription();
    if (!spec->errorString().isEmpty())
        text << QStringLiteral("<span style=\"color:#c0392b\">%1</span>").arg(spec->errorString());

    QStringList dependencies;
    for (const ExtensionSystem::PluginDependency &dependency : spec->dependencies()) {
        dependencies << QStringLiteral("%1 (%2)").arg(
            dependency.id,
            dependency.type == ExtensionSystem::PluginDependency::Type::Required
                ? QObject::tr("required")
                : QObject::tr("optional"));
    }
    if (!dependencies.isEmpty())
        text << QObject::tr("Dependencies: %1").arg(dependencies.join(QStringLiteral(", ")));

    details->setText(text.join(QStringLiteral("<br>")));
}

void PluginPage::PluginPagePrivate::onItemChanged(QTableWidgetItem *item)
{
    if (updating || !item || item->column() != kEnabledColumn)
        return;

    auto *nameItem = table->item(item->row(), 0);
    if (!nameItem)
        return;

    auto *spec = reinterpret_cast<ExtensionSystem::PluginSpec *>(
        nameItem->data(Qt::UserRole).value<quintptr>());
    if (!spec || spec->isRequired())
        return;

    spec->setEnabledBySettings(item->checkState() == Qt::Checked);
    if (Shell::host())
        Shell::host()->askForRestart(QObject::tr("Plugin changes take effect after a restart."));
}

PluginPage::PluginPage(QWidget *parent)
    : QWidget(parent)
    , d_ptr(std::make_unique<PluginPagePrivate>(this))
{
    d_ptr->table = new QTableWidget(this);
    d_ptr->table->setColumnCount(5);
    d_ptr->table->setHorizontalHeaderLabels({tr("Plugin"),
                                             tr("Version"),
                                             tr("State"),
                                             tr("Load (ms)"),
                                             tr("Enabled")});
    d_ptr->table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    d_ptr->table->verticalHeader()->hide();
    d_ptr->table->setSelectionBehavior(QAbstractItemView::SelectRows);
    d_ptr->table->setSelectionMode(QAbstractItemView::SingleSelection);
    d_ptr->table->setEditTriggers(QAbstractItemView::NoEditTriggers);

    d_ptr->details = new QLabel(this);
    d_ptr->details->setWordWrap(true);
    d_ptr->details->setTextInteractionFlags(Qt::TextSelectableByMouse);

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(d_ptr->table, 1);
    layout->addWidget(d_ptr->details);

    connect(d_ptr->table, &QTableWidget::itemChanged, this, [this](QTableWidgetItem *item) {
        d_ptr->onItemChanged(item);
    });
    connect(d_ptr->table, &QTableWidget::itemSelectionChanged, this, [this] {
        d_ptr->updateDetails();
    });
    connect(ExtensionSystem::PluginManager::instance(),
            &ExtensionSystem::PluginManager::pluginsChanged,
            this,
            [this] { d_ptr->refresh(); });

    d_ptr->refresh();
}

PluginPage::~PluginPage() = default;

} // namespace Core
