#ifndef CONFIGWIDGET_H
#define CONFIGWIDGET_H

#include <QWidget>

class ConfigWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ConfigWidget(QWidget *parent = nullptr);
    ~ConfigWidget() override;

private slots:
    void onReloadLanguage(int index);

protected:
    void changeEvent(QEvent *e) override;

private:
    void buildConnect();
    void setTr();

    class ConfigWidgetPrivate;
    QScopedPointer<ConfigWidgetPrivate> d_ptr;
};

#endif // CONFIGWIDGET_H
