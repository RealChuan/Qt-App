#pragma once

#include <shell/page.hpp>

#include <QWidget>

#include <memory>

namespace Hello {

// 模板插件的页面：一个居中标签 + 计数。
class HelloWidget final : public QWidget, public Shell::IPage
{
    Q_OBJECT

public:
    explicit HelloWidget(QWidget *parent = nullptr);
    ~HelloWidget() override;

    auto id() const -> QString override { return QStringLiteral("helloplugin.page"); }
    auto title() const -> QString override { return tr("Hello"); }
    auto order() const -> int override { return 10; }
    auto widget() -> QWidget * override { return this; }

    void setCount(int count);
    void increment();

signals:
    void countChanged(int count);

private:
    class HelloWidgetPrivate;
    std::unique_ptr<HelloWidgetPrivate> d_ptr;
};

} // namespace Hello
