#pragma once

#include <QWidget>

namespace Plugin {

class HashWidget : public QWidget
{
    Q_OBJECT
public:
    explicit HashWidget(QWidget *parent = nullptr);
    ~HashWidget() override;

private slots:
    void onTestHash();
    void onTestBenchFinished(double result); // MB/s
    void onSelectFile();
    void onCalculate();
    void onHashFinished(const QString &result);

private:
    void buildConnect();

    class HashWidgetPrivate;
    QScopedPointer<HashWidgetPrivate> d_ptr;
};

} // namespace Plugin
