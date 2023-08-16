#ifndef HASHTHREAD_HPP
#define HASHTHREAD_HPP

#include <QCryptographicHash>
#include <QThread>

namespace Plugin {

class HashThread : public QThread
{
    Q_OBJECT
public:
    explicit HashThread(QObject *parent = nullptr);
    ~HashThread();

    bool startHash(const QString &input, QCryptographicHash::Algorithm algorithm);
    void stop();

signals:
    void hashFinished(const QString &hash);

protected:
    void run();

private:
    class HashThreadPrivate;
    QScopedPointer<HashThreadPrivate> d_ptr;
};

} // namespace Plugin

#endif // HASHTHREAD_HPP
