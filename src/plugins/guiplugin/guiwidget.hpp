#ifndef GUIWIDGET_HPP
#define GUIWIDGET_HPP

#include <QWidget>

class QGroupBox;

namespace Plugin {

class GuiWidget : public QWidget
{
    Q_OBJECT
public:
    explicit GuiWidget(QWidget *parent = nullptr);

private:
    void setupUI();
    auto createButtonGroup() -> QGroupBox *;
    auto createInputGroup() -> QGroupBox *;
    auto createBarGroup() -> QGroupBox *;
    auto createMenu() -> QMenu *;
};

} // namespace Plugin

#endif // GUIWIDGET_HPP
