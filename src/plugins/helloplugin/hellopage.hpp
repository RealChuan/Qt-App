#pragma once

#include <shell/page.hpp>

#include <QWidget>

#include <memory>

namespace Hello {

// 模板插件的页面：一个居中标签 + 计数。
class HelloPage final : public QWidget, public Shell::IPage
{
    Q_OBJECT

public:
    explicit HelloPage(QWidget *parent = nullptr);
    ~HelloPage() override;

    [[nodiscard]] auto id() const -> QString override { return QStringLiteral("helloplugin.page"); }
    [[nodiscard]] auto title() const -> QString override { return tr("Hello"); }
    [[nodiscard]] auto subtitle() const -> QString override
    {
        return tr("A page contributed by helloplugin while the shell is starting up.");
    }
    [[nodiscard]] auto order() const -> int override { return Shell::PageOrder::kFirst; }
    [[nodiscard]] auto widget() -> QWidget * override { return this; }

    void setCount(int count);
    void increment();

signals:
    void countChanged(int count);

protected:
    void changeEvent(QEvent *event) override;

private:
    void updateText();

    class HelloPagePrivate;
    std::unique_ptr<HelloPagePrivate> d_ptr;
};

} // namespace Hello
