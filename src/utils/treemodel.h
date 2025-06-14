// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#pragma once

#include "utils_global.h"

#include "indexedcontainerproxyconstiterator.h"

#include <utils/qtcassert.h>

#include <QSortFilterProxyModel>

#include <functional>

namespace Utils {

class BaseTreeModel;

class UTILS_EXPORT TreeItem
{
public:
    TreeItem();
    virtual ~TreeItem();

    virtual QVariant data(int column, int role) const;
    virtual bool setData(int column, const QVariant &data, int role);
    virtual Qt::ItemFlags flags(int column) const;

    virtual bool hasChildren() const;
    virtual bool canFetchMore() const;
    virtual void fetchMore() {}

    TreeItem *parent() const { return m_parent; }

    void prependChild(TreeItem *item);
    void appendChild(TreeItem *item);
    void insertChild(int pos, TreeItem *item);
    void insertOrderedChild(TreeItem *item,
                            const std::function<bool(const TreeItem *, const TreeItem *)> &cmp);

    void removeChildAt(int pos);
    void removeChildren();
    void removeChildrenSilently();
    void sortChildren(const std::function<bool(const TreeItem *, const TreeItem *)> &cmp);
    void update();
    void updateAll();
    void updateColumn(int column);
    void expand();
    void collapse();
    TreeItem *firstChild() const;
    TreeItem *lastChild() const;
    int level() const;

    using const_iterator = QList<TreeItem *>::const_iterator;
    using value_type = TreeItem *;
    int childCount() const { return m_children.size(); }
    int indexInParent() const;
    TreeItem *childAt(int index) const;
    int indexOf(const TreeItem *item) const;
    const_iterator begin() const { return m_children.begin(); }
    const_iterator end() const { return m_children.end(); }
    QModelIndex index() const;
    QAbstractItemModel *model() const;

    void forSelectedChildren(const std::function<bool(TreeItem *)> &pred) const;
    void forAllChildren(const std::function<void(TreeItem *)> &pred) const;
    TreeItem *findAnyChild(const std::function<bool(TreeItem *)> &pred) const;
    // like findAnyChild() but processes children in exact reverse order
    // (bottom to top, most inner children first)
    TreeItem *reverseFindAnyChild(const std::function<bool(TreeItem *)> &pred) const;

    // Levels are 1-based: Child at Level 1 is an immediate child.
    void forChildrenAtLevel(int level, const std::function<void(TreeItem *)> &pred) const;
    TreeItem *findChildAtLevel(int level, const std::function<bool(TreeItem *)> &pred) const;

private:
    TreeItem(const TreeItem &) = delete;
    void operator=(const TreeItem &) = delete;

    void clear();
    void removeItemAt(int pos);
    void propagateModel(BaseTreeModel *m);
    void updateChildrenRecursively();

    TreeItem *m_parent = nullptr;     // Not owned.
    BaseTreeModel *m_model = nullptr; // Not owned.
    QList<TreeItem *> m_children;     // Owned.
    friend class BaseTreeModel;
};

// A TreeItem with children all of the same type.
template<class ChildType, class ParentType = TreeItem>
class TypedTreeItem : public TreeItem
{
public:
    ChildType *childAt(int index) const { return childItemCast(TreeItem::childAt(index)); }

    void sortChildren(const std::function<bool(const ChildType *, const ChildType *)> &lessThan)
    {
        return TreeItem::sortChildren([lessThan, this](const TreeItem *a, const TreeItem *b) {
            return lessThan(childItemCast(a), childItemCast(b));
        });
    }

    using value_type = ChildType *;
    using const_iterator = IndexedContainerProxyConstIterator<TypedTreeItem>;
    using size_type = int;

    ChildType *operator[](int index) const { return childAt(index); }
    int size() const { return childCount(); }
    const_iterator begin() const { return const_iterator(*this, 0); }
    const_iterator end() const { return const_iterator(*this, size()); }

    template<typename Predicate>
    void forAllChildren(const Predicate &pred) const
    {
        const auto pred0 = [pred, this](TreeItem *treeItem) { pred(childItemCast(treeItem)); };
        TreeItem::forAllChildren(pred0);
    }

    template<typename Predicate>
    void forFirstLevelChildren(Predicate pred) const
    {
        const auto pred0 = [pred, this](TreeItem *treeItem) { pred(childItemCast(treeItem)); };
        TreeItem::forChildrenAtLevel(1, pred0);
    }

    template<typename Predicate>
    ChildType *findFirstLevelChild(Predicate pred) const
    {
        const auto pred0 = [pred, this](TreeItem *treeItem) {
            return pred(childItemCast(treeItem));
        };
        return static_cast<ChildType *>(TreeItem::findChildAtLevel(1, pred0));
    }

