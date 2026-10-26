#include "commandpalette.hpp"

#include <shell/command.hpp>
#include <shell/registry.hpp>
#include <utils/uihelpers.hpp>
#include <utils/utils.hpp>

#include <QAbstractItemView>
#include <QEvent>
#include <QFrame>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QVBoxLayout>

using namespace Qt::StringLiterals;

namespace Core {

namespace {

// 子序列匹配："dkm" 能匹配 "danmaku push"，不要求连续。
auto matches(const QString &text, const QString &filter) -> bool
{
    if (filter.isEmpty())
        return true;

    qsizetype index = 0;
    for (const QChar ch : text) {
        if (index < filter.size() && ch.toCaseFolded() == filter.at(index).toCaseFolded())
            ++index;
        if (index == filter.size())
            return true;
    }
    return false;
}

constexpr int kOuterMargin = 14;

} // namespace

class CommandPalette::CommandPalettePrivate
{
public:
    explicit CommandPalettePrivate(CommandPalette *q)
        : q_ptr(q)
    {}

    void refresh(const QString &filter);
    void activateCurrent();

    CommandPalette *q_ptr = nullptr;
    QLineEdit *edit = nullptr;
    QListWidget *list = nullptr;
    QLabel *hint = nullptr;
    bool activatedOnce = false;
};

void CommandPalette::CommandPalettePrivate::refresh(const QString &filter)
{
    list->clear();
    const QList<Shell::ICommand *> commands = Shell::Registry::instance()->commands();
    for (Shell::ICommand *command : commands) {
        if (!command->isEnabled())
            continue;
        if (!matches(command->title(), filter) && !matches(command->subtitle(), filter))
            continue;

        auto *item = new QListWidgetItem(list);
        item->setText(
            command->shortcut().isEmpty()
                ? command->title()
                : u"%1    %2"_s.arg(command->title(), command->shortcut().toString()));
        item->setData(Qt::UserRole, QVariant::fromValue(command));
        if (!command->subtitle().isEmpty())
            item->setToolTip(command->subtitle());
    }

    if (list->count() > 0)
        list->setCurrentRow(0);
}

void CommandPalette::CommandPalettePrivate::activateCurrent()
{
    QListWidgetItem *item = list->currentItem();
    if (!item)
        return;

    auto *command = item->data(Qt::UserRole).value<Shell::ICommand *>();
    q_ptr->hide();
    if (command)
        command->execute();
}

CommandPalette::CommandPalette(QWidget *parent)
    : QDialog(parent)
    , d_ptr(std::make_unique<CommandPalettePrivate>(this))
{
    setObjectName("CommandPalette");
    setWindowTitle(tr("Command Palette"));
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    resize(580, 420);

    d_ptr->edit = new QLineEdit(this);
    d_ptr->edit->setObjectName("PaletteSearch");

    d_ptr->list = new QListWidget(this);
    d_ptr->list->setObjectName("PaletteList");
    d_ptr->list->setFrameShape(QFrame::NoFrame);
    d_ptr->list->setAlternatingRowColors(false);
    d_ptr->list->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

    auto *separator = new QFrame(this);
    separator->setObjectName("PaletteSeparator");
    separator->setFrameShape(QFrame::NoFrame);

    d_ptr->hint = new QLabel(this);
    d_ptr->hint->setObjectName("PaletteHint");

    // 圆角卡片 + 阴影：外面那 14px 是给阴影留的位置。
    auto *card = new QFrame(this);
    card->setObjectName("PaletteCard");
    auto *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(12, 8, 12, 6);
    cardLayout->setSpacing(0);
    cardLayout->addWidget(d_ptr->edit);
    cardLayout->addWidget(separator);
    cardLayout->addWidget(d_ptr->list, 1);
    cardLayout->addWidget(d_ptr->hint);

    // 比窗口阴影更重、更下坠：命令面板是压在内容上的一块浮层。
    Utils::addDropShadow(
        card, {.margin = kOuterMargin, .offset = {0, 8}, .color = QColor(0, 0, 0, 90)});

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(kOuterMargin, kOuterMargin, kOuterMargin, kOuterMargin);
    layout->setSpacing(0);
    layout->addWidget(card);

    retranslateUi();

    connect(d_ptr->edit, &QLineEdit::textChanged, this, &CommandPalette::onFilterChanged);
    connect(d_ptr->edit, &QLineEdit::returnPressed, this, &CommandPalette::activateCurrentCommand);
    connect(d_ptr->list, &QListWidget::itemActivated, this, &CommandPalette::activateCurrentCommand);
    connect(d_ptr->list, &QListWidget::itemClicked, this, &CommandPalette::activateCurrentCommand);
}

void CommandPalette::retranslateUi()
{
    d_ptr->edit->setPlaceholderText(tr("Type a command"));
    d_ptr->hint->setText(tr("Enter runs the selected command, Esc closes this panel"));
}

void CommandPalette::onFilterChanged(const QString &filter)
{
    d_ptr->refresh(filter);
}

void CommandPalette::activateCurrentCommand()
{
    d_ptr->activateCurrent();
}

CommandPalette::~CommandPalette() = default;

void CommandPalette::openPalette()
{
    d_ptr->edit->clear();
    d_ptr->refresh({});

    if (QWidget *parent = parentWidget()) {
        const QRect geometry = parent->geometry();
        move(geometry.center() - rect().center());
    }

    Utils::restoreAndActivate(this);
    d_ptr->edit->setFocus();
}

bool CommandPalette::event(QEvent *event)
{
    switch (event->type()) {
    case QEvent::WindowActivate:
        d_ptr->activatedOnce = true;
        break;
    case QEvent::WindowDeactivate:
        // 点了别处就收起：命令面板是"浮层"，不是常驻窗口。
        if (d_ptr->activatedOnce)
            hide();
        break;
    case QEvent::Close:
        hide();
        event->ignore();
        return true;
    default:
        break;
    }
    return QDialog::event(event);
}

void CommandPalette::changeEvent(QEvent *event)
{
    QDialog::changeEvent(event);
    if (event->type() == QEvent::LanguageChange)
        retranslateUi();
}

} // namespace Core
