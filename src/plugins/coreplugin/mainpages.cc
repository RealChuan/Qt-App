#include "mainwindow_p.hpp"

#include "pageframe.hpp"

#include <shell/registry.hpp>

#include <QButtonGroup>
#include <QLabel>
#include <QPushButton>
#include <QSizePolicy>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QWidget>

using namespace Qt::StringLiterals;

namespace Core {
namespace {

constexpr auto kPageIdProperty = "pageId";
constexpr auto kPageOrderProperty = "pageOrder";

auto sectionIndex(Shell::PageArea area) -> int
{
    switch (area) {
    case Shell::PageArea::Workspace:
        return 0;
    case Shell::PageArea::Games:
        return 1;
    case Shell::PageArea::Settings:
        return 2;
    case Shell::PageArea::Help:
        return 3;
    }
    return 0;
}

} // namespace

// 插件按需注册，空分组连标题一起隐藏，避免侧栏出现孤立的小标题。
void MainWindow::MainWindowPrivate::updateSections()
{
    for (Section &section : sections) {
        const bool visible = section.layout->count() > 0;
        section.label->setVisible(visible);
        section.container->setVisible(visible);
    }
}

auto MainWindow::MainWindowPrivate::sectionAt(Shell::PageArea area) -> Section &
{
    return sections[sectionIndex(area)];
}

// 语言切换后，侧栏按钮与页面标题都来自 IPage::title()/subtitle()，需要重新取一次。
void MainWindow::MainWindowPrivate::retranslatePages()
{
    for (Shell::IPage *page : Shell::Registry::instance()->pages()) {
        const auto it = pageIndex.constFind(page->id());
        if (it == pageIndex.cend())
            continue;

        if (auto *button = group->button(it.value()))
            button->setText(page->title());
        // stack 里只放 PageFrame，index 来自 pageIndex，类型是确定的。
        static_cast<PageFrame *>(stack->widget(it.value()))
            ->setTexts(page->title(), page->subtitle());
    }
}

auto MainWindow::MainWindowPrivate::buttonForPage(Shell::IPage *page) -> QPushButton *
{
    auto *button = new QPushButton(page->title());
    button->setObjectName(u"SidebarButton"_s);
    button->setCheckable(true);
    button->setCursor(Qt::PointingHandCursor);
    button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    if (!page->icon().isNull())
        button->setIcon(page->icon());
    button->setProperty(kPageIdProperty, page->id());
    button->setProperty(kPageOrderProperty, page->order());
    return button;
}

void MainWindow::MainWindowPrivate::addPage(Shell::IPage *page)
{
    if (!page || pageIndex.contains(page->id()))
        return;

    QWidget *widget = page->widget();
    if (!widget)
        return;

    QPushButton *button = buttonForPage(page);
    QVBoxLayout *layout = sectionAt(page->area()).layout;

    // 同组内按 IPage::order() 插入，插件注册顺序不影响侧边栏顺序。
    const int order = page->order();
    int position = layout->count();
    for (int i = 0; i < layout->count(); ++i) {
        auto *existing = qobject_cast<QPushButton *>(layout->itemAt(i)->widget());
        if (!existing)
            continue;
        if (existing->property(kPageOrderProperty).toInt() > order) {
            position = i;
            break;
        }
    }
    layout->insertWidget(position, button);

    auto *frame = new PageFrame(widget, stack);
    frame->setTexts(page->title(), page->subtitle());

    const int index = stack->addWidget(frame);
    group->addButton(button, index);
    pageIndex.insert(page->id(), index);

    // 默认页取 order() 最小的 Workspace 页。
    if (page->area() == Shell::PageArea::Workspace
        && (!defaultPageChosen || order < defaultPageOrder)) {
        if (auto *selected = group->button(index))
            selected->setChecked(true);
        stack->setCurrentIndex(index);
        defaultPageChosen = true;
        defaultPageOrder = order;
    }

    updateSections();
}

} // namespace Core
