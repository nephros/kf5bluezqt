/****************************************************************************
**
** Copyright (C) 2016 Jolla Ltd.
** Contact: Bea Lam <bea.lam@jolla.com>
**
** This file is free software; you can redistribute it and/or
** modify it under the terms of the GNU Lesser General Public
** License as published by the Free Software Foundation; either
** version 2.1 of the License, or (at your option) any later version.
**
** This file is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public
** License along with this file; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
**
****************************************************************************/

#include "obexmanager_bluez4_p.h"
#include "obextransfer_bluez4_p.h"
#include "obexsession_bluez4_p.h"

#include "obexmanager_p.h"
#include "obexmanager.h"
#include "obexsession.h"
#include "obexsession_p.h"
#include "debug.h"
#include "utils.h"

#include "obextransfer_p.h"
#include "pendingcall.h"
#include "bluezqt_dbustypes.h"

#include <QDBusMessage>

namespace BluezQt
{

ObexManagerNotifier::ObexManagerNotifier(ObexManagerBluez4 *parent)
    : QDBusAbstractAdaptor(parent)
    , m_managerBluez4(parent)
{
    qDBusRegisterMetaType<QVariantMapList>();
}

void ObexManagerNotifier::notifyTransferAborted(const QString &transferPath)
{
    if (!transferPath.isEmpty()) {
        QDBusMessage call = QDBusMessage::createMethodCall(service(), objectPath(), interface(), QStringLiteral("setTransferAborted"));
        call << transferPath;

        if (!ObexManagerNotifier::connection().send(call)) {
            qCWarning(BLUEZQT) << "Request: Failed to notify manager of aborted transfer";
        }
    }
}

QVariantMapList ObexManagerNotifier::getSessions()
{
    return m_managerBluez4->sessionProperties();
}

QVariantMapList ObexManagerNotifier::getTransfers()
{
    return m_managerBluez4->transferProperties();
}

void ObexManagerNotifier::setTransferAborted(const QString &transferPath)
{
    m_managerBluez4->setTransferAborted(transferPath);
}


ObexManagerBluez4::ObexManagerBluez4(ObexManagerPrivate *obexManagerPrivate)
    : QObject(obexManagerPrivate)
    , m_obexManagerPrivate(obexManagerPrivate)
    , m_managerNotifier(0)
    , m_initializedSessions(false)
    , m_initializedTransfers(false)
{
    m_bluez4ObexClient = new Bluez4ObexClient(QStringLiteral("org.bluez.obex.client"), "/", DBusConnection::orgBluezObex(), this);
    m_bluez4ObexManager = new Bluez4ObexManager(Strings::orgBluezObex(), "/", DBusConnection::orgBluezObex(), this);

    qDBusRegisterMetaType<QVariantMapList>();
}

void ObexManagerBluez4::load()
{
    connect(m_bluez4ObexManager, &Bluez4ObexManager::TransferStarted,
            this, &ObexManagerBluez4::transferStarted);
    connect(m_bluez4ObexManager, &Bluez4ObexManager::TransferCompleted,
            this, &ObexManagerBluez4::transferCompleted);

    // Fetch known sessions from the "system" ObexManager
    QDBusMessage getSessionsCall = QDBusMessage::createMethodCall(ObexManagerNotifier::service(),
                                                                  ObexManagerNotifier::objectPath(),
                                                                  ObexManagerNotifier::interface(),
                                                                  QStringLiteral("getSessions"));

    QDBusPendingCallWatcher *getSessionsWatcher = new QDBusPendingCallWatcher(ObexManagerNotifier::connection().asyncCall(getSessionsCall), this);
    connect(getSessionsWatcher, &QDBusPendingCallWatcher::finished, this, &ObexManagerBluez4::getSessionsFinished);

    // Fetch known transfers from the "system" ObexManager, including those that are pending
    // Authorize() and therefore cannot normally be fetched as BlueZ 4 would not have emitted
    // transferStarted() for them yet.
    QDBusMessage getTransfersCall = QDBusMessage::createMethodCall(ObexManagerNotifier::service(),
                                                                   ObexManagerNotifier::objectPath(),
                                                                   ObexManagerNotifier::interface(),
                                                                   QStringLiteral("getTransfers"));

    QDBusPendingCallWatcher *getTransfersWatcher = new QDBusPendingCallWatcher(ObexManagerNotifier::connection().asyncCall(getTransfersCall), this);
    connect(getTransfersWatcher, &QDBusPendingCallWatcher::finished, this, &ObexManagerBluez4::getTransfersFinished);
}

void ObexManagerBluez4::getSessionsFinished(QDBusPendingCallWatcher *watcher)
{
    const QDBusPendingReply<QVariantMapList> &reply = *watcher;
    watcher->deleteLater();

    m_initializedSessions = true;

    if (!reply.isError()) {
        QVariantMapList::const_iterator it;
        const QVariantMapList &sessions = reply.value();

        for (it = sessions.constBegin(); it != sessions.constEnd(); ++it) {
            QVariantMap properties = *it;
            const QString &path = properties.value(obexSessionPathKey()).toString();

            m_obexManagerPrivate->addSession(path, properties);
        }
    } else if (reply.error().name() != QStringLiteral("org.freedesktop.DBus.Error.ServiceUnknown")) {
        // ServiceUnknown can be ignored as it occurs if registerAgent() has not yet been called on any
        // managers, which means this is probably the manager that will be used to register agents.
        qCWarning(BLUEZQT) << "Failed to find existing sessions:" << reply.error().message();
    }

    if (m_initializedSessions && m_initializedTransfers) {
        completeLoading();
    }
}

void ObexManagerBluez4::getTransfersFinished(QDBusPendingCallWatcher *watcher)
{
    const QDBusPendingReply<QVariantMapList> &reply = *watcher;
    watcher->deleteLater();

    m_initializedTransfers = true;

    if (!reply.isError()) {
        QVariantMapList::const_iterator it;
        const QVariantMapList &transfers = reply.value();

        for (it = transfers.constBegin(); it != transfers.constEnd(); ++it) {
            QVariantMap properties = *it;
            const QString &path = properties.value(obexTransferPathKey()).toString();

            if (!m_oppTransfers.contains(path)) {
                ObexTransferPtr transfer = ObexTransferPtr(new ObexTransfer(path, properties));
                transfer->d->q = transfer.toWeakRef();
                m_oppTransfers.insert(path, transfer);
            }
        }
    } else if (reply.error().name() != QStringLiteral("org.freedesktop.DBus.Error.ServiceUnknown")) {
        // ServiceUnknown can be ignored as it occurs if registerAgent() has not yet been called on any
        // managers, which means this is probably the manager that will be used to register agents.
        qCWarning(BLUEZQT) << "Failed to find existing transfers:" << reply.error().message();
    }

    if (m_initializedSessions && m_initializedTransfers) {
        completeLoading();
    }
}

void ObexManagerBluez4::completeLoading()
{
    if (!ObexManagerNotifier::connection().connect(QString(), QString(), ObexManagerNotifier::interface(),
                                                   QStringLiteral("objectPushTransferCreated"),
                                                   this, SLOT(objectPushTransferCreated(QString,QVariantMap,QString,QVariantMap)))) {
        qCWarning(BLUEZQT) << "Failed to connect to objectPushTransferCreated() signal";
    }
    if (!ObexManagerNotifier::connection().connect(QString(), QString(), ObexManagerNotifier::interface(),
                                                   QStringLiteral("objectPushTransferFinished"),
                                                   this, SLOT(objectPushTransferFinished(QString,QString,bool)))) {
        qCWarning(BLUEZQT) << "Failed to connect to objectPushTransferFinished() signal";
    }

    Q_EMIT loaded();
}

QDBusPendingReply<void> ObexManagerBluez4::registerAgent(const QDBusObjectPath &agentPath)
{
    if (!m_managerNotifier) {
        m_managerNotifier = new ObexManagerNotifier(this);

        // The first agent registered (assumed to be the system agent) will share the known
        // sessions and transfers with other agents.
        if (ObexManagerNotifier::connection().registerObject(ObexManagerNotifier::objectPath(), this)
                && ObexManagerNotifier::connection().registerService(ObexManagerNotifier::service())) {
            qCDebug(BLUEZQT) << "Registered system OBEX manager notifier for" << agentPath.path();
        } else {
            qCWarning(BLUEZQT) << "Error registering system OBEX manager notifier for" << agentPath.path();
        }
    }

    return m_bluez4ObexManager->RegisterAgent(agentPath);
}

QDBusPendingReply<void> ObexManagerBluez4::unregisterAgent(const QDBusObjectPath &agentPath)
{
    return m_bluez4ObexManager->UnregisterAgent(agentPath);
}

PendingCall *ObexManagerBluez4::createSession(const QString &destination, const QVariantMap &args)
{
    // Manually add/remove sessions as DBusObjectManager is not available in BlueZ 4 to detect when
    // they are created/removed.
    PendingCall *call = new PendingCall(m_bluez4ObexClient->CreateSession(destination, args),
                                        PendingCall::ReturnObjectPath, this);
    call->setProperty("destination", destination);
    call->setProperty("args", args);
    connect(call, &PendingCall::finished, this, &ObexManagerBluez4::createSessionFinished);

    return call;
}

PendingCall *ObexManagerBluez4::removeSession(const QDBusObjectPath &session)
{
    // Manually add/remove sessions as DBusObjectManager is not available in BlueZ 4 to detect when
    // they are created/removed.
    PendingCall *call = new PendingCall(m_bluez4ObexClient->RemoveSession(session),
                                        PendingCall::ReturnVoid, this);
    call->setProperty("session", session.path());
    connect(call, &PendingCall::finished, this, &ObexManagerBluez4::removeSessionFinished);

    return call;
}

void ObexManagerBluez4::createSessionFinished(PendingCall *call)
{
    if (call->error()) {
        return;
    }

    QString destination = call->property("destination").toString();
    QVariantMap args = call->property("args").toMap();
    if (destination.isEmpty()) {
        qCWarning(BLUEZQT) << "No destination provided for created session";
        return;
    }
    if (args.isEmpty()) {
        qCWarning(BLUEZQT) << "No args provided for created session";
        return;
    }

    m_obexManagerPrivate->addSession(destination, args);
}

void ObexManagerBluez4::removeSessionFinished(PendingCall *call)
{
    if (call->error()) {
        return;
    }

    QString session = call->property("session").toString();
    if (session.isEmpty()) {
        qCWarning(BLUEZQT) << "No session path provided for removed session";
        return;
    }

    m_obexManagerPrivate->removeSession(session);
}

ObexTransferPtr ObexManagerBluez4::newObjectPushTransfer(const QDBusObjectPath &transferPath, const QVariantMap &transferProperties, const QString &destinationAddress)
{
    ObexTransferPtr transfer = ObexTransferPtr(new ObexTransfer(transferPath.path(), transferProperties));
    transfer->d->q = transfer.toWeakRef();
    m_oppTransfers.insert(transferPath.path(), transfer);

    // Source, Channel and Target properties not added; not supportable in BlueZ 4.
    QVariantMap sessionProperties;
    sessionProperties.insert(QStringLiteral("Destination"), destinationAddress);
    sessionProperties.insert(QStringLiteral("Root"), QDir::home().absolutePath());
    sessionProperties.insert(ObexSessionBluez4::objectPushTransferPathKey(), transferPath.path());

    QString sessionPath = QStringLiteral("/org/bluez/obex/server/session") + QString::number(m_obexManagerPrivate->m_sessions.count());
    m_obexManagerPrivate->addSession(sessionPath, sessionProperties);

    notifyObexManagers(QStringLiteral("objectPushTransferCreated"),
                       QVariantList() << transferPath.path() << transferProperties
                                      << sessionPath << sessionProperties);
    return transfer;
}

void ObexManagerBluez4::objectPushTransferCreated(const QString &transferPath, const QVariantMap &transferProperties,
                                                   const QString &sessionPath, const QVariantMap &sessionProperties)
{
    if (!m_oppTransfers.contains(transferPath)) {
        ObexTransferPtr transfer = ObexTransferPtr(new ObexTransfer(transferPath, transferProperties));
        transfer->d->q = transfer.toWeakRef();
        m_oppTransfers.insert(transferPath, transfer);
    }

    if (!m_obexManagerPrivate->m_sessions.contains(sessionPath)) {
        m_obexManagerPrivate->addSession(sessionPath, sessionProperties);
    }
}

void ObexManagerBluez4::objectPushTransferFinished(const QString &transferPath, const QString &sessionPath, bool success)
{
    Q_UNUSED(sessionPath);

    transferCompleted(QDBusObjectPath(transferPath), success);
}

void ObexManagerBluez4::setTransferAborted(const QString &transferPath)
{
    // For BlueZ 4 OPP sessions are manually manged, and there is no way to detect when a transfer
    // is rejected by Authorize() and thus the session and transfer should be removed, so the
    // manager depends on this method being called in those cases.

    Q_FOREACH (ObexSessionPtr session, m_obexManagerPrivate->m_sessions) {
        if (session->d->m_bluez4.m_oppTransferPath == transferPath) {
            notifyObexManagers(QStringLiteral("objectPushTransferFinished"),
                               QVariantList() << transferPath << session->objectPath().path() << false);
            break;
        }
    }
}

QVariantMapList ObexManagerBluez4::sessionProperties() const
{
    QVariantMapList result;

    Q_FOREACH (ObexSessionPtr session, m_obexManagerPrivate->m_sessions) {
        QVariantMap properties = session->d->m_bluez4.m_properties;
        properties.insert(obexSessionPathKey(), session->objectPath().path());
        result.append(properties);
    }

    return result;
}

QVariantMapList ObexManagerBluez4::transferProperties() const
{
    QVariantMapList result;
    QHash<QString, ObexTransferPtr>::const_iterator it;

    for (it = m_oppTransfers.constBegin(); it != m_oppTransfers.constEnd(); ++it) {
        const QString &path = it.key();
        const ObexTransferPtr &transfer = it.value();

        QVariantMap properties = transfer->d->m_bluez4->m_properties;
        properties.insert(obexTransferPathKey(), path);
        result.append(properties);
    }

    return result;
}

ObexSessionPtr ObexManagerBluez4::sessionForObjectPushTransfer(const QDBusObjectPath &transferPath)
{
    Q_FOREACH (ObexSessionPtr session, m_obexManagerPrivate->m_sessions) {
        if (session->d->m_bluez4.m_oppTransferPath == transferPath.path()) {
            return session;
        }
    }
    return ObexSessionPtr();
}

void ObexManagerBluez4::transferStarted(const QDBusObjectPath &objectPath)
{
    ObexTransferPtr transfer = m_oppTransfers.value(objectPath.path());
    if (!transfer.isNull()) {
        transfer->d->m_bluez4->setTransferProperty(QStringLiteral("Status"), QStringLiteral("active"));
    }
}

void ObexManagerBluez4::transferCompleted(const QDBusObjectPath &objectPath, bool success)
{
    ObexTransferPtr transfer = m_oppTransfers.value(objectPath.path());

    if (!transfer.isNull()) {
        transfer->d->m_bluez4->setTransferProperty(QStringLiteral("Status"),
                                                 success ? QStringLiteral("complete") : QStringLiteral("error"));
        m_oppTransfers.remove(objectPath.path());
    }

    ObexSessionPtr session = sessionForObjectPushTransfer(objectPath);
    if (session) {
        removeSession(session->objectPath());
    }
}

void ObexManagerBluez4::notifyObexManagers(const QString &signalName, const QVariantList &args)
{
    QDBusMessage message = QDBusMessage::createSignal(ObexManagerNotifier::objectPath(), ObexManagerNotifier::interface(), signalName);
    message.setArguments(args);

    if (!ObexManagerNotifier::connection().send(message)) {
        qCWarning(BLUEZQT) << "Failed to notify OBEX managers of signal" << signalName
                           << "with args" << args;
    }
}

QString ObexManagerBluez4::obexSessionPathKey()
{
    return QStringLiteral("org.kde.bluezqt.ObexSession.Path");
}

QString ObexManagerBluez4::obexTransferPathKey()
{
    return QStringLiteral("org.kde.bluezqt.ObexTransfer.Path");
}

} // namespace BluezQt
