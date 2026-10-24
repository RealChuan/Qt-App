#pragma once

#include "widgets_global.hpp"

#include <QSize>
#include <QTreeView>

#include <memory>

namespace Widgets {

// 整行高亮画成圆角条的树视图。
//
// 颜色不在这里决定：选中/悬停的底色仍然由样式表的 QTreeView::item:selected
// 与 ::item:hover 给出。这里只解决形状——QSS 的 ::item 是按单元格绘制的，
// 圆角只能落在每个格子自己的四角上，多列之间会露出白缝。
class WIDGETS_EXPORT RoundedRowTreeView : public QTreeView
{
    Q_OBJECT

public:
    explicit RoundedRowTreeView(QWidget *parent = nullptr);
    ~RoundedRowTreeView() override;

    // 圆角半径，默认 6px。
    void setRowRadius(int radius);
    [[nodiscard]] auto rowRadius() const -> int;

    // 高亮相对整行的内缩（左右、上下），默认 (2, 1)。
    // 留一点缝，高亮条不会贴着面板边缘，也不会和相邻行连成一片。
    void setRowInset(const QSize &inset);
    [[nodiscard]] auto rowInset() const -> QSize;

protected:
    void drawRow(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index)
        const override;

private:
    class RoundedRowTreeViewPrivate;
    std::unique_ptr<RoundedRowTreeViewPrivate> d_ptr;
};

} // namespace Widgets
