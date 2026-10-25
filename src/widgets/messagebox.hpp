#pragma once

#include "dialog.hpp"

#include <QList>
#include <QMessageBox>

#include <memory>

class QAbstractButton;

namespace Widgets {

#ifndef Q_OS_MACOS
// 自绘消息框：图标、标准按钮、默认按钮、exec() 的返回值全部照 QMessageBox 的语义来，
// 这样 macOS 分支可以直接用原生 QMessageBox，调用方不需要判断平台。
class WIDGETS_EXPORT MessageBox final : public Dialog
{
    Q_OBJECT
public:
    explicit MessageBox(QWidget *parent = nullptr);
    ~MessageBox() override;

    void setText(const QString &text);
    void setIcon(QMessageBox::Icon icon);
    // 只实现 Ok / Yes / No / Cancel / Close；其余标准按钮会被忽略并打一条告警。
    void setStandardButtons(QMessageBox::StandardButtons buttons);
    void setDefaultButton(QMessageBox::StandardButton button);

    // 与 QMessageBox 一致：返回被点击的标准按钮；关窗且没有可对应的按钮时返回 NoButton。
    auto exec() -> int override;
    [[nodiscard]] auto clickedButton() const -> QAbstractButton *;
    // 同名同签名地抄 QMessageBox：调用方和测试都不必判断平台分支。
    [[nodiscard]] auto buttons() const -> QList<QAbstractButton *>;
    [[nodiscard]] auto button(QMessageBox::StandardButton which) const -> QAbstractButton *;

    static auto information(
        QWidget *parent,
        const QString &title,
        const QString &text,
        QMessageBox::StandardButtons buttons = QMessageBox::Ok,
        QMessageBox::StandardButton defaultButton = QMessageBox::NoButton)
        -> QMessageBox::StandardButton;
    static auto warning(
        QWidget *parent,
        const QString &title,
        const QString &text,
        QMessageBox::StandardButtons buttons = QMessageBox::Ok,
        QMessageBox::StandardButton defaultButton = QMessageBox::NoButton)
        -> QMessageBox::StandardButton;
    static auto question(
        QWidget *parent,
        const QString &title,
        const QString &text,
        QMessageBox::StandardButtons buttons = QMessageBox::Yes | QMessageBox::No,
        QMessageBox::StandardButton defaultButton = QMessageBox::NoButton)
        -> QMessageBox::StandardButton;
    static auto critical(
        QWidget *parent,
        const QString &title,
        const QString &text,
        QMessageBox::StandardButtons buttons = QMessageBox::Ok,
        QMessageBox::StandardButton defaultButton = QMessageBox::NoButton)
        -> QMessageBox::StandardButton;

public slots:
    // Esc 与关窗都走这里，按 QMessageBox 的习惯折成取消类按钮。
    void reject() override;

protected:
    // 按钮文案由本类提供，语言切换时同样要刷新。
    void changeEvent(QEvent *event) override;

private:
    void retranslateUi();
    void onStandardButtonClicked();
    void applyStandardButtons();
    void applyDefaultButton();
    [[nodiscard]] auto buttonText(QMessageBox::StandardButton button) const -> QString;

    class MessageBoxPrivate;
    std::unique_ptr<MessageBoxPrivate> d_ptr;
};

#else

// macOS：消息框就是原生 QMessageBox，接口天然一致，不需要再包一层。
using MessageBox = QMessageBox;

#endif

} // namespace Widgets
