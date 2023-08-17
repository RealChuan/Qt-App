#include "hashthread.hpp"

#include <QFile>

namespace Plugin {

class HashThread::HashThreadPrivate
{
public:
    explicit HashThreadPrivate(HashThread *q)
        : q_ptr(q)
    {}

    HashThread *q_ptr;

    QString input;
    QCryptographicHash::Algorithm algorithm;
};

HashThread::HashThread(QObject *parent)
    : QThread{parent}
    , d_ptr{new HashThreadPrivate{this}}
{}

HashThread::~HashThread()
{
    stop();
}

auto HashThread::startHash(const QString &input, QCryptographicHash::Algorithm algorithm) -> bool
{
    d_ptr->input = input;
    d_ptr->algorithm = algorithm;
    if (isRunning()) {
        return false;
    }
    start();
    return true;
}

void HashThread::stop()
{
    if (isRunning()) {
        quit();
        wait();
    }
}

void HashThread::run()
{
    QCryptographicHash hashObj{d_ptr->algorithm};
    QFile file{d_ptr->input};
    if (file.exists() && file.open(QIODevice::ReadOnly)) {
        hashObj.addData(&file);
        file.close();
    } else {
        hashObj.addData(d_ptr->input.toUtf8());
    }
    auto result = hashObj.result();
    emit hashFinished(result.toHex());
}

} // namespace Plugin
