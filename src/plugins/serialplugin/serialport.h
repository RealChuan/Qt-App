#ifndef SERIALPORT_H
#define SERIALPORT_H

#include "serialsettings.hpp"

namespace Plugin {

class SerialPort : public QSerialPort
{
    Q_OBJECT
public:
    explicit SerialPort(QObject *parent = nullptr);
    ~SerialPort() override;

    auto openSerialPort(const SerialSettings &param) -> bool;
    void closeSerialPort();

signals:
    void onLine(bool);
    void errorMessage(const QString &);
    void message(const QByteArray &);

private slots:
    void onError();
    void onReadyRead();

private:
    void buildConnect();
};

} // namespace Plugin

#endif // SERIALPORT_H
