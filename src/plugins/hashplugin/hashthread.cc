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
    std::atomic_bool running{false};
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
    if (isRunning()) {
        return false;
    }
    d_ptr->input = input;
    d_ptr->algorithm = algorithm;
    d_ptr->running.store(true);
    start();
    return true;
}

void HashThread::stop()
{
    d_ptr->running.store(false);
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
        while (d_ptr->running.load() && !file.atEnd()) {
            auto data = file.read(1024 * 1024); // 1MB
            hashObj.addData(data);
        }
    } else {
        hashObj.addData(d_ptr->input.toUtf8());
    }
    emit hashFinished(hashObj.result().toHex());
}

} // namespace Plugin
