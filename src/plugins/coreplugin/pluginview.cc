#include "pluginview.hpp"

#include <extensionsystem/pluginmanager.hpp>
#include <extensionsystem/pluginspec.hpp>
#include <utils/algorithm.hpp>
#include <widgets/roundedrowtreeview.hpp>
#include <widgets/treemodel.hpp>

#include <QApplication>
#include <QColor>
#include <QEvent>
#include <QHeaderView>
#include <QIcon>
#include <QMap>
#include <QSortFilterProxyModel>
#include <QStyle>
#include <QStyleOptionViewItem>
#include <QStyledItemDelegate>
#include <QVBoxLayout>

namespace Core {

namespace {

enum Column { NameColumn, LoadColumn, VersionColumn, StateColumn, LoadTimeColumn, ColumnCount };

// 排序用的角色：组按"必需组优先 + 组名"，插件按显示名。
constexpr int kSortRole = Qt::UserRole + 1;
// 过滤用的角色：除显示名外还能按 Id、分类搜索。
constexpr int kFilterRole = Qt::UserRole + 2;

// 默认列宽。名称列是树列，还要扣掉两级缩进和错误图标，所以给得比文本宽；
// 其余四列按表头文字量，窄了表头自己就被省略了。
// 五列合计超过插件页默认的列表宽度，横向滚动条是预期行为——总比把名称挤成一条缝好。
constexpr int kNameColumnWidth = 220;
constexpr int kLoadColumnWidth = 56;
constexpr int kVersionColumnWidth = 76;
constexpr int kStateColumnWidth = 92;
constexpr int kLoadTimeColumnWidth = 112;

auto errorIcon() -> QIcon
{
    return QApplication::style()->standardIcon(QStyle::SP_MessageBoxCritical);
}

} // namespace

using ExtensionSystem::PluginSpec;

class PluginItem final : public Widgets::TreeItem
{
public:
    explicit PluginItem(PluginSpec *spec)
        : m_spec(spec)
    {}

    [[nodiscard]] auto data(int column, int role) const -> QVariant override
    {
        if (role == kFilterRole) {
            if (column != NameColumn)
                return {};
            return QStringLiteral("%1 %2 %3")
                .arg(m_spec->id(), m_spec->displayName(), m_spec->category());
        }

        switch (column) {
        case NameColumn:
            if (role == Qt::DisplayRole || role == kSortRole)
                return m_spec->displayName();
            if (role == Qt::ToolTipRole) {
                return QStringLiteral("%1\n%2").arg(m_spec->id(), m_spec->filePath().toUserOutput());
            }
            if (role == Qt::DecorationRole && m_spec->hasError())
                return errorIcon();
            break;
        case LoadColumn:
            if (role == Qt::CheckStateRole) {
                if (m_spec->isRequired())
                    return Qt::Checked;
                return m_spec->isEnabledBySettings() ? Qt::Checked : Qt::Unchecked;
            }
            if (role == Qt::ToolTipRole) {
                return m_spec->isRequired() ? PluginView::tr("Required plugin, cannot be disabled.")
                                            : PluginView::tr("Load on startup");
            }
            break;
        case VersionColumn:
            if (role == Qt::DisplayRole || role == kSortRole)
                return m_spec->version();
            break;
        case StateColumn:
            if (role == Qt::DisplayRole || role == kSortRole)
                return m_spec->stateText();
            if (role == Qt::ToolTipRole && m_spec->hasError())
                return m_spec->errorString();
            if (role == Qt::ForegroundRole && m_spec->hasError())
                return QColor(0xc0, 0x39, 0x2b);
            break;
        case LoadTimeColumn:
            if (role == Qt::DisplayRole)
                return m_spec->loadTimeMs() > 0 ? QString::number(m_spec->loadTimeMs())
                                                : QStringLiteral("-");
            break;
        default:
            break;
        }
        return {};
    }

    auto setData(int column, const QVariant &value, int role) -> bool override
    {
        if (column != LoadColumn || role != Qt::CheckStateRole || m_spec->isRequired())
            return false;

        const bool enabled = value.toInt() == Qt::Checked;
        if (enabled == m_spec->isEnabledBySettings())
            return false;

        m_spec->setEnabledBySettings(enabled);
        updateColumn(LoadColumn);
        // 子项变化后组的三态勾选要跟着刷新。
        if (Widgets::TreeItem *group = parent())
            group->updateColumn(LoadColumn);
        return true;
    }

