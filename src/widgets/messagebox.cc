#include "messagebox.hpp"

#include <QDebug>
#include <QEvent>
#include <QHBoxLayout>
#include <QHash>
#include <QLabel>
#include <QPushButton>
#include <QStyle>
#include <QToolButton>
#include <QVBoxLayout>

namespace Widgets {

#ifndef Q_OS_MACOS

namespace {

constexpr int kIconSize = 40;
constexpr auto kStandardButtonProperty = "standardButton";

// 按钮的固定顺序：确认类在前、否定类在后，和 QMessageBox 在 Windows 上的排布一致。
constexpr QMessageBox::StandardButton kButtonOrder[] = {
    QMessageBox::Ok,
    QMessageBox::Yes,
    QMessageBox::No,
    QMessageBox::Cancel,
    QMessageBox::Close,
};

constexpr QMessageBox::StandardButtons kSupportedButtons = QMessageBox::Ok | QMessageBox::Yes
                                                           | QMessageBox::No | QMessageBox::Cancel
                                                           | QMessageBox::Close;

// 只有 Ok / Yes 会让调用方拿到"用户同意了"，其余都算拒绝。
auto isAcceptButton(QMessageBox::StandardButton button) -> bool
{
    return button == QMessageBox::Ok || button == QMessageBox::Yes;
}

auto standardIconFor(QMessageBox::Icon icon) -> QStyle::StandardPixmap
{
    switch (icon) {
    case QMessageBox::Warning:
        return QStyle::SP_MessageBoxWarning;
    case QMessageBox::Critical:
        return QStyle::SP_MessageBoxCritical;
    case QMessageBox::Question:
        return QStyle::SP_MessageBoxQuestion;
    default:
        return QStyle::SP_MessageBoxInformation;
    }
}

} // namespace

class MessageBox::MessageBoxPrivate
{
public:
    explicit MessageBoxPrivate(MessageBox *q)
        : q_ptr(q)
        , iconButton(new QToolButton(q))
        , messageLabel(new QLabel(q))
        , buttonRow(new QHBoxLayout)
    {
        iconButton->setIconSize({kIconSize, kIconSize});
        iconButton->setObjectName("MessageIcon");
        iconButton->hide();

        messageLabel->setObjectName("MessageLabel");
        messageLabel->setWordWrap(true);
        messageLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        messageLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);

        buttonRow->setContentsMargins({});
        buttonRow->setSpacing(8);
        buttonRow->addStretch();
    }

    void setupUi()
    {
        auto *widget = new QWidget(q_ptr);

        // 图标在左、正文在右；按钮统一靠右，和正文左对齐形成一条干净的基线。
        auto *textRow = new QHBoxLayout;
        textRow->setContentsMargins({});
        textRow->setSpacing(16);
        textRow->addWidget(iconButton, 0, Qt::AlignTop);
        textRow->addWidget(messageLabel, 1);

        auto *layout = new QVBoxLayout(widget);
        layout->setContentsMargins(24, 24, 24, 20);
        layout->setSpacing(20);
        layout->addLayout(textRow, 1);
        layout->addLayout(buttonRow);

        q_ptr->setCentralWidget(widget);
    }

    MessageBox *q_ptr = nullptr;
    QToolButton *iconButton = nullptr;
    QLabel *messageLabel = nullptr;
    QHBoxLayout *buttonRow = nullptr;
    QHash<int, QPushButton *> buttons;
    QMessageBox::StandardButtons standardButtons = QMessageBox::Ok;
    QMessageBox::StandardButton defaultButton = QMessageBox::NoButton;
    QMessageBox::StandardButton clicked = QMessageBox::NoButton;
};

MessageBox::MessageBox(QWidget *parent)
    : Dialog(parent)
    , d_ptr(std::make_unique<MessageBoxPrivate>(this))
{
    d_ptr->setupUi();
    applyStandardButtons();
    resize(420, 220);
}

MessageBox::~MessageBox() = default;

auto MessageBox::exec() -> int
{
    d_ptr->clicked = QMessageBox::NoButton;
    Dialog::exec();
    return d_ptr->clicked;
}

auto MessageBox::clickedButton() const -> QAbstractButton *
{
    return d_ptr->buttons.value(int(d_ptr->clicked), nullptr);
}

auto MessageBox::buttons() const -> QList<QAbstractButton *>
{
    // 按 kButtonOrder 的顺序返回，与创建顺序一致；QHash 本身不保序。
    QList<QAbstractButton *> result;
    for (const auto standard : kButtonOrder) {
        if (QPushButton *button = d_ptr->buttons.value(int(standard), nullptr))
            result.append(button);
    }
    return result;
}

auto MessageBox::button(QMessageBox::StandardButton which) const -> QAbstractButton *
{
    return d_ptr->buttons.value(int(which), nullptr);
}

void MessageBox::setText(const QString &text)
{
    d_ptr->messageLabel->setText(text);
}

void MessageBox::setIcon(QMessageBox::Icon icon)
{
    if (icon == QMessageBox::NoIcon) {
        d_ptr->iconButton->hide();
        return;
    }

    d_ptr->iconButton->setIcon(style()->standardIcon(standardIconFor(icon)));
    d_ptr->iconButton->show();
}

void MessageBox::setStandardButtons(QMessageBox::StandardButtons buttons)
{
    if (const QMessageBox::StandardButtons unsupported = buttons & ~kSupportedButtons;
        unsupported != QMessageBox::StandardButtons()) {
        // 响亮地拒绝，而不是画一个按下去没反应的按钮。
        qWarning().noquote() << "Widgets::MessageBox: unsupported standard buttons ignored:"
                             << unsupported;
    }

    d_ptr->standardButtons = buttons & kSupportedButtons;
    if (d_ptr->standardButtons == QMessageBox::StandardButtons())
        d_ptr->standardButtons = QMessageBox::Close; // 一个按钮都不剩就关不掉窗了

    applyStandardButtons();
}

