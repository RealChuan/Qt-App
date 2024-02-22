#ifndef TCPWIDGET_H
#define TCPWIDGET_H

#include <QWidget>

namespace Plugin {

class TcpWidget : public QWidget
{
    Q_OBJECT
public:
    enum MessageType { Send, Recv, SuccessInfo, ErrorInfo };

    explicit TcpWidget(QWidget *parent = nullptr);
    ~TcpWidget() override;

private slots:
    void onModelChange(const QString & /*text*/);
    void onListenOrConnect(bool /*state*/);
    void onSendData();
    void onAppendError(const QString & /*error*/);

    void onServerOnline(bool /*state*/);
    void onServerNewClient(const QString & /*clientInfo*/);
    void onServerDisconnectClient(const QString & /*clientInfo*/);
    void onServerRecvMessage(const QString & /*clientInfo*/, const QByteArray & /*bytes*/);

    void onClientStateChanged(const QString &text, bool onLine);
    void onClientRecvMessage(const QByteArray & /*bytes*/);
    void onAutoReconnectStartOrStop(bool /*state*/);
    void onAutoConnect();

    void onAutoSend(bool /*state*/);
    void onSave();

private:
    void buildConnect();

    void resetTcpServer();
    void resetTcpClient();

    class TcpWidgetPrivate;
    QScopedPointer<TcpWidgetPrivate> d_ptr;
};

} // namespace Plugin

#endif // TCPWIDGET_H
