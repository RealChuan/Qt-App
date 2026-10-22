#pragma once

#include "widgets_global.hpp"

#include <QAbstractItemModel>
#include <QList>
#include <QStringList>
#include <QVariant>

#include <functional>
#include <memory>

namespace Utils {

class TreeModel;

// 树节点只描述自身数据和子节点；行插入/删除的模型通知一律由 TreeModel 发出。
// 节点由父节点（或 TreeModel 的根）拥有，派生类只需重写 data()/setData()/flags()。
class WIDGETS_EXPORT TreeItem
{
public:
    TreeItem();
    virtual ~TreeItem();

    Q_DISABLE_COPY_MOVE(TreeItem)

    [[nodiscard]] virtual auto data(int column, int role) const -> QVariant;
    virtual auto setData(int column, const QVariant &value, int role) -> bool;
    [[nodiscard]] virtual auto flags(int column) const -> Qt::ItemFlags;

    [[nodiscard]] auto parent() const -> TreeItem *;
    [[nodiscard]] auto childCount() const -> int;
    [[nodiscard]] auto childAt(int index) const -> TreeItem *;
    [[nodiscard]] auto indexOf(const TreeItem *item) const -> int;
    [[nodiscard]] auto model() const -> TreeModel *;
    [[nodiscard]] auto index() const -> QModelIndex;

    void appendChild(TreeItem *item);
    void insertChild(int position, TreeItem *item);
    void removeChildren();

    // 通知视图本行数据已变；update() 覆盖所有列。
    void update();
    void updateColumn(int column);

    [[nodiscard]] auto findAnyChild(const std::function<bool(TreeItem *)> &predicate) const
        -> TreeItem *;

private:
    void clear();
    void propagateModel(TreeModel *model);

    class TreeItemPrivate;
    std::unique_ptr<TreeItemPrivate> d_ptr;

    friend class TreeModel;
};

// 通用多级模型：根节点不可见，根节点的孩子就是视图的顶层行。
class WIDGETS_EXPORT TreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit TreeModel(QObject *parent = nullptr);
    ~TreeModel() override;

    void setHeader(const QStringList &titles);
    void clear();

    [[nodiscard]] auto rootItem() const -> TreeItem *;
    [[nodiscard]] auto itemForIndex(const QModelIndex &index) const -> TreeItem *;
    [[nodiscard]] auto indexForItem(const TreeItem *item) const -> QModelIndex;

    [[nodiscard]] auto rowCount(const QModelIndex &parent = {}) const -> int override;
    [[nodiscard]] auto columnCount(const QModelIndex &parent = {}) const -> int override;
    [[nodiscard]] auto data(const QModelIndex &index, int role) const -> QVariant override;
    auto setData(const QModelIndex &index, const QVariant &value, int role) -> bool override;
    [[nodiscard]] auto flags(const QModelIndex &index) const -> Qt::ItemFlags override;
    [[nodiscard]] auto index(int row, int column, const QModelIndex &parent = {}) const
        -> QModelIndex override;
    [[nodiscard]] auto parent(const QModelIndex &index) const -> QModelIndex override;
    [[nodiscard]] auto headerData(
        int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const
        -> QVariant override;

private:
    friend class TreeItem;

    class TreeModelPrivate;
    std::unique_ptr<TreeModelPrivate> d_ptr;
};

} // namespace Utils
