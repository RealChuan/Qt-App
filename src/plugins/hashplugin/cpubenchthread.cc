#include "cpubenchthread.hpp"

#include <utils/utils.h>

namespace Plugin {

class CpuBenchThread::CpuBenchThreadPrivate
{
public:
    explicit CpuBenchThreadPrivate(CpuBenchThread *parent)
        : q_ptr{parent}
    {}

    CpuBenchThread *q_ptr;

    Params params;
};

CpuBenchThread::CpuBenchThread(QObject *parent)
    : QThread{parent}
    , d_ptr{new CpuBenchThreadPrivate{this}}
{}

CpuBenchThread::~CpuBenchThread()
{
    stop();
}

void CpuBenchThread::startBench(const Params &params)
{
    if (isRunning()) {
        return;
    }
    d_ptr->params = params;
    start();
}

void CpuBenchThread::stop()
{
    if (isRunning()) {
        quit();
        wait();
    }
}

void CpuBenchThread::run()
{
    auto result = Utils::cpuBench(d_ptr->params.iterations,
                                  d_ptr->params.durationMs,
                                  d_ptr->params.dataSize,
                                  d_ptr->params.algorithm);
    emit benchFinished(result);
}

} // namespace Plugin
