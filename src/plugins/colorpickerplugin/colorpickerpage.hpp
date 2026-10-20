#pragma once

#include <shell/page.hpp>

#include <QWidget>

namespace ColorPicker {

class ColorPickerPage final : public QWidget, public Shell::IPage
{
    Q_OBJECT

public:
    explicit ColorPickerPage(QWidget *parent = nullptr);

    auto id() const -> QString override { return QStringLiteral("colorpickerplugin.page"); }
    auto title() const -> QString override { return tr("Color Picker"); }
    auto area() const -> Shell::PageArea override { return Shell::PageArea::Help; }
    auto order() const -> int override { return 40; }
    auto widget() -> QWidget * override { return this; }

signals:
    void openRequested();
};

} // namespace ColorPicker
