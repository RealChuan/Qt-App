#ifndef HELLOWIDGET_HPP
#define HELLOWIDGET_HPP

#include <QWidget>

namespace Plugin {

class HelloWidget : public QWidget
{
    Q_OBJECT
public:
    explicit HelloWidget(QWidget *parent = nullptr);
    ~HelloWidget() override;

private slots:
    void onPrevious();
    void onNext();

protected:
    void changeEvent(QEvent *event) override;

private:
    void setupTr();
    void buildConnect();

    class HelloWidgetPrivate;
    QScopedPointer<HelloWidgetPrivate> d_ptr;
};

} // namespace Plugin

#endif // HELLOWIDGET_HPP
