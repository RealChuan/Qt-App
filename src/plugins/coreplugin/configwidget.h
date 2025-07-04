#pragma once

#include <QWidget>

class ConfigWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ConfigWidget(QWidget *parent = nullptr);
    ~ConfigWidget() override;

private slots:
    void onReloadLanguage(int index);
    void onMaskCheckStateChanged(Qt::CheckState state);
    void onBlurCheckStateChanged(Qt::CheckState state);

protected:
    void changeEvent(QEvent *event) override;

private:
    void buildConnect();
    void setTr();

    class ConfigWidgetPrivate;
    QScopedPointer<ConfigWidgetPrivate> d_ptr;
};
