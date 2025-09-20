// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "stringtable.h"

#include "async.h"

#include <QCoreApplication>
#include <QDebug>
#include <QElapsedTimer>
#include <QMutex>
#include <QSet>
#include <QTimer>

// FIXME: Provide tst_StringTable that would run GC, make a sleep inside GC being run in other
// thread and execute destructor of StringTable in main thread. In this case the test should
// ensure that destructor of StringTable waits for its internal thread to finish.

using namespace std::chrono;

namespace Utils::StringTable {

enum { DebugStringTable = 0 };

static std::atomic_bool s_isScheduled = false;

class StringTablePrivate
{
public:
    StringTablePrivate() { m_strings.reserve(1000); }
    ~StringTablePrivate() { cancelAndWait(); }

    void cancelAndWait();
    QString insert(const QString &string);
    void startGC();
    void GC(QPromise<void> &promise);

    QFuture<void> m_future;
    QMutex m_lock;
    QSet<QString> m_strings;
};

static StringTablePrivate &stringTable()
{
    static StringTablePrivate theStringTable;
    return theStringTable;
}

UTILS_EXPORT QString insert(const QString &string)
{
    return stringTable().insert(string);
}

void StringTablePrivate::cancelAndWait()
{
    if (!m_future.isRunning())
        return;
    m_future.cancel();
    m_future.waitForFinished();
}

QString StringTablePrivate::insert(const QString &string)
{
    if (string.isEmpty())
        return string;

    QMutexLocker locker(&m_lock);
    // From this point of time any possible new call to startGC() will be held until
    // we finish this function. So we are sure that after canceling the running GC() method now,
    // no new call to GC() will be executed until we finish this function.
    cancelAndWait();
    // A possibly running GC() thread already finished, so it's safe to modify m_strings from
    // now until we unlock the mutex.
    return *m_strings.insert(string);
}

void StringTablePrivate::startGC()
{
    s_isScheduled.exchange(false);
    QMutexLocker locker(&m_lock);
    cancelAndWait();
    m_future = Utils::asyncRun(&StringTablePrivate::GC, this);
}

UTILS_EXPORT void scheduleGC()
{
    if (!s_isScheduled.exchange(true))
        QTimer::singleShot(10s, qApp, [] { stringTable().startGC(); });
}

// qtbase/3f61f736266ece40d627dcf6214618a22a009fd1 changed QArrayData::{ref_ → m_ref};
// adapt:
template<typename S>
auto getQArrayDataRef(const S *s) -> decltype(s->ref_)
{
    return s->ref_.loadRelaxed();
}
template<typename S>
auto getQArrayDataRef(const S *s) -> decltype(s->m_ref)
{
    return s->m_ref.loadRelaxed();
}

static inline bool isDetached(const QString &string, int &bytesSaved)
{
    if (DebugStringTable) {
        QStringPrivate &data_ptr = const_cast<QString &>(string).data_ptr();
        const int ref = getQArrayDataRef(data_ptr->d_ptr());
        bytesSaved += (ref - 1) * string.size();
        if (ref > 10)
            qDebug() << ref << string.size() << string.left(50);
    }
    return string.isDetached();
}

void StringTablePrivate::GC(QPromise<void> &promise)
{
    int initialSize = 0;
    int bytesSaved = 0;
    QElapsedTimer timer;
    if (DebugStringTable) {
        initialSize = m_strings.size();
        timer.start();
    }

    // Collect all QStrings which have refcount 1. (One reference in m_strings and nowhere else.)
    for (QSet<QString>::iterator i = m_strings.begin(); i != m_strings.end();) {
        if (promise.isCanceled())
            return;

        if (isDetached(*i, bytesSaved))
            i = m_strings.erase(i);
        else
            ++i;
    }

    if (DebugStringTable) {
        const int currentSize = m_strings.size();
        qDebug() << "StringTable::GC removed" << initialSize - currentSize << "strings in"
                 << timer.elapsed() << "ms, size is now" << currentSize << "saved: " << bytesSaved
                 << "bytes";
    }
}

} // namespace Utils::StringTable
