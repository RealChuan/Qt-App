// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#pragma once

#include "gui_global.hpp"

#include <utils/indexedcontainerproxyconstiterator.h>

#include <QAbstractItemModel>

#include <functional>

namespace GUI {

class BaseTreeModel;

class GUI_EXPORT TreeItem
{
public:
    TreeItem();
    virtual ~TreeItem();

    [[nodiscard]] virtual auto data(int column, int role) const -> QVariant;
    virtual auto setData(int column, const QVariant &data, int role) -> bool;
    [[nodiscard]] virtual Qt::ItemFlags flags(int column) const;

    [[nodiscard]] virtual auto hasChildren() const -> bool;
    [[nodiscard]] virtual auto canFetchMore() const -> bool;
    virtual void fetchMore() {}

    [[nodiscard]] auto parent() const -> TreeItem * { return m_parent; }

    void prependChild(TreeItem *item);
    void appendChild(TreeItem *item);
    void insertChild(int pos, TreeItem *item);
    void insertOrderedChild(TreeItem *item,
                            const std::function<bool(const TreeItem *, const TreeItem *)> &cmp);

    void removeChildAt(int pos);
    void removeChildren();
    void sortChildren(const std::function<bool(const TreeItem *, const TreeItem *)> &cmp);
    void update();
    void updateAll();
    void updateColumn(int column);
    void expand();
    void collapse();
    [[nodiscard]] auto firstChild() const -> TreeItem *;
    [[nodiscard]] auto lastChild() const -> TreeItem *;
    [[nodiscard]] auto level() const -> int;

    using const_iterator = QVector<TreeItem *>::const_iterator;
    using value_type = TreeItem *;
    [[nodiscard]] auto childCount() const -> int { return m_children.size(); }
    [[nodiscard]] auto indexInParent() const -> int;
    [[nodiscard]] auto childAt(int index) const -> TreeItem *;
    auto indexOf(const TreeItem *item) const -> int;
    [[nodiscard]] const_iterator begin() const { return m_children.begin(); }
    [[nodiscard]] const_iterator end() const { return m_children.end(); }
    [[nodiscard]] auto index() const -> QModelIndex;
    [[nodiscard]] auto model() const -> QAbstractItemModel *;

    void forSelectedChildren(const std::function<bool(TreeItem *)> &pred) const;
    void forAllChildren(const std::function<void(TreeItem *)> &pred) const;
    [[nodiscard]] auto findAnyChild(const std::function<bool(TreeItem *)> &pred) const -> TreeItem *;
    // like findAnyChild() but processes children in exact reverse order
    // (bottom to top, most inner children first)
    [[nodiscard]] auto reverseFindAnyChild(const std::function<bool(TreeItem *)> &pred) const -> TreeItem *;

    // Levels are 1-based: Child at Level 1 is an immediate child.
    void forChildrenAtLevel(int level, const std::function<void(TreeItem *)> &pred) const;
    [[nodiscard]] auto findChildAtLevel(int level, const std::function<bool(TreeItem *)> &pred) const -> TreeItem *;

private:
    TreeItem(const TreeItem &) = delete;
    void operator=(const TreeItem &) = delete;

    void clear();
    void removeItemAt(int pos);
    void propagateModel(BaseTreeModel *m);

    TreeItem *m_parent = nullptr;     // Not owned.
    BaseTreeModel *m_model = nullptr; // Not owned.
    QVector<TreeItem *> m_children;   // Owned.
    friend class BaseTreeModel;
};

// A TreeItem with children all of the same type.
template<class ChildType, class ParentType = TreeItem>
class TypedTreeItem : public TreeItem
{
public:
    auto childAt(int index) const -> ChildType *
    {
        return static_cast<ChildType *>(TreeItem::childAt(index));
    }

    void sortChildren(const std::function<bool(const ChildType *, const ChildType *)> &lessThan)
    {
        return TreeItem::sortChildren([lessThan](const TreeItem *a, const TreeItem *b) {
            return lessThan(static_cast<const ChildType *>(a), static_cast<const ChildType *>(b));
        });
    }

    using value_type = ChildType *;
    using const_iterator = Utils::IndexedContainerProxyConstIterator<TypedTreeItem>;
    using size_type = int;

    auto operator[](int index) const -> ChildType * { return childAt(index); }
    [[nodiscard]] auto size() const -> int { return childCount(); }
    auto begin() const -> const_iterator { return const_iterator(*this, 0); }
    auto end() const -> const_iterator { return const_iterator(*this, size()); }

    template<typename Predicate>
    void forAllChildren(const Predicate &pred) const
    {
        const auto pred0 = [pred](TreeItem *treeItem) { pred(static_cast<ChildType *>(treeItem)); };
        TreeItem::forAllChildren(pred0);
    }

    template<typename Predicate>
    void forFirstLevelChildren(Predicate pred) const
    {
        const auto pred0 = [pred](TreeItem *treeItem) { pred(static_cast<ChildType *>(treeItem)); };
        TreeItem::forChildrenAtLevel(1, pred0);
    }

    template<typename Predicate>
    auto findFirstLevelChild(Predicate pred) const -> ChildType *
    {
        const auto pred0 = [pred](TreeItem *treeItem) {
            return pred(static_cast<ChildType *>(treeItem));
        };
        return static_cast<ChildType *>(TreeItem::findChildAtLevel(1, pred0));
    }

