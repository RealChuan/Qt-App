#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QTcpSocket>

namespace Plugin {

class TcpClient : public QTcpSocket
{
    Q_OBJECT
public:
    explicit TcpClient(const QString &ip, quint16 port, QObject *parent = nullptr);
    ~TcpClient() override;

    void setIpPort(const QString &ip, quint16 port);

    void connectToServer();
    void connectToServer(const QString &ip, quint16 port);
    void closeSocket();

    auto isConnected() -> bool;

signals:
    void errorMessage(const QString &);
    void serverMessage(const QByteArray &);
    void socketStateChanged(const QString &text, bool onLine);

private slots:
    void onError(QAbstractSocket::SocketError socketError);
    void onReadyRead();
    void onStateChange(QAbstractSocket::SocketState socketState);

private:
    void buildConnect();

    class TcpClientPrivate;
    QScopedPointer<TcpClientPrivate> d_ptr;
};

} // namespace Plugin

#endif // TCPCLIENT_H
