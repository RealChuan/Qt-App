#include "commandpalette.hpp"

#include <shell/command.hpp>
#include <shell/registry.hpp>

#include <QEvent>
#include <QLineEdit>
#include <QListWidget>
#include <QVBoxLayout>

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
        item->setText(command->shortcut().isEmpty()
                          ? command->title()
                          : QStringLiteral("%1    %2").arg(command->title(),
                                                           command->shortcut().toString()));
        item->setData(Qt::UserRole, QVariant::fromValue<quintptr>(reinterpret_cast<quintptr>(command)));
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

    auto *command = reinterpret_cast<Shell::ICommand *>(item->data(Qt::UserRole).value<quintptr>());
    q_ptr->hide();
    if (command)
        command->execute();
}

CommandPalette::CommandPalette(QWidget *parent)
    : QDialog(parent)
    , d_ptr(std::make_unique<CommandPalettePrivate>(this))
{
    setWindowTitle(tr("Command Palette"));
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    resize(560, 380);

    d_ptr->edit = new QLineEdit(this);
    d_ptr->edit->setPlaceholderText(tr("Type a command..."));

    d_ptr->list = new QListWidget(this);
    d_ptr->list->setAlternatingRowColors(true);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->addWidget(d_ptr->edit);
    layout->addWidget(d_ptr->list, 1);

    connect(d_ptr->edit, &QLineEdit::textChanged, this, [this](const QString &filter) {
        d_ptr->refresh(filter);
    });
    connect(d_ptr->edit, &QLineEdit::returnPressed, this, [this] { d_ptr->activateCurrent(); });
    connect(d_ptr->list, &QListWidget::itemActivated, this, [this] { d_ptr->activateCurrent(); });
    connect(d_ptr->list, &QListWidget::itemClicked, this, [this] { d_ptr->activateCurrent(); });
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

    show();
    raise();
    activateWindow();
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

} // namespace Core
