// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#pragma once

#include "aggregation_global.h"

#include <QObject>
#include <QList>
#include <QHash>
#include <QReadWriteLock>
#include <QReadLocker>

namespace Aggregation {

class AGGREGATION_EXPORT Aggregate : public QObject
{
    Q_OBJECT

public:
    explicit Aggregate(QObject *parent = nullptr);
    ~Aggregate() override;

    void add(QObject *component);
    void remove(QObject *component);

    template <typename T> auto component() -> T * {
        QReadLocker locker(&lock());
        for (QObject *component : std::as_const(m_components)) {
            if (T *result = qobject_cast<T *>(component))
                return result;
        }
        return nullptr;
    }

    template <typename T> auto components() -> QList<T *> {
        QReadLocker locker(&lock());
        QList<T *> results;
        for (QObject *component : std::as_const(m_components)) {
            if (T *result = qobject_cast<T *>(component)) {
                results << result;
            }
        }
        return results;
    }

    static auto parentAggregate(QObject *obj) -> Aggregate *;
    static auto lock() -> QReadWriteLock &;

signals:
    void changed();

private:
    void deleteSelf(QObject *obj);

    static auto aggregateMap() -> QHash<QObject *, Aggregate *> &;

    QList<QObject *> m_components;
};

// get a component via global template function
template <typename T> auto query(Aggregate *obj) -> T *
{
    if (!obj)
        return nullptr;
    return obj->template component<T>();
}

template <typename T> auto query(QObject *obj) -> T *
{
    if (!obj)
        return nullptr;
    T *result = qobject_cast<T *>(obj);
    if (!result) {
        QReadLocker locker(&Aggregate::lock());
        Aggregate *parentAggregation = Aggregate::parentAggregate(obj);
        result = (parentAggregation ? query<T>(parentAggregation) : nullptr);
    }
    return result;
}

// get all components of a specific type via template function
template <typename T> auto query_all(Aggregate *obj) -> QList<T *>
{
    if (!obj)
        return QList<T *>();
    return obj->template components<T>();
}

template <typename T> auto query_all(QObject *obj) -> QList<T *>
{
    if (!obj)
        return QList<T *>();
    QReadLocker locker(&Aggregate::lock());
    Aggregate *parentAggregation = Aggregate::parentAggregate(obj);
    QList<T *> results;
    if (parentAggregation)
        results = query_all<T>(parentAggregation);
    else if (T *result = qobject_cast<T *>(obj))
        results.append(result);
    return results;
}

} // namespace Aggregation
