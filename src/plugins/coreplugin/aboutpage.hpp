#pragma once

#include <shell/page.hpp>

#include <QWidget>

namespace Core {

// 关于页：应用信息 + 系统信息。原来是一个独立插件，现在并入外壳。
class AboutPage final : public QWidget, public Shell::IPage
{
    Q_OBJECT

public:
    explicit AboutPage(QWidget *parent = nullptr);

    auto id() const -> QString override { return QStringLiteral("coreplugin.about"); }
    auto title() const -> QString override { return tr("About"); }
    auto area() const -> Shell::PageArea override { return Shell::PageArea::Help; }
    auto order() const -> int override { return 10; }
    auto widget() -> QWidget * override { return this; }
};

} // namespace Core