    auto parent() const -> ParentType * { return static_cast<ParentType *>(TreeItem::parent()); }

    void insertOrderedChild(ChildType *item,
                            const std::function<bool(const ChildType *, const ChildType *)> &cmp)
    {
        const auto cmp0 = [cmp](const TreeItem *lhs, const TreeItem *rhs) {
            return cmp(static_cast<const ChildType *>(lhs), static_cast<const ChildType *>(rhs));
        };
        TreeItem::insertOrderedChild(item, cmp0);
    }

    auto findAnyChild(const std::function<bool(TreeItem *)> &pred) const -> ChildType *
    {
        return static_cast<ChildType *>(TreeItem::findAnyChild(pred));
    }

    auto reverseFindAnyChild(const std::function<bool(TreeItem *)> &pred) const -> ChildType *
    {
        return static_cast<ChildType *>(TreeItem::reverseFindAnyChild(pred));
    }
};

class GUI_EXPORT StaticTreeItem : public TreeItem
{
public:
    explicit StaticTreeItem(const QStringList &displays);
    explicit StaticTreeItem(const QString &display);
    StaticTreeItem(const QStringList &displays, const QStringList &toolTips);

    [[nodiscard]] auto data(int column, int role) const -> QVariant override;
    [[nodiscard]] Qt::ItemFlags flags(int column) const override;

private:
    QStringList m_displays;
    QStringList m_toolTips;
};

// A general purpose multi-level model where each item can have its
// own (TreeItem-derived) type.
class GUI_EXPORT BaseTreeModel : public QAbstractItemModel
{
    Q_OBJECT

protected:
    explicit BaseTreeModel(QObject *parent = nullptr);
    explicit BaseTreeModel(TreeItem *root, QObject *parent = nullptr);
    ~BaseTreeModel() override;

    void setHeader(const QStringList &displays);
    void setHeaderToolTip(const QStringList &tips);
    void clear();

    [[nodiscard]] auto rootItem() const -> TreeItem *;
    void setRootItem(TreeItem *item);
    [[nodiscard]] auto itemForIndex(const QModelIndex &) const -> TreeItem *;
    auto indexForItem(const TreeItem *needle) const -> QModelIndex;

    [[nodiscard]] auto rowCount(const QModelIndex &idx = QModelIndex()) const -> int override;
    [[nodiscard]] auto columnCount(const QModelIndex &idx) const -> int override;

    auto setData(const QModelIndex &idx, const QVariant &data, int role) -> bool override;
    [[nodiscard]] auto data(const QModelIndex &idx, int role) const -> QVariant override;
    [[nodiscard]] auto index(int, int, const QModelIndex &idx = QModelIndex()) const -> QModelIndex override;
    [[nodiscard]] auto parent(const QModelIndex &idx) const -> QModelIndex override;
    [[nodiscard]] auto sibling(int row, int column, const QModelIndex &idx) const -> QModelIndex override;
    [[nodiscard]] Qt::ItemFlags flags(const QModelIndex &idx) const override;
    [[nodiscard]] auto headerData(int section, Qt::Orientation orientation, int role) const -> QVariant override;
    [[nodiscard]] auto hasChildren(const QModelIndex &idx) const -> bool override;

    [[nodiscard]] auto canFetchMore(const QModelIndex &idx) const -> bool override;
    void fetchMore(const QModelIndex &idx) override;

    auto takeItem(TreeItem *item) -> TreeItem *; // item is not destroyed.
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
    auto findItemAtLevel(
        const Predicate &pred) const -> typename Internal::SelectType<Level, LevelItemTypes...>::Type *
    {
        using ItemType = typename Internal::SelectType<Level, LevelItemTypes...>::Type;
        const auto pred0 = [pred](TreeItem *treeItem) {
            return pred(static_cast<ItemType *>(treeItem));
        };
        return static_cast<ItemType *>(m_root->findChildAtLevel(Level, pred0));
    }

    auto rootItem() const -> RootItem * { return static_cast<RootItem *>(BaseTreeModel::rootItem()); }

    template<int Level>
    auto itemForIndexAtLevel(
        const QModelIndex &idx) const -> typename Internal::SelectType<Level, LevelItemTypes...>::Type *
    {
        TreeItem *item = BaseTreeModel::itemForIndex(idx);
        return item && item->level() == Level
                   ? static_cast<typename Internal::SelectType<Level, LevelItemTypes...>::Type *>(
                       item)
                   : nullptr;
    }

    auto nonRootItemForIndex(const QModelIndex &idx) const -> BestItem *
    {
        TreeItem *item = BaseTreeModel::itemForIndex(idx);
        return (item != nullptr) && (item->parent() != nullptr) ? static_cast<BestItem *>(item) : nullptr;
    }

    template<class Predicate>
    auto findNonRootItem(const Predicate &pred) const -> BestItem *
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

    auto itemForIndex(const QModelIndex &idx) const -> BestItem *
    {
        return static_cast<BestItem *>(BaseTreeModel::itemForIndex(idx));
    }
};

} // namespace GUI

Q_DECLARE_METATYPE(GUI::TreeItem *)
