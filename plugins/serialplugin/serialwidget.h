#ifndef SERIALWIDGET_H
#define SERIALWIDGET_H

#include <QWidget>

namespace Plugin {

class SerialWidget : public QWidget
{
    Q_OBJECT
public:
    enum MessageType { Send, Recv, SuccessInfo, ErrorInfo };

    explicit SerialWidget(QWidget *parent = nullptr);
    ~SerialWidget() override;

private slots:
    void onSendData();
    void onParamChanged(const QString &);

    void onOpenOrCloseSerial(bool);

    void onLine(bool);
    void onAppendError(const QString &);
    void onSerialRecvMessage(const QByteArray &bytes);

    void onAutoSend(bool);
    void onSave();

private:
    void buildConnect();

    class SerialWidgetPrivate;
    QScopedPointer<SerialWidgetPrivate> d_ptr;
};

} // namespace Plugin

#endif // SERIALWIDGET_H