    ParentType *parent() const { return parentItemCast(TreeItem::parent()); }

    void insertOrderedChild(ChildType *item,
                            const std::function<bool(const ChildType *, const ChildType *)> &cmp)
    {
        const auto cmp0 = [cmp, this](const TreeItem *lhs, const TreeItem *rhs) {
            return cmp(childItemCast(lhs), childItemCast(rhs));
        };
        TreeItem::insertOrderedChild(item, cmp0);
    }

    ChildType *findAnyChild(const std::function<bool(TreeItem *)> &pred) const
    {
        return childItemCast(TreeItem::findAnyChild(pred));
    }

    ChildType *reverseFindAnyChild(const std::function<bool(TreeItem *)> &pred) const
    {
        return childItemCast(TreeItem::reverseFindAnyChild(pred));
    }

private:
    ChildType *childItemCast(TreeItem *item) const { return itemCast<ChildType>(item); }
    const ChildType *childItemCast(const TreeItem *item) const
    {
        return itemCast<ChildType>(const_cast<TreeItem *>(item));
    }
    ParentType *parentItemCast(TreeItem *item) const { return itemCast<ParentType>(item); }

    template<typename T>
    T *itemCast(TreeItem *item) const
    {
#ifdef NDEBUG
        return static_cast<T *>(item);
#else
        if (!item)
            return nullptr;
        const auto cItem = dynamic_cast<T *>(item);
        QTC_CHECK(cItem);
        return cItem;
#endif
    }
};

class UTILS_EXPORT StaticTreeItem : public TreeItem
{
public:
    StaticTreeItem(const QStringList &displays);
    StaticTreeItem(const QString &display);
    StaticTreeItem(const QStringList &displays, const QStringList &toolTips);

    QVariant data(int column, int role) const override;
    Qt::ItemFlags flags(int column) const override;

private:
    QStringList m_displays;
    QStringList m_toolTips;
};

// A general purpose multi-level model where each item can have its
// own (TreeItem-derived) type.
class UTILS_EXPORT BaseTreeModel : public QAbstractItemModel
{
    Q_OBJECT

protected:
    explicit BaseTreeModel(QObject *parent = nullptr);
    explicit BaseTreeModel(TreeItem *root, QObject *parent = nullptr);
    ~BaseTreeModel() override;

    void setHeader(const QStringList &displays);
    void setHeaderToolTip(const QStringList &tips);
    void clear();

    TreeItem *rootItem() const;
    void setRootItem(TreeItem *item); // resets the model
    void setRootItemInternal(TreeItem *item);
    TreeItem *itemForIndex(const QModelIndex &) const;
    QModelIndex indexForItem(const TreeItem *needle) const;

    int rowCount(const QModelIndex &idx = QModelIndex()) const override;
    int columnCount(const QModelIndex &idx) const override;

    bool setData(const QModelIndex &idx, const QVariant &data, int role) override;
    QVariant data(const QModelIndex &idx, int role) const override;
    QModelIndex index(int, int, const QModelIndex &idx = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &idx) const override;
    QModelIndex sibling(int row, int column, const QModelIndex &idx) const override;
    Qt::ItemFlags flags(const QModelIndex &idx) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    bool hasChildren(const QModelIndex &idx) const override;

    bool canFetchMore(const QModelIndex &idx) const override;
    void fetchMore(const QModelIndex &idx) override;

    TreeItem *takeItem(TreeItem *item); // item is not destroyed.
    void destroyItem(TreeItem *item);   // item is destroyed.

signals:
    void requestExpansion(const QModelIndex &);
    void requestCollapse(const QModelIndex &);

protected:
    friend class TreeItem;

    TreeItem *m_root; // Owned.
    QStringList m_header;
    QStringList m_headerToolTip;
    int m_columnCount;
};

namespace Internal {

// SelectType<N, T0, T1, T2, ...> selects the Nth type from the list
// If there are not enough types in the list, 'TreeItem' is used.
template<int N, typename... All>
struct SelectType;

template<int N, typename First, typename... Rest>
struct SelectType<N, First, Rest...>
{
    using Type = typename SelectType<N - 1, Rest...>::Type;
};

template<typename First, typename... Rest>
struct SelectType<0, First, Rest...>
{
    using Type = First;
};

template<int N>
struct SelectType<N>
{
    using Type = TreeItem;
};

// BestItem<T0, T1, T2, ... > selects T0 if all types are equal and 'TreeItem' otherwise
template<typename... All>
struct BestItemType;

template<typename First, typename Second, typename... Rest>
struct BestItemType<First, Second, Rest...>
{
    using Type = TreeItem;
};

template<typename First, typename... Rest>
struct BestItemType<First, First, Rest...>
{
    using Type = typename BestItemType<First, Rest...>::Type;
};

template<typename First>
struct BestItemType<First>
{
    using Type = First;
};

template<>
struct BestItemType<>
{
    using Type = TreeItem;
};

} // namespace Internal

// A multi-level model with possibly uniform types per level.
template<typename... LevelItemTypes>
class TreeModel : public BaseTreeModel
{
public:
    using RootItem = typename Internal::SelectType<0, LevelItemTypes...>::Type;
    using BestItem = typename Internal::BestItemType<LevelItemTypes...>::Type;

