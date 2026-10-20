#pragma once

#include <shell/page.hpp>

#include <QWidget>

#include <memory>

namespace Core {

// 外观与语言设置。原来叫 ConfigWidget，现在也实现了 IPage，直接作为设置分组的一页。
class ConfigPage final : public QWidget, public Shell::IPage
{
    Q_OBJECT

public:
    explicit ConfigPage(QWidget *parent = nullptr);
    ~ConfigPage() override;

    auto id() const -> QString override { return QStringLiteral("coreplugin.appearance"); }
    auto title() const -> QString override { return tr("Appearance"); }
    auto area() const -> Shell::PageArea override { return Shell::PageArea::Settings; }
    auto order() const -> int override { return 30; }
    auto widget() -> QWidget * override { return this; }

protected:
    void changeEvent(QEvent *event) override;

private:
    class ConfigPagePrivate;
    std::unique_ptr<ConfigPagePrivate> d_ptr;
};

} // namespace Core
