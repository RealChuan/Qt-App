// Copyright (C) 2024 Jarek Kobus
// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef TASKING_NETWORKQUERY_H
#define TASKING_NETWORKQUERY_H

#include "tasking_global.h"

#include "tasktree.h"

#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>

#include <memory>

QT_BEGIN_NAMESPACE
class QNetworkAccessManager;

namespace Tasking {

// This class introduces the dependency to Qt::Network, otherwise Tasking namespace
// is independent on Qt::Network.
// Possibly, it could be placed inside Qt::Network library, as a wrapper around QNetworkReply.

enum class NetworkOperation { Get, Put, Post, Delete };

class TASKING_EXPORT NetworkQuery final : public QObject
{
    Q_OBJECT

public:
    ~NetworkQuery();
    void setRequest(const QNetworkRequest &request) { m_request = request; }
    void setOperation(NetworkOperation operation) { m_operation = operation; }
    void setWriteData(const QByteArray &data) { m_writeData = data; }
    void setNetworkAccessManager(QNetworkAccessManager *manager) { m_manager = manager; }
    QNetworkReply *reply() const { return m_reply.get(); }
    void start();

Q_SIGNALS:
    void started();
    void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
#if QT_CONFIG(ssl)
    void sslErrors(const QList<QSslError> &errors);
#endif
    void done(DoneResult result);

private:
    QNetworkRequest m_request;
    NetworkOperation m_operation = NetworkOperation::Get;
    QByteArray m_writeData; // Used by Put and Post
    QNetworkAccessManager *m_manager = nullptr;
    std::unique_ptr<QNetworkReply> m_reply;
};

using NetworkQueryTask = SimpleCustomTask<NetworkQuery>;

} // namespace Tasking

QT_END_NAMESPACE

#endif // TASKING_NETWORKQUERY_H
