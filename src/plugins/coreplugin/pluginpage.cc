#include "pluginpage.hpp"

#include "plugindetailsview.hpp"
#include "pluginview.hpp"

#include <extensionsystem/pluginmanager.hpp>
#include <shell/host.hpp>

#include <QEvent>
#include <QFrame>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QScrollArea>
#include <QSplitter>
#include <QVBoxLayout>

namespace Core {

class PluginPage::PluginPagePrivate
{
public:
    explicit PluginPagePrivate(PluginPage *q)
        : q_ptr(q)
    {}

    void setupUi();

    PluginPage *q_ptr = nullptr;
    QLineEdit *filter = nullptr;
    QSplitter *splitter = nullptr;
    PluginView *view = nullptr;
    PluginDetailsView *details = nullptr;
};

// 只搭布局与父子关系；文案在 retranslateUi()，信号在 setupConnections()。
void PluginPage::PluginPagePrivate::setupUi()
{
    filter = new QLineEdit(q_ptr);
    filter->setObjectName("FilterEdit");
    filter->setClearButtonEnabled(true);
    filter->setMaximumWidth(320);

    view = new PluginView(q_ptr);

    // 两块内容各自装进卡片：左侧列表、右侧详情，中间留 12px 让背景透出来。
    auto *listCard = new QFrame(q_ptr);
    listCard->setObjectName("Card");
    auto *listLayout = new QVBoxLayout(listCard);
    listLayout->setContentsMargins(5, 5, 5, 5);
    listLayout->addWidget(view);

    details = new PluginDetailsView(q_ptr);
    auto *scroll = new QScrollArea(q_ptr);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->setWidget(details);

    auto *detailsCard = new QFrame(q_ptr);
    detailsCard->setObjectName("Card");
    auto *detailsLayout = new QVBoxLayout(detailsCard);
    detailsLayout->setContentsMargins(18, 16, 18, 16);
    detailsLayout->addWidget(scroll);

    splitter = new QSplitter(Qt::Horizontal, q_ptr);
    splitter->setChildrenCollapsible(false);
    splitter->addWidget(listCard);
    splitter->addWidget(detailsCard);
    splitter->setStretchFactor(0, 3);
    splitter->setStretchFactor(1, 2);
    // 只给 stretchFactor 时 QSplitter 会先按 sizeHint 排，右侧详情会被压得很窄。
    // 字号是 14/13px，详情列至少要 340 才不会把描述挤成一条窄缝。
    listCard->setMinimumWidth(420);
    detailsCard->setMinimumWidth(316);
    splitter->setSizes({460, 316});

    auto *filterRow = new QHBoxLayout;
    filterRow->setContentsMargins({});
    filterRow->addWidget(filter);
    filterRow->addStretch();

    auto *layout = new QVBoxLayout(q_ptr);
    layout->setContentsMargins({});
    layout->setSpacing(12);
    layout->addLayout(filterRow);
    layout->addWidget(splitter, 1);
}

PluginPage::PluginPage(QWidget *parent)
    : QWidget(parent)
    , d_ptr(std::make_unique<PluginPagePrivate>(this))
{
    d_ptr->setupUi();
    retranslateUi();
    setupConnections();
}

PluginPage::~PluginPage() = default;

void PluginPage::retranslateUi()
{
    d_ptr->filter->setPlaceholderText(tr("Filter plugins..."));
}

void PluginPage::setupConnections()
{
    connect(d_ptr->filter, &QLineEdit::textChanged, this, &PluginPage::onFilterChanged);
    connect(d_ptr->view, &PluginView::currentPluginChanged, this, &PluginPage::onCurrentPluginChanged);
    connect(d_ptr->view, &PluginView::pluginsChanged, this, &PluginPage::onPluginsChanged);

    connect(
        ExtensionSystem::PluginManager::instance(),
        &ExtensionSystem::PluginManager::pluginsChanged,
        d_ptr->view,
        &PluginView::rebuild);
}

void PluginPage::onFilterChanged(const QString &text)
{
    d_ptr->view->setFilter(text);
}

void PluginPage::onCurrentPluginChanged(ExtensionSystem::PluginSpec *spec)
{
    d_ptr->details->setSpec(spec);
}

void PluginPage::onPluginsChanged()
{
    if (Shell::host())
        Shell::host()->askForRestart(tr("Plugin changes take effect after a restart."));
}

void PluginPage::changeEvent(QEvent *event)
{
    QWidget::changeEvent(event);
    if (event->type() == QEvent::LanguageChange)
        retranslateUi();
}

} // namespace Core
