#ifndef HASHWIDGET_HPP
#define HASHWIDGET_HPP

#include <QWidget>

namespace Plugin {

class HashWidget : public QWidget
{
    Q_OBJECT
public:
    explicit HashWidget(QWidget *parent = nullptr);
    ~HashWidget();

private slots:
    void onSelectFile();
    void onCalculate();
    void onHashFinished(const QString &result);

private:
    void buildConnect();

    class HashWidgetPrivate;
    QScopedPointer<HashWidgetPrivate> d_ptr;
};

} // namespace Plugin

#endif // HASHWIDGET_HPP
