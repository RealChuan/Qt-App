#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QTcpServer>

namespace Plugin {

class TcpServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit TcpServer(QObject *parent = nullptr);
    ~TcpServer() override;

    void sendMessage(const QByteArray &bytes, const QString &clientInfo = "");

signals:
    void errorMessage(const QString &);
    void newClientInfo(const QString &);
    void disconnectClientInfo(const QString &);
    void clientMessage(const QString &, const QByteArray &);

private slots:
    void onError();
    void onNewConnect();
    void onClientError(QAbstractSocket::SocketError);
    void onClientDisconnect();
    void onClientReadyRead();

private:
    void buildConnect();

    class TcpServerPrivate;
    QScopedPointer<TcpServerPrivate> d_ptr;
};

} // namespace Plugin

#endif // TCPSERVER_H
