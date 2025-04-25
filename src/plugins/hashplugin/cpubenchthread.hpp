#pragma once

#include <QCryptographicHash>
#include <QThread>

namespace Plugin {

class CpuBenchThread : public QThread
{
    Q_OBJECT
public:
    struct Params
    {
        int iterations = 10;
        int durationMs = 1000;
        int dataSize = 1024 * 1024; // 1MB
        QCryptographicHash::Algorithm algorithm = QCryptographicHash::Sha256;
    };

    explicit CpuBenchThread(QObject *parent = nullptr);
    ~CpuBenchThread() override;

    void startBench(const Params &params);
    void stop();

signals:
    void benchFinished(double result); // MB/s

protected:
    void run() override;

private:
    class CpuBenchThreadPrivate;
    QScopedPointer<CpuBenchThreadPrivate> d_ptr;
};

} // namespace Plugin
