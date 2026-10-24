#include "roundedrowtreeview.hpp"

#include <QPainter>
#include <QPainterPath>
#include <QRectF>

namespace Widgets {

namespace {
constexpr int kDefaultRowRadius = 6;
constexpr int kDefaultRowInsetX = 2;
constexpr int kDefaultRowInsetY = 1;
} // namespace

class RoundedRowTreeView::RoundedRowTreeViewPrivate
{
public:
    int radius = kDefaultRowRadius;
    QSize inset{kDefaultRowInsetX, kDefaultRowInsetY};
};

RoundedRowTreeView::RoundedRowTreeView(QWidget *parent)
    : QTreeView(parent)
    , d_ptr(std::make_unique<RoundedRowTreeViewPrivate>())
{}

RoundedRowTreeView::~RoundedRowTreeView() = default;

auto RoundedRowTreeView::rowRadius() const -> int
{
    return d_ptr->radius;
}

auto RoundedRowTreeView::rowInset() const -> QSize
{
    return d_ptr->inset;
}

void RoundedRowTreeView::setRowRadius(int radius)
{
    if (d_ptr->radius == radius)
        return;

    d_ptr->radius = radius;
    viewport()->update();
}

void RoundedRowTreeView::setRowInset(const QSize &inset)
{
    if (d_ptr->inset == inset)
        return;

    d_ptr->inset = inset;
    viewport()->update();
}

// 每一行的内容（单元格底色、分支区、文字）都裁剪到同一个圆角矩形里，
// 这样连续多列的选中条只有一个外轮廓，列与列之间不会有缝。
void RoundedRowTreeView::drawRow(
    QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    const int width = viewport()->width() - 2 * d_ptr->inset.width();
    const int height = option.rect.height() - 2 * d_ptr->inset.height();
    if (width <= 0 || height <= 0) {
        QTreeView::drawRow(painter, option, index);
        return;
    }

    QPainterPath clip;
    clip.addRoundedRect(
        QRectF(d_ptr->inset.width(), option.rect.y() + d_ptr->inset.height(), width, height),
        d_ptr->radius,
        d_ptr->radius);

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setClipPath(clip, Qt::IntersectClip);
    QTreeView::drawRow(painter, option, index);
    painter->restore();
}

} // namespace Widgets
