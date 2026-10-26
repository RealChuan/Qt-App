#pragma once

#include <shell/page.hpp>

#include <QWidget>

#include <memory>

class QEvent;

using namespace Qt::StringLiterals;

namespace ColorPicker {

class ColorPickerPage final : public QWidget, public Shell::IPage
{
    Q_OBJECT

public:
    explicit ColorPickerPage(QWidget *parent = nullptr);
    ~ColorPickerPage() override;

    [[nodiscard]] auto id() const -> QString override { return u"colorpickerplugin.page"_s; }
    [[nodiscard]] auto title() const -> QString override { return tr("Color Picker"); }
    [[nodiscard]] auto subtitle() const -> QString override
    {
        return tr("Sample any pixel on screen and copy its HEX value.");
    }
    [[nodiscard]] auto order() const -> int override { return Shell::PageOrder::kFifth; }
    [[nodiscard]] auto widget() -> QWidget * override { return this; }

signals:
    void openRequested();

protected:
    void changeEvent(QEvent *event) override;

private:
    void setupUi();
    void retranslateUi();
    void setupConnections();

    class ColorPickerPagePrivate;
    std::unique_ptr<ColorPickerPagePrivate> d_ptr;
};

} // namespace ColorPicker
