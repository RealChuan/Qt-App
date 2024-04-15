#ifndef LOGASYNC_H
#define LOGASYNC_H

#include <QThread>

#include "singleton.hpp"
#include "utils_global.h"

namespace Utils {

class FileUtil : public QObject
{
    Q_OBJECT
public:
    explicit FileUtil(qint64 days = 7, QObject *parent = nullptr);
    ~FileUtil() override;

public slots:
    void onWrite(const QString & /*msg*/);

private slots:
    void onFlush();

private:
    auto rollFile(int /*count*/) -> bool;
    void autoDelFile();
    void setTimer();

    class FileUtilPrivate;
    QScopedPointer<FileUtilPrivate> d_ptr;
};

struct LogAsyncPrivate;
class UTILS_EXPORT LogAsync : public QThread
{
    Q_OBJECT
public:
    enum Orientation { Std = 1, File, StdAndFile };

    void setOrientation(Orientation /*orientation*/);
    auto orientation() -> Orientation;

    void setLogLevel(QtMsgType /*type*/);
    auto logLevel() -> QtMsgType;

    void startWork();
    void stop();

signals:
    void appendBuf(const QString &);

protected:
    void run() override;

private:
    explicit LogAsync(QObject *parent = nullptr);
    ~LogAsync() override;

    QScopedPointer<LogAsyncPrivate> d_ptr;

    SINGLETON(LogAsync)
};

} // namespace Utils

#endif // LOGASYNC_H
