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
    void onModelChange(const QString &);
    void onListenOrConnect(bool);
    void onSendData();
    void onAppendError(const QString &);

    void onServerOnline(bool);
    void onServerNewClient(const QString &);
    void onServerDisconnectClient(const QString &);
    void onServerRecvMessage(const QString &, const QByteArray &);

    void onClientStateChanged(const QString &text, bool onLine);
    void onClientRecvMessage(const QByteArray &);
    void onAutoReconnectStartOrStop(bool);
    void onAutoConnect();

    void onAutoSend(bool);
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