    [[nodiscard]] auto flags(int column) const -> Qt::ItemFlags override
    {
        Qt::ItemFlags result = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
        if (column == LoadColumn && !m_spec->isRequired())
            result |= Qt::ItemIsUserCheckable;
        return result;
    }

    [[nodiscard]] auto spec() const -> PluginSpec * { return m_spec; }

private:
    PluginSpec *m_spec = nullptr;
};

class CollectionItem final : public Widgets::TreeItem
{
public:
    CollectionItem(const QString &title, const QList<PluginSpec *> &plugins)
        : m_title(title)
    {
        for (PluginSpec *spec : plugins)
            appendChild(new PluginItem(spec));
    }

    void setSortKey(const QString &key) { m_sortKey = key; }

    [[nodiscard]] auto data(int column, int role) const -> QVariant override
    {
        if (role == kFilterRole)
            return column == NameColumn ? QVariant(m_title) : QVariant();
        if (column == NameColumn) {
            if (role == Qt::DisplayRole)
                return m_title;
            if (role == kSortRole)
                return m_sortKey;
        }
        if (column == LoadColumn) {
            if (role == Qt::CheckStateRole)
                return checkState();
            if (role == Qt::ToolTipRole)
                return PluginView::tr("Load plugins of this group on startup");
        }
        return {};
    }

    auto setData(int column, const QVariant &value, int role) -> bool override
    {
        if (column != LoadColumn || role != Qt::CheckStateRole)
            return false;

        const bool enabled = value.toInt() == Qt::Checked;
        bool changed = false;
        for (int i = 0; i < childCount(); ++i) {
            auto *item = dynamic_cast<PluginItem *>(childAt(i));
            if (!item)
                continue;

            PluginSpec *spec = item->spec();
            if (spec->isRequired() || spec->isEnabledBySettings() == enabled)
                continue;

            spec->setEnabledBySettings(enabled);
            item->updateColumn(LoadColumn);
            changed = true;
        }

        if (changed)
            updateColumn(LoadColumn);
        return changed;
    }

    [[nodiscard]] auto flags(int column) const -> Qt::ItemFlags override
    {
        Qt::ItemFlags result = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
        if (column == LoadColumn && hasCheckableChild())
            result |= Qt::ItemIsUserCheckable;
        return result;
    }

private:
    [[nodiscard]] auto checkState() const -> Qt::CheckState
    {
        int loaded = 0;
        for (int i = 0; i < childCount(); ++i) {
            const auto *item = dynamic_cast<const PluginItem *>(childAt(i));
            if (!item)
                continue;
            if (item->spec()->isEnabledBySettings())
                ++loaded;
        }

        if (loaded == 0)
            return Qt::Unchecked;
        return loaded == childCount() ? Qt::Checked : Qt::PartiallyChecked;
    }

    [[nodiscard]] auto hasCheckableChild() const -> bool
    {
        for (int i = 0; i < childCount(); ++i) {
            const auto *item = dynamic_cast<const PluginItem *>(childAt(i));
            if (item && !item->spec()->isRequired())
                return true;
        }
        return false;
    }

    QString m_title;
    QString m_sortKey;
};

class PluginModel final : public Widgets::TreeModel
{
public:
    explicit PluginModel(QObject *parent = nullptr)
        : TreeModel(parent)
    {
        // 列数必须在 setModel() 前确定：QHeaderView 只在接管模型时读一次 columnCount，
        // 之后 setHeader() 只更新表头文字，不会重建列。标题由 retranslateUi() 补。
        setHeader(QStringList(ColumnCount, QString()));
    }

    // 必需插件单独成组并置顶，其余按 Category 分组，组名与组内插件都按名称排序。
    void rebuild(
        const QList<PluginSpec *> &plugins, const QString &requiredTitle, const QString &otherTitle)
    {
        clear();

        QList<PluginSpec *> required;
        QMap<QString, QList<PluginSpec *>> groups;
        for (PluginSpec *spec : plugins) {
            if (spec->isRequired()) {
                required.append(spec);
                continue;
            }
            const QString category = spec->category().isEmpty() ? otherTitle : spec->category();
            groups[category].append(spec);
        }

        if (!required.isEmpty())
            appendGroup(requiredTitle, required, QStringLiteral("0"));

        for (auto it = groups.cbegin(); it != groups.cend(); ++it)
            appendGroup(it.key(), it.value(), QStringLiteral("1"));
    }

