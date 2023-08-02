#include "serialport.h"

#include <QDebug>

namespace Plugin {

SerialPort::SerialPort(QObject *parent)
    : QSerialPort(parent)
{
    buildConnect();
}

SerialPort::~SerialPort()
{
    closeSerialPort();
}

bool SerialPort::openSerialPort(const SerialSettings &param)
{
    if (param.portName.isEmpty()) {
        emit errorMessage(tr("Serial Error: PortName is Empty!"));
        return false;
    }

    closeSerialPort();

    bool ok = true;
    setPortName(param.portName);
    ok &= open(QIODevice::ReadWrite);
    flush();
    ok &= setBaudRate(static_cast<QSerialPort::BaudRate>(param.baudRate));
    ok &= setDataBits(static_cast<QSerialPort::DataBits>(param.dataBits));
    ok &= setStopBits(static_cast<QSerialPort::StopBits>(param.stopBits));
    ok &= setParity(static_cast<QSerialPort::Parity>(param.parity));
    ok &= setFlowControl(static_cast<QSerialPort::FlowControl>(param.flowControl));

    if (!ok) {
        onError();
    }
    emit onLine(ok);
    return ok;
}

void SerialPort::closeSerialPort()
{
    if (isOpen()) {
        close();
        emit onLine(false);
    }
}

void SerialPort::onError()
{
    if (error() == QSerialPort::NoError) {
        return;
    }
    const auto err = tr("Serial Error[%1]: %2.").arg(QString::number(error()), errorString());
    emit errorMessage(err);
}

void SerialPort::onReadyRead()
{
    if (bytesAvailable() <= 0) {
        return;
    }

    QByteArray bytes;
    while (!atEnd()) {
        bytes += readAll();
    }
    emit message(bytes);
}

void SerialPort::buildConnect()
{
    connect(this, &SerialPort::errorOccurred, this, &SerialPort::onError);
    connect(this, &SerialPort::readyRead, this, &SerialPort::onReadyRead);
}

} // namespace Plugin
