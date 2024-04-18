#ifndef ABOUTWIDGET_HPP
#define ABOUTWIDGET_HPP

#include <QWidget>

namespace Plugin {

class AboutWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AboutWidget(QWidget *parent = nullptr);

private slots:
    void setupUI();
};

} // namespace Plugin

#endif // ABOUTWIDGET_HPP