    [[nodiscard]] auto itemForSpec(PluginSpec *spec) const -> PluginItem *
    {
        Widgets::TreeItem *found = rootItem()->findAnyChild([spec](Widgets::TreeItem *item) {
            const auto *pluginItem = dynamic_cast<const PluginItem *>(item);
            return pluginItem && pluginItem->spec() == spec;
        });
        return dynamic_cast<PluginItem *>(found);
    }

private:
    void appendGroup(
        const QString &title, const QList<PluginSpec *> &plugins, const QString &sortPrefix)
    {
        QList<PluginSpec *> ordered = plugins;
        Utils::sort(ordered, [](const PluginSpec *lhs, const PluginSpec *rhs) {
            return lhs->displayName().compare(rhs->displayName(), Qt::CaseInsensitive) < 0;
        });

        auto *group = new CollectionItem(title, ordered);
        group->setSortKey(sortPrefix + title);
        rootItem()->appendChild(group);
    }
};

// 必需插件的 Load 列不可勾选，但 Qt 不会因此把勾选框画成禁用态：
// “能不能勾”没有对应的 QSS 伪状态，只能由委托把 State_Enabled 关掉，
// 灰化后的图标仍然来自 base.css 的 ::indicator:checked:disabled。
class PluginItemDelegate final : public QStyledItemDelegate
{
public:
    using QStyledItemDelegate::QStyledItemDelegate;

protected:
    void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const override
    {
        QStyledItemDelegate::initStyleOption(option, index);
        if (index.column() == LoadColumn && !(index.flags() & Qt::ItemIsUserCheckable))
            option->state &= ~QStyle::State_Enabled;
    }
};

class PluginView::PluginViewPrivate
{
public:
    explicit PluginViewPrivate(PluginView *q)
        : q_ptr(q)
    {}

    void setupUi();
    void rebuild(const QString &requiredTitle, const QString &otherTitle);
    [[nodiscard]] auto specForIndex(const QModelIndex &index) const -> PluginSpec *;
    void selectPlugin(PluginSpec *spec);

