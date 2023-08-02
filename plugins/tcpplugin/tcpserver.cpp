#include "tcpserver.h"

#include <QTcpSocket>

namespace Plugin {

class TcpServer::TcpServerPrivate
{
public:
    TcpServerPrivate(TcpServer *q)
        : q_ptr(q)
    {}

    TcpServer *q_ptr;

    QVector<QTcpSocket *> tcpClients;
};

TcpServer::TcpServer(QObject *parent)
    : QTcpServer(parent)
    , d_ptr(new TcpServerPrivate(this))
{
    buildConnect();
}

TcpServer::~TcpServer()
{
    if (isListening()) {
        close();
    }
}

void TcpServer::sendMessage(const QByteArray &bytes, const QString &clientInfo)
{
    if (bytes.isEmpty()) {
        return;
    }

    if (d_ptr->tcpClients.isEmpty()) {
        auto error = tr(
            "There is currently no client online, sending failed, please stop sending!");
        emit errorMessage(error);
        return;
    }

    if (clientInfo.isEmpty()) {
        for (auto client : qAsConst(d_ptr->tcpClients)) {
            client->write(bytes);
        }
        return;
    }

    auto clientIP = clientInfo.split(":")[0].trimmed();
    auto clientPort = clientInfo.split(":")[1].toInt();
    for (auto client : qAsConst(d_ptr->tcpClients)) {
        if (client->peerAddress().toString().split("::ffff:")[0] == clientIP
            && client->peerPort() == clientPort)
            client->write(bytes);
    }
}

void TcpServer::onError()
{
    auto err = tr("TCPServer accept Error: %1").arg(errorString());
    emit errorMessage(err);
}

void TcpServer::onNewConnect()
{
    auto client = nextPendingConnection();
    client->setParent(this); //利用Qt的对象树进行析构
    d_ptr->tcpClients.append(client);

    auto clientInfo = tr("%1 : %2").arg(client->peerAddress().toString().split("::ffff:")[0],
                                        QString::number(client->peerPort()));
    emit newClientInfo(clientInfo);

    connect(client, &QTcpSocket::errorOccurred, this, &TcpServer::onClientError);
    connect(client, &QTcpSocket::disconnected, this, &TcpServer::onClientDisconnect);
    connect(client, &QTcpSocket::readyRead, this, &TcpServer::onClientReadyRead);
}

void TcpServer::onClientError(QAbstractSocket::SocketError)
{
    auto client = qobject_cast<QTcpSocket *>(sender());
    if (nullptr == client) {
        return;
    }

    auto err = tr("Client [%1 : %2] Error: %3.")
                   .arg(client->peerAddress().toString().split("::ffff:")[0],
                        QString::number(client->peerPort()),
                        client->errorString());
    emit errorMessage(err);
}

void TcpServer::onClientDisconnect()
{
    auto client = qobject_cast<QTcpSocket *>(sender());
    if (nullptr == client) {
        return;
    }

    if (client->state() == QAbstractSocket::UnconnectedState) {
        auto clientInfo = tr("%1 : %2").arg(client->peerAddress().toString().split("::ffff:")[0],
                                            QString::number(client->peerPort()));
        emit disconnectClientInfo(clientInfo);
        d_ptr->tcpClients.removeOne(client);
        client->deleteLater();
    }
}

void TcpServer::onClientReadyRead()
{
    auto client = qobject_cast<QTcpSocket *>(sender());
    if (nullptr == client) {
        return;
    }
    if (client->bytesAvailable() <= 0) {
        return;
    }

    QByteArray bytes;
    while (!client->atEnd()) {
        bytes += client->readAll();
    }

    auto clientInfo = tr("Client [%1 : %2] : ")
                          .arg(client->peerAddress().toString().split("::ffff:")[0],
                               QString::number(client->peerPort()));
    emit clientMessage(clientInfo, bytes);
}

void TcpServer::buildConnect()
{
    connect(this, &TcpServer::acceptError, this, &TcpServer::onError);
    connect(this, &TcpServer::newConnection, this, &TcpServer::onNewConnect);
}

} // namespace Plugin
