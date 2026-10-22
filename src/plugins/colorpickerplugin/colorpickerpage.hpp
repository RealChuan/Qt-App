#pragma once

#include <shell/page.hpp>

#include <QWidget>

namespace ColorPicker {

class ColorPickerPage final : public QWidget, public Shell::IPage
{
    Q_OBJECT

public:
    explicit ColorPickerPage(QWidget *parent = nullptr);

    [[nodiscard]] auto id() const -> QString override
    {
        return QStringLiteral("colorpickerplugin.page");
    }
    [[nodiscard]] auto title() const -> QString override { return tr("Color Picker"); }
    [[nodiscard]] auto subtitle() const -> QString override
    {
        return tr("Sample any pixel on screen and copy its HEX value.");
    }
    [[nodiscard]] auto order() const -> int override { return Shell::PageOrder::kFifth; }
    [[nodiscard]] auto widget() -> QWidget * override { return this; }

signals:
    void openRequested();
};

} // namespace ColorPicker