    PluginView *q_ptr = nullptr;
    Widgets::RoundedRowTreeView *view = nullptr;
    PluginModel *model = nullptr;
    QSortFilterProxyModel *proxy = nullptr;
    QString requiredGroupTitle;
    QString otherGroupTitle;
    bool changePending = false;
    bool initialScrollDone = false;
};

void PluginView::PluginViewPrivate::setupUi()
{
    model = new PluginModel(q_ptr);

    proxy = new QSortFilterProxyModel(q_ptr);
    proxy->setSourceModel(model);
    proxy->setSortRole(kSortRole);
    proxy->setSortLocaleAware(true);
    proxy->setFilterRole(kFilterRole);
    proxy->setFilterKeyColumn(-1);
    proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    proxy->setRecursiveFilteringEnabled(true);

    view = new Widgets::RoundedRowTreeView(q_ptr);
    view->setObjectName("PluginTree");
    view->setItemDelegate(new PluginItemDelegate(view));
    view->setModel(proxy);
    view->setAlternatingRowColors(false);
    view->setUniformRowHeights(true);
    view->setAllColumnsShowFocus(true);
    view->setSelectionMode(QAbstractItemView::SingleSelection);
    view->setSelectionBehavior(QAbstractItemView::SelectRows);
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    view->setSortingEnabled(true);
    view->sortByColumn(NameColumn, Qt::AscendingOrder);

    // 全部 Interactive：Stretch / ResizeToContents 都不接受鼠标拖拽，列宽只能由
    // 代码定死，用户碰到截断时没有任何办法。放不下就交给横向滚动条；
    // 末列用 stretchLastSection 吃掉多余空间，右侧不会留一条空白。
    QHeaderView *header = view->header();
    header->setSectionResizeMode(QHeaderView::Interactive);
    header->setStretchLastSection(true);

    view->setColumnWidth(NameColumn, kNameColumnWidth);
    view->setColumnWidth(LoadColumn, kLoadColumnWidth);
    view->setColumnWidth(VersionColumn, kVersionColumnWidth);
    view->setColumnWidth(StateColumn, kStateColumnWidth);
    view->setColumnWidth(LoadTimeColumn, kLoadTimeColumnWidth);

    auto *layout = new QVBoxLayout(q_ptr);
    layout->setContentsMargins({});
    layout->setSpacing(0);
    layout->addWidget(view);
}

void PluginView::PluginViewPrivate::rebuild(const QString &requiredTitle, const QString &otherTitle)
{
    PluginSpec *current = q_ptr->currentPlugin();
    model->rebuild(ExtensionSystem::PluginManager::plugins(), requiredTitle, otherTitle);
    view->expandAll();

    if (current) {
        selectPlugin(current);
        return;
    }

    // 首次打开时选中第一个插件，右侧详情不至于空白。
    Widgets::TreeItem *firstGroup = model->rootItem()->childAt(0);
    const auto *firstItem = firstGroup ? dynamic_cast<PluginItem *>(firstGroup->childAt(0))
                                       : nullptr;
    if (firstItem)
        selectPlugin(firstItem->spec());
}

auto PluginView::PluginViewPrivate::specForIndex(const QModelIndex &index) const -> PluginSpec *
{
    if (!index.isValid())
        return nullptr;

    auto *item = dynamic_cast<PluginItem *>(model->itemForIndex(proxy->mapToSource(index)));
    return item ? item->spec() : nullptr;
}

void PluginView::PluginViewPrivate::selectPlugin(PluginSpec *spec)
{
    PluginItem *item = spec ? model->itemForSpec(spec) : nullptr;
    if (!item)
        return;

    const QModelIndex index = proxy->mapFromSource(item->index());
    if (!index.isValid())
        return;

    view->setCurrentIndex(index);
    view->scrollTo(index);
}

PluginView::PluginView(QWidget *parent)
    : QWidget(parent)
    , d_ptr(std::make_unique<PluginViewPrivate>(this))
{
    d_ptr->setupUi();
    retranslateUi();
    setupConnections();
    rebuild();
}

PluginView::~PluginView() = default;

auto PluginView::currentPlugin() const -> PluginSpec *
{
    return d_ptr->specForIndex(d_ptr->view->currentIndex());
}

void PluginView::setFilter(const QString &text)
{
    d_ptr->proxy->setFilterFixedString(text);
    d_ptr->view->expandAll();
}

void PluginView::rebuild()
{
    d_ptr->rebuild(d_ptr->requiredGroupTitle, d_ptr->otherGroupTitle);
}

void PluginView::retranslateUi()
{
    d_ptr->requiredGroupTitle = tr("Required");
    d_ptr->otherGroupTitle = tr("Other");
    d_ptr->model->setHeader(
        {tr("Name"), tr("Load"), tr("Version"), tr("State"), tr("Load time (ms)")});
}

void PluginView::setupConnections()
{
    connect(
        d_ptr->view->selectionModel(),
        &QItemSelectionModel::currentChanged,
        this,
        &PluginView::onCurrentChanged);
    connect(d_ptr->model, &QAbstractItemModel::dataChanged, this, &PluginView::onDataChanged);
}

void PluginView::onCurrentChanged(const QModelIndex &current)
{
    emit currentPluginChanged(d_ptr->specForIndex(current));
}

void PluginView::onDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    if (topLeft.column() > LoadColumn || bottomRight.column() < LoadColumn)
        return;

    // 组勾选会连带着改一串子项，合并成一次通知，避免弹出多次重启对话框。
    if (d_ptr->changePending)
        return;

    d_ptr->changePending = true;
    QMetaObject::invokeMethod(this, &PluginView::notifyPluginsChanged, Qt::QueuedConnection);
}

void PluginView::notifyPluginsChanged()
{
    d_ptr->changePending = false;
    emit pluginsChanged();
}

void PluginView::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);

    // 构造期视图还没有尺寸，selectPlugin() 里的 scrollTo() 会按 0 高度计算，
    // 等页面真正显示时滚动条已经停在下半截。首次显示时布局已完成，回顶一次；
    // 之后再切回本页就不干预了，保留用户自己的滚动位置。
    if (!d_ptr->initialScrollDone) {
        d_ptr->initialScrollDone = true;
        d_ptr->view->scrollToTop();
    }
}

void PluginView::changeEvent(QEvent *event)
{
    QWidget::changeEvent(event);
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
        rebuild();
    }
}

} // namespace Core