void MessageBox::setDefaultButton(QMessageBox::StandardButton button)
{
    d_ptr->defaultButton = button;
    applyDefaultButton();
}

void MessageBox::reject()
{
    if (d_ptr->clicked == QMessageBox::NoButton) {
        // Esc / 关窗：找取消类按钮顶替，都没有就留 NoButton —— QMessageBox 也这么报。
        for (const auto candidate : {QMessageBox::Cancel, QMessageBox::Close, QMessageBox::No}) {
            if (d_ptr->buttons.contains(int(candidate))) {
                d_ptr->clicked = candidate;
                break;
            }
        }
    }

    Dialog::reject();
}

void MessageBox::changeEvent(QEvent *event)
{
    Dialog::changeEvent(event);
    if (event->type() == QEvent::LanguageChange)
        retranslateUi();
}

void MessageBox::retranslateUi()
{
    for (auto it = d_ptr->buttons.cbegin(); it != d_ptr->buttons.cend(); ++it) {
        it.value()->setText(buttonText(static_cast<QMessageBox::StandardButton>(it.key())));
    }
}

void MessageBox::onStandardButtonClicked()
{
    const auto *button = qobject_cast<QPushButton *>(sender());
    if (!button)
        return;

    const auto standard = static_cast<QMessageBox::StandardButton>(
        button->property(kStandardButtonProperty).toInt());
    d_ptr->clicked = standard;

    if (isAcceptButton(standard))
        accept();
    else
        reject();
}

void MessageBox::applyStandardButtons()
{
    // 换按钮集合时重建，不能叠一层。
    for (QPushButton *button : std::as_const(d_ptr->buttons))
        delete button;
    d_ptr->buttons.clear();

    for (const auto standard : kButtonOrder) {
        if (!d_ptr->standardButtons.testFlag(standard))
            continue;

        auto *button = new QPushButton(buttonText(standard), this);
        button->setObjectName(isAcceptButton(standard) ? "PrimaryButton" : "GrayButton");
        button->setProperty(kStandardButtonProperty, int(standard));
        connect(button, &QPushButton::clicked, this, &MessageBox::onStandardButtonClicked);

        d_ptr->buttonRow->addWidget(button);
        d_ptr->buttons.insert(int(standard), button);
    }

    applyDefaultButton();
}

void MessageBox::applyDefaultButton()
{
    QMessageBox::StandardButton target = d_ptr->defaultButton;
    if (!d_ptr->buttons.contains(int(target))) {
        // 没指定、或指定的按钮不在当前集合里：按 QMessageBox 的习惯让第一个确认按钮做默认。
        target = QMessageBox::NoButton;
        for (const auto candidate : kButtonOrder) {
            if (d_ptr->buttons.contains(int(candidate)) && isAcceptButton(candidate)) {
                target = candidate;
                break;
            }
        }
        if (target == QMessageBox::NoButton) {
            for (const auto candidate : kButtonOrder) {
                if (d_ptr->buttons.contains(int(candidate))) {
                    target = candidate;
                    break;
                }
            }
        }
    }

    for (auto it = d_ptr->buttons.cbegin(); it != d_ptr->buttons.cend(); ++it)
        it.value()->setDefault(it.key() == int(target));
}

auto MessageBox::buttonText(QMessageBox::StandardButton button) const -> QString
{
    switch (button) {
    case QMessageBox::Ok:
        return tr("OK");
    case QMessageBox::Yes:
        return tr("Yes");
    case QMessageBox::No:
        return tr("No");
    case QMessageBox::Cancel:
        return tr("Cancel");
    case QMessageBox::Close:
        return tr("Close");
    default:
        return {};
    }
}

namespace {

auto runMessageBox(
    QWidget *parent,
    QMessageBox::Icon icon,
    const QString &title,
    const QString &text,
    QMessageBox::StandardButtons buttons,
    QMessageBox::StandardButton defaultButton) -> QMessageBox::StandardButton
{
    MessageBox box(parent);
    box.setWindowTitle(title);
    box.setIcon(icon);
    box.setText(text);
    box.setStandardButtons(buttons);
    box.setDefaultButton(defaultButton);
    return static_cast<QMessageBox::StandardButton>(box.exec());
}

} // namespace

auto MessageBox::information(
    QWidget *parent,
    const QString &title,
    const QString &text,
    QMessageBox::StandardButtons buttons,
    QMessageBox::StandardButton defaultButton) -> QMessageBox::StandardButton
{
    return runMessageBox(parent, QMessageBox::Information, title, text, buttons, defaultButton);
}

auto MessageBox::warning(
    QWidget *parent,
    const QString &title,
    const QString &text,
    QMessageBox::StandardButtons buttons,
    QMessageBox::StandardButton defaultButton) -> QMessageBox::StandardButton
{
    return runMessageBox(parent, QMessageBox::Warning, title, text, buttons, defaultButton);
}

auto MessageBox::question(
    QWidget *parent,
    const QString &title,
    const QString &text,
    QMessageBox::StandardButtons buttons,
    QMessageBox::StandardButton defaultButton) -> QMessageBox::StandardButton
{
    return runMessageBox(parent, QMessageBox::Question, title, text, buttons, defaultButton);
}

auto MessageBox::critical(
    QWidget *parent,
    const QString &title,
    const QString &text,
    QMessageBox::StandardButtons buttons,
    QMessageBox::StandardButton defaultButton) -> QMessageBox::StandardButton
{
    return runMessageBox(parent, QMessageBox::Critical, title, text, buttons, defaultButton);
}

#endif

} // namespace Widgets
