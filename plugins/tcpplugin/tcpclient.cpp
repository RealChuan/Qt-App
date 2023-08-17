#include "tcpclient.h"

namespace Plugin {

class TcpClient::TcpClientPrivate
{
public:
    explicit TcpClientPrivate(TcpClient *q)
        : q_ptr(q)
    {}

    TcpClient *q_ptr;

    QString ip;
    quint16 port;
};

TcpClient::TcpClient(const QString &ip, quint16 port, QObject *parent)
    : QTcpSocket(parent)
    , d_ptr(new TcpClientPrivate(this))
{
    buildConnect();
    setIpPort(ip, port);
}

TcpClient::~TcpClient()
{
    closeSocket();
}

void TcpClient::setIpPort(const QString &ip, quint16 port)
{
    d_ptr->ip = ip;
    d_ptr->port = port;
}

void TcpClient::connectToServer()
{
    if (isConnected()) {
        return;
    }
    connectToHost(d_ptr->ip, d_ptr->port);
    //waitForConnected(1000);
}

void TcpClient::connectToServer(const QString &ip, quint16 port)
{
    setIpPort(ip, port);
    connectToServer();
}

void TcpClient::closeSocket()
{
    if (isOpen()) {
        disconnectFromHost();
        close();
    }
}

auto TcpClient::isConnected() -> bool
{
    return state() == QAbstractSocket::ConnectedState;
}

void TcpClient::onError(SocketError socketError)
{
    const auto err = tr("[Client Error]: %1,%2").arg(QString::number(socketError), errorString());
    emit errorMessage(err);
}

void TcpClient::onReadyRead()
{
    if (bytesAvailable() <= 0) {
        return;
    }
    QByteArray bytes;
    while (!atEnd()) {
        bytes += readAll();
    }
    emit serverMessage(bytes);
}

void TcpClient::onStateChange(QAbstractSocket::SocketState socketState)
{
    switch (socketState) {
    case QAbstractSocket::UnconnectedState:
        emit socketStateChanged(tr("The socket is not connected."), false);
        break;
    case QAbstractSocket::HostLookupState:
        emit socketStateChanged(tr("The socket is performing a host name lookup."), false);
        break;
    case QAbstractSocket::ConnectingState:
        emit socketStateChanged(tr("The socket has started establishing a connection."), false);
        break;
    case QAbstractSocket::ConnectedState:
        emit socketStateChanged(tr("A connection is established."), true);
        break;
    case QAbstractSocket::BoundState:
        emit socketStateChanged(tr("The socket is bound to an address and port."), false);
        break;
    case QAbstractSocket::ClosingState:
        emit socketStateChanged(
            tr("The socket is about to close (data may still be waiting to be written)."), false);
        break;
    case QAbstractSocket::ListeningState:
        emit socketStateChanged(tr("For internal use only."), false);
        break;
    default: break;
    }
}

void TcpClient::buildConnect()
{
    connect(this, &TcpClient::readyRead, this, &TcpClient::onReadyRead);
    connect(this, &TcpClient::stateChanged, this, &TcpClient::onStateChange);
    connect(this, &TcpClient::errorOccurred, this, &TcpClient::onError);
}

} // namespace Plugin
