#include "treemodel.hpp"

#include <utils/qtcassert.hpp>

#include <utility>

namespace Utils {

class TreeModel::TreeModelPrivate
{
public:
    TreeItem *root = nullptr;
    QStringList header;
    int columnCount = 1;
};

class TreeItem::TreeItemPrivate
{
public:
    TreeItem *parent = nullptr; // 不拥有
    TreeModel *model = nullptr; // 不拥有
    QList<TreeItem *> children; // 拥有
};

TreeItem::TreeItem()
    : d_ptr(std::make_unique<TreeItemPrivate>())
{}

TreeItem::~TreeItem()
{
    QTC_CHECK(d_ptr->parent == nullptr);
    QTC_CHECK(d_ptr->model == nullptr);
    clear();
}

auto TreeItem::data(int column, int role) const -> QVariant
{
    Q_UNUSED(column)
    Q_UNUSED(role)
    return {};
}

auto TreeItem::setData(int column, const QVariant &value, int role) -> bool
{
    Q_UNUSED(column)
    Q_UNUSED(value)
    Q_UNUSED(role)
    return false;
}

auto TreeItem::flags(int column) const -> Qt::ItemFlags
{
    Q_UNUSED(column)
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

auto TreeItem::parent() const -> TreeItem *
{
    return d_ptr->parent;
}

auto TreeItem::childCount() const -> int
{
    return int(d_ptr->children.size());
}

auto TreeItem::childAt(int index) const -> TreeItem *
{
    return index >= 0 && index < childCount() ? d_ptr->children.at(index) : nullptr;
}

auto TreeItem::indexOf(const TreeItem *item) const -> int
{
    for (int i = 0; i < childCount(); ++i) {
        if (d_ptr->children.at(i) == item)
            return i;
    }
    return -1;
}

auto TreeItem::model() const -> TreeModel *
{
    return d_ptr->model;
}

auto TreeItem::index() const -> QModelIndex
{
    return d_ptr->model ? d_ptr->model->indexForItem(this) : QModelIndex();
}

void TreeItem::appendChild(TreeItem *item)
{
    insertChild(childCount(), item);
}

void TreeItem::insertChild(int position, TreeItem *item)
{
    QTC_ASSERT(item, return);
    QTC_ASSERT(item->d_ptr->parent == nullptr && item->d_ptr->model == nullptr, return);
    QTC_ASSERT(position >= 0 && position <= childCount(), return);

    item->d_ptr->parent = this;
    if (!d_ptr->model) {
        d_ptr->children.insert(position, item);
        return;
    }

    const QModelIndex parentIndex = index();
    d_ptr->model->beginInsertRows(parentIndex, position, position);
    item->propagateModel(d_ptr->model);
    d_ptr->children.insert(position, item);
    d_ptr->model->endInsertRows();
}

void TreeItem::removeChildren()
{
    if (d_ptr->children.isEmpty())
        return;
    if (!d_ptr->model) {
        clear();
        return;
    }

    const QModelIndex parentIndex = index();
    d_ptr->model->beginRemoveRows(parentIndex, 0, childCount() - 1);
    clear();
    d_ptr->model->endRemoveRows();
}

void TreeItem::update()
{
    if (!d_ptr->model)
        return;

    const QModelIndex rowIndex = index();
    if (!rowIndex.isValid())
        return;
    emit d_ptr->model->dataChanged(
        rowIndex.sibling(rowIndex.row(), 0),
        rowIndex.sibling(rowIndex.row(), d_ptr->model->d_ptr->columnCount - 1));
}

void TreeItem::updateColumn(int column)
{
    if (!d_ptr->model)
        return;

    const QModelIndex rowIndex = index();
    if (!rowIndex.isValid())
        return;
    const QModelIndex cellIndex = rowIndex.sibling(rowIndex.row(), column);
    emit d_ptr->model->dataChanged(cellIndex, cellIndex);
}

auto TreeItem::findAnyChild(const std::function<bool(TreeItem *)> &predicate) const -> TreeItem *
{
    for (TreeItem *item : d_ptr->children) {
        if (predicate(item))
            return item;
        if (TreeItem *found = item->findAnyChild(predicate))
            return found;
    }
    return nullptr;
}

void TreeItem::clear()
{
    while (!d_ptr->children.isEmpty()) {
        TreeItem *item = d_ptr->children.takeLast();
        item->d_ptr->model = nullptr;
        item->d_ptr->parent = nullptr;
        delete item;
    }
}

void TreeItem::propagateModel(TreeModel *model)
{
    if (!model || d_ptr->model == model)
        return;
    QTC_ASSERT(d_ptr->model == nullptr, return);

    d_ptr->model = model;
    for (TreeItem *child : std::as_const(d_ptr->children))
        child->propagateModel(model);
}

TreeModel::TreeModel(QObject *parent)
    : QAbstractItemModel(parent)
    , d_ptr(std::make_unique<TreeModelPrivate>())
{
    d_ptr->root = new TreeItem;
    d_ptr->root->d_ptr->model = this;
}

TreeModel::~TreeModel()
{
    d_ptr->root->d_ptr->model = nullptr;
    delete d_ptr->root;
}

void TreeModel::setHeader(const QStringList &titles)
{
    d_ptr->header = titles;
    d_ptr->columnCount = qMax(1, int(titles.size()));
    emit headerDataChanged(Qt::Horizontal, 0, d_ptr->columnCount - 1);
}

void TreeModel::clear()
{
    d_ptr->root->removeChildren();
}

auto TreeModel::rootItem() const -> TreeItem *
{
    return d_ptr->root;
}

auto TreeModel::itemForIndex(const QModelIndex &index) const -> TreeItem *
{
    return index.isValid() ? static_cast<TreeItem *>(index.internalPointer()) : d_ptr->root;
}

auto TreeModel::indexForItem(const TreeItem *item) const -> QModelIndex
{
    if (!item || item == d_ptr->root)
        return {};

    TreeItem *parentItem = item->parent();
    if (!parentItem)
        return {};
    return createIndex(parentItem->indexOf(item), 0, const_cast<TreeItem *>(item));
}

auto TreeModel::rowCount(const QModelIndex &parent) const -> int
{
    if (parent.column() > 0)
        return 0;

    const TreeItem *item = parent.isValid() ? itemForIndex(parent) : d_ptr->root;
    return item ? item->childCount() : 0;
}

auto TreeModel::columnCount(const QModelIndex &parent) const -> int
{
    return parent.column() > 0 ? 0 : d_ptr->columnCount;
}

auto TreeModel::data(const QModelIndex &index, int role) const -> QVariant
{
    const TreeItem *item = itemForIndex(index);
    return item ? item->data(index.column(), role) : QVariant();
}

auto TreeModel::setData(const QModelIndex &index, const QVariant &value, int role) -> bool
{
    TreeItem *item = itemForIndex(index);
    if (!item || !item->setData(index.column(), value, role))
        return false;

    emit dataChanged(index, index);
    return true;
}

auto TreeModel::flags(const QModelIndex &index) const -> Qt::ItemFlags
{
    if (!index.isValid())
        return {};

    const TreeItem *item = itemForIndex(index);
    return item ? item->flags(index.column()) : Qt::NoItemFlags;
}

auto TreeModel::index(int row, int column, const QModelIndex &parent) const -> QModelIndex
{
    if (!hasIndex(row, column, parent))
        return {};

    const TreeItem *parentItem = parent.isValid() ? itemForIndex(parent) : d_ptr->root;
    if (!parentItem)
        return {};

    TreeItem *item = parentItem->childAt(row);
    return item ? createIndex(row, column, item) : QModelIndex();
}

auto TreeModel::parent(const QModelIndex &index) const -> QModelIndex
{
    if (!index.isValid())
        return {};

    const TreeItem *item = itemForIndex(index);
    if (!item)
        return {};

    TreeItem *parentItem = item->parent();
    if (!parentItem || parentItem == d_ptr->root)
        return {};

    TreeItem *grandParent = parentItem->parent();
    if (!grandParent)
        return {};
    return createIndex(grandParent->indexOf(parentItem), 0, parentItem);
}

auto TreeModel::headerData(int section, Qt::Orientation orientation, int role) const -> QVariant
{
    if (orientation != Qt::Horizontal || section < 0 || section >= d_ptr->header.size())
        return {};
    if (role == Qt::DisplayRole)
        return d_ptr->header.at(section);
    return {};
}

} // namespace Utils
