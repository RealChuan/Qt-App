#ifndef SERIALSETTINGS_HPP
#define SERIALSETTINGS_HPP

#include <QSerialPort>

namespace Plugin {

struct SerialSettings
{
    QString portName;
    QSerialPort::BaudRate baudRate = QSerialPort::Baud9600;
    QSerialPort::DataBits dataBits = QSerialPort::Data8;
    QSerialPort::Parity parity = QSerialPort::NoParity;
    QSerialPort::StopBits stopBits = QSerialPort::OneStop;
    QSerialPort::FlowControl flowControl = QSerialPort::NoFlowControl;
};

} // namespace Plugin

#endif // SERIALSETTINGS_HPP
