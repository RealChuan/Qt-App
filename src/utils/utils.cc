#include "utils.hpp"

#include <QApplication>
#include <QCryptographicHash>
#include <QDir>
#include <QElapsedTimer>
#include <QProcess>
#include <QRandomGenerator>
#include <QSysInfo>
#include <QThread>

#include <array>
#include <ranges>
#include <vector>

using namespace Qt::StringLiterals;

namespace Utils {

namespace {

auto compilerString() -> QString
{
#if defined(__apple_build_version__) // Apple clang has other version numbers
    return u"Clang %1.%2 (Apple)"_s.arg(__clang_major__).arg(__clang_minor__);
#elif defined(Q_CC_GNU)
    return u"GCC %1"_s.arg(QLatin1StringView(__VERSION__));
#elif defined(Q_CC_MSVC)
    return u"MSVC Version: %1"_s.arg(_MSC_VER);
#else
    return u"<unknown compiler>"_s;
#endif
}

} // namespace

auto systemInfo() -> QString
{
    const QStringList lines{
        u"%1 (%2) on %3 (%4) with CPU Cores: %5"_s.arg(
            QSysInfo::prettyProductName(),
            QSysInfo::kernelVersion(),
            QSysInfo::currentCpuArchitecture(),
            QSysInfo::machineHostName(),
            QString::number(QThread::idealThreadCount())),
        u"Build with: Qt %1 (%2, %3)"_s
            .arg(QString::fromLatin1(qVersion()), compilerString(), QSysInfo::buildAbi()),
    };
    return lines.join(u"\n"_s);
}

void reboot()
{
    QProcess::startDetached(
        QApplication::applicationFilePath(), QApplication::arguments(), QDir::currentPath());
    QApplication::exit();
}

void quitApplication()
{
    QMetaObject::invokeMethod(qApp, &QApplication::quit, Qt::QueuedConnection);
}

auto formatBytes(qint64 bytes, int precision) -> QString
{
    static constexpr std::array<const char *, 9>
        units{"B", "KB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB"};
    constexpr double unit = 1024.0;

    double size = static_cast<double>(bytes);
    std::size_t index = 0;
    while (size >= unit && index + 1 < units.size()) {
        size /= unit;
        ++index;
    }

    return u"%1 %2"_s.arg(QString::number(size, 'f', precision), QLatin1StringView(units[index]));
}

namespace {

auto generateRandomData(int size) -> QByteArray
{
    const auto numWords = static_cast<std::size_t>((size + sizeof(quint32) - 1) / sizeof(quint32));
    std::vector<quint32> buffer(numWords);
    QRandomGenerator::global()->fillRange(buffer.data(), qsizetype(numWords));

    QByteArray
        data(reinterpret_cast<const char *>(buffer.data()), qsizetype(numWords * sizeof(quint32)));
    data.resize(size);
    return data;
}

auto hashBenchOnce(int durationMs, QCryptographicHash::Algorithm algorithm, const QByteArray &data)
    -> double
{
    QElapsedTimer timer;
    timer.start();

    qint64 totalBytes = 0;
    while (timer.elapsed() < durationMs) {
        // 只算输入字节：摘要输出不属于被处理的吞吐量。
        QCryptographicHash::hash(data, algorithm);
        totalBytes += data.size();
    }

    const double elapsedSeconds = timer.elapsed() / 1000.0;
    return elapsedSeconds <= 0.0 ? 0.0 : (double(totalBytes) / elapsedSeconds) / (1024.0 * 1024.0);
}

} // namespace

auto hashBench(int iterations, int durationMs, int dataSize, QCryptographicHash::Algorithm algorithm)
    -> double
{
    // 数据只生成一次：每轮重新填随机数会把内存分配算进耗时，短测试尤其明显。
    const QByteArray data = generateRandomData(dataSize);

    // 多轮取最好值：第一轮包含升频，取平均反而把噪声算进来。
    double best = 0.0;
    for (int i = 0; i < iterations; ++i)
        best = (std::ranges::max) (best, hashBenchOnce(durationMs, algorithm, data));
    return best;
}

} // namespace Utils
