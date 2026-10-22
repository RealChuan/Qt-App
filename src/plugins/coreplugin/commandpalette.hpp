#pragma once

#include <QDialog>

#include <memory>

namespace Core {

// Ctrl+K 命令面板：列出 Shell::Registry 注册的全部命令，按输入做子序列过滤。
// 行为对齐"弹出面板"：失去焦点或按 Esc 立即收起，不会一直悬浮。
class CommandPalette final : public QDialog
{
    Q_OBJECT

public:
    explicit CommandPalette(QWidget *parent = nullptr);
    ~CommandPalette() override;

    void openPalette();

private slots:
    void onFilterChanged(const QString &filter);
    void activateCurrentCommand();

protected:
    bool event(QEvent *event) override;
    void changeEvent(QEvent *event) override;

private:
    void retranslateUi();

    class CommandPalettePrivate;
    std::unique_ptr<CommandPalettePrivate> d_ptr;
};

} // namespace Core