    explicit TreeModel(QObject *parent = nullptr)
        : BaseTreeModel(new RootItem, parent)
    {}
    explicit TreeModel(RootItem *root, QObject *parent = nullptr)
        : BaseTreeModel(root, parent)
    {}

    using BaseTreeModel::canFetchMore;
    using BaseTreeModel::clear;
    using BaseTreeModel::columnCount;
    using BaseTreeModel::data;
    using BaseTreeModel::destroyItem;
    using BaseTreeModel::fetchMore;
    using BaseTreeModel::hasChildren;
    using BaseTreeModel::index;
    using BaseTreeModel::indexForItem;
    using BaseTreeModel::parent;
    using BaseTreeModel::rowCount;
    using BaseTreeModel::setData;
    using BaseTreeModel::setHeader;
    using BaseTreeModel::setHeaderToolTip;
    using BaseTreeModel::takeItem;

    template<int Level, class Predicate>
    void forItemsAtLevel(const Predicate &pred) const
    {
        using ItemType = typename Internal::SelectType<Level, LevelItemTypes...>::Type;
        const auto pred0 = [pred](TreeItem *treeItem) { pred(static_cast<ItemType *>(treeItem)); };
        m_root->forChildrenAtLevel(Level, pred0);
    }

    template<int Level, class Predicate>
    typename Internal::SelectType<Level, LevelItemTypes...>::Type *findItemAtLevel(
        const Predicate &pred) const
    {
        using ItemType = typename Internal::SelectType<Level, LevelItemTypes...>::Type;
        const auto pred0 = [pred](TreeItem *treeItem) {
            return pred(static_cast<ItemType *>(treeItem));
        };
        return static_cast<ItemType *>(m_root->findChildAtLevel(Level, pred0));
    }

    RootItem *rootItem() const { return static_cast<RootItem *>(BaseTreeModel::rootItem()); }

    template<int Level>
    typename Internal::SelectType<Level, LevelItemTypes...>::Type *itemForIndexAtLevel(
        const QModelIndex &idx) const
    {
        TreeItem *item = BaseTreeModel::itemForIndex(idx);
        return item && item->level() == Level
                   ? static_cast<typename Internal::SelectType<Level, LevelItemTypes...>::Type *>(
                         item)
                   : nullptr;
    }

    BestItem *nonRootItemForIndex(const QModelIndex &idx) const
    {
        TreeItem *item = BaseTreeModel::itemForIndex(idx);
        return item && item->parent() ? static_cast<BestItem *>(item) : nullptr;
    }

    template<class Predicate>
    BestItem *findNonRootItem(const Predicate &pred) const
    {
        const auto pred0 = [pred](TreeItem *treeItem) -> bool {
            return pred(static_cast<BestItem *>(treeItem));
        };
        return static_cast<BestItem *>(m_root->findAnyChild(pred0));
    }

    template<class Predicate>
    void forSelectedItems(const Predicate &pred) const
    {
        const auto pred0 = [pred](TreeItem *treeItem) -> bool {
            return pred(static_cast<BestItem *>(treeItem));
        };
        m_root->forSelectedChildren(pred0);
    }

    template<class Predicate>
    void forAllItems(const Predicate &pred) const
    {
        const auto pred0 = [pred](TreeItem *treeItem) -> void {
            pred(static_cast<BestItem *>(treeItem));
        };
        m_root->forAllChildren(pred0);
    }

    BestItem *itemForIndex(const QModelIndex &idx) const
    {
        return static_cast<BestItem *>(BaseTreeModel::itemForIndex(idx));
    }
};

// By default, does natural sorting by display name. Call setLessThan() to customize.
class UTILS_EXPORT SortModel : public QSortFilterProxyModel
{
public:
    using QSortFilterProxyModel::QSortFilterProxyModel;
    using LessThan = std::function<bool(const QModelIndex &, const QModelIndex &)>;
    void setLessThan(const LessThan &lessThan) { m_lessThan = lessThan; }

protected:
    bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override;

private:
    LessThan m_lessThan;
};

} // namespace Utils

Q_DECLARE_METATYPE(Utils::TreeItem *)
