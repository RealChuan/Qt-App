#ifndef SYSTEMINFOWIDGET_HPP
#define SYSTEMINFOWIDGET_HPP

#include <QWidget>

namespace Plugin {

class SystemInfoWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SystemInfoWidget(QWidget *parent = nullptr);

private:
    void setupUI();
};

} // namespace Plugin

#endif // SYSTEMINFOWIDGET_HPP
