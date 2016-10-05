/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * Copyright (C) 2014-2015 David Rosca <nowrep@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 */

#include "obexmanager_p.h"
#include "obexmanager.h"
#include "obexsession.h"
#include "obexsession_p.h"
#include "debug.h"
#include "utils.h"

#if KF5BLUEZQT_BLUEZ_VERSION >= 5
#include "dbusobjectmanager.h"
#else
#include "obextransfer_p.h"
#include "pendingcall.h"
#include "bluezqt_dbustypes.h"
#include <QDBusMessage>
#endif


#include <QDBusServiceWatcher>

namespace BluezQt
{

#if KF5BLUEZQT_BLUEZ_VERSION >= 5
typedef org::freedesktop::DBus::ObjectManager DBusObjectManager;
#else
ObexManagerNotifier::ObexManagerNotifier(ObexManagerPrivate *parent)
    : QDBusAbstractAdaptor(parent)
    , m_manager(parent)
{
    qDBusRegisterMetaType<QVariantMapMap>();
}

QMap<QString, QVariantMap> ObexManagerNotifier::getSessions()
{
    return m_manager->sessionProperties();
}

QMap<QString, QVariantMap> ObexManagerNotifier::getTransfers()
{
    return m_manager->transferProperties();
}

void ObexManagerNotifier::setTransferAborted(const QString &transferPath)
{
    m_manager->setTransferAborted(transferPath);
}

#endif

ObexManagerPrivate::ObexManagerPrivate(ObexManager *q)
    : QObject(q)
    , q(q)
    , m_obexClient(0)
    , m_obexAgentManager(0)
#if KF5BLUEZQT_BLUEZ_VERSION >= 5
    , m_dbusObjectManager(0)
#else
    , m_managerNotifier(0)
    , m_initializedSessions(false)
    , m_initializedTransfers(false)
#endif
    , m_initialized(false)
    , m_obexRunning(false)
    , m_loaded(false)
{
    qDBusRegisterMetaType<DBusManagerStruct>();
    qDBusRegisterMetaType<QVariantMapMap>();

    m_timer.setSingleShot(true);
    connect(&m_timer, &QTimer::timeout, this, &ObexManagerPrivate::load);
}

void ObexManagerPrivate::init()
{
    // Keep an eye on org.bluez.obex service
    QDBusServiceWatcher *serviceWatcher = new QDBusServiceWatcher(Strings::orgBluezObex(), DBusConnection::orgBluezObex(),
            QDBusServiceWatcher::WatchForRegistration | QDBusServiceWatcher::WatchForUnregistration, this);

    connect(serviceWatcher, &QDBusServiceWatcher::serviceRegistered, this, &ObexManagerPrivate::serviceRegistered);
    connect(serviceWatcher, &QDBusServiceWatcher::serviceUnregistered, this, &ObexManagerPrivate::serviceUnregistered);

    // Update the current state of org.bluez.obex service
    if (!DBusConnection::orgBluezObex().isConnected()) {
        Q_EMIT initError(QStringLiteral("DBus session bus is not connected!"));
        return;
    }

    QDBusMessage call = QDBusMessage::createMethodCall(Strings::orgFreedesktopDBus(),
                        QStringLiteral("/"),
                        Strings::orgFreedesktopDBus(),
                        QStringLiteral("NameHasOwner"));

    call << Strings::orgBluezObex();

    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(DBusConnection::orgBluezObex().asyncCall(call));
    connect(watcher, &QDBusPendingCallWatcher::finished, this, &ObexManagerPrivate::nameHasOwnerFinished);
}

void ObexManagerPrivate::nameHasOwnerFinished(QDBusPendingCallWatcher *watcher)
{
    const QDBusPendingReply<bool> &reply = *watcher;
    watcher->deleteLater();

    if (reply.isError()) {
        Q_EMIT initError(reply.error().message());
        return;
    }

    m_obexRunning = reply.value();

    if (m_obexRunning) {
        load();
    } else {
        m_initialized = true;
        Q_EMIT initFinished();
    }
}

void ObexManagerPrivate::load()
{
    if (!m_obexRunning || m_loaded) {
        return;
    }

    // Force QDBus to cache owner of org.bluez.obex - this will be the only blocking call on session connection
    DBusConnection::orgBluezObex().connect(Strings::orgBluezObex(),
                                           QStringLiteral("/"),
                                           Strings::orgFreedesktopDBus(),
                                           QStringLiteral("Dummy"),
                                           this,
                                           SLOT(dummy()));

#if KF5BLUEZQT_BLUEZ_VERSION >= 5
    m_dbusObjectManager = new DBusObjectManager(Strings::orgBluezObex(), QStringLiteral("/"),
            DBusConnection::orgBluezObex(), this);

    connect(m_dbusObjectManager, &DBusObjectManager::InterfacesAdded,
            this, &ObexManagerPrivate::interfacesAdded);
    connect(m_dbusObjectManager, &DBusObjectManager::InterfacesRemoved,
            this, &ObexManagerPrivate::interfacesRemoved);

    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(m_dbusObjectManager->GetManagedObjects(), this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, &ObexManagerPrivate::getManagedObjectsFinished);
#else
    m_obexClient = new ObexClient(QStringLiteral("org.bluez.obex.client"), "/", DBusConnection::orgBluezObex(), this);
    m_obexAgentManager = new ObexAgentManager(Strings::orgBluezObex(), "/", DBusConnection::orgBluezObex(), this);

    connect(m_obexAgentManager, &ObexAgentManager::TransferStarted,
            this, &ObexManagerPrivate::transferStarted);
    connect(m_obexAgentManager, &ObexAgentManager::TransferCompleted,
            this, &ObexManagerPrivate::transferCompleted);

    // Fetch known sessions from the "system" ObexManager
    QDBusMessage getSessionsCall = QDBusMessage::createMethodCall(ObexManagerNotifier::service(),
                                                       ObexManagerNotifier::objectPath(),
                                                       ObexManagerNotifier::interface(),
                                                       QStringLiteral("getSessions"));

    QDBusPendingCallWatcher *getSessionsWatcher = new QDBusPendingCallWatcher(ObexManagerNotifier::connection().asyncCall(getSessionsCall));
    connect(getSessionsWatcher, &QDBusPendingCallWatcher::finished, this, &ObexManagerPrivate::getSessionsFinished);

    // Fetch known transfers from the "system" ObexManager, including those that are pending
    // Authorize() and therefore cannot normally be fetched as BlueZ 4 would not have emitted
    // transferStarted() for them yet.
    QDBusMessage getTransfersCall = QDBusMessage::createMethodCall(ObexManagerNotifier::service(),
                                                       ObexManagerNotifier::objectPath(),
                                                       ObexManagerNotifier::interface(),
                                                       QStringLiteral("getTransfers"));

    QDBusPendingCallWatcher *getTransfersWatcher = new QDBusPendingCallWatcher(ObexManagerNotifier::connection().asyncCall(getTransfersCall));
    connect(getTransfersWatcher, &QDBusPendingCallWatcher::finished, this, &ObexManagerPrivate::getTransfersFinished);
#endif
}

#if KF5BLUEZQT_BLUEZ_VERSION >= 5
void ObexManagerPrivate::getManagedObjectsFinished(QDBusPendingCallWatcher *watcher)
{
    const QDBusPendingReply<DBusManagerStruct> &reply = *watcher;
    watcher->deleteLater();

    if (reply.isError()) {
        Q_EMIT initError(reply.error().message());
        return;
    }

    DBusManagerStruct::const_iterator it;
    const DBusManagerStruct &managedObjects = reply.value();

    for (it = managedObjects.constBegin(); it != managedObjects.constEnd(); ++it) {
        const QString &path = it.key().path();
        const QVariantMapMap &interfaces = it.value();

        if (interfaces.contains(Strings::orgBluezObexSession1())) {
            addSession(path, interfaces.value(Strings::orgBluezObexSession1()));
        } else if (interfaces.contains(Strings::orgBluezObexClient1()) && interfaces.contains(Strings::orgBluezObexAgentManager1())) {
            m_obexClient = new ObexClient(Strings::orgBluezObex(), path, DBusConnection::orgBluezObex(), this);
            m_obexAgentManager = new ObexAgentManager(Strings::orgBluezObex(), path, DBusConnection::orgBluezObex(), this);
        }
    }

    if (!m_obexClient) {
        Q_EMIT initError(QStringLiteral("Cannot find org.bluez.obex.Client1 object!"));
        return;
    }

    if (!m_obexAgentManager) {
        Q_EMIT initError(QStringLiteral("Cannot find org.bluez.obex.AgentManager1 object!"));
        return;
    }

    m_loaded = true;
    m_initialized = true;

    Q_EMIT q->operationalChanged(true);
    Q_EMIT initFinished();
}
#endif

void ObexManagerPrivate::clear()
{
    m_loaded = false;

    // Delete all sessions
    while (!m_sessions.isEmpty()) {
        ObexSessionPtr session = m_sessions.begin().value();
        m_sessions.remove(m_sessions.begin().key());
        Q_EMIT q->sessionRemoved(session);
    }

    // Delete all other objects
    if (m_obexClient) {
        m_obexClient->deleteLater();
        m_obexClient = Q_NULLPTR;
    }

    if (m_obexAgentManager) {
        m_obexAgentManager->deleteLater();
        m_obexAgentManager = Q_NULLPTR;
    }

#if KF5BLUEZQT_BLUEZ_VERSION >= 5
    if (m_dbusObjectManager) {
        m_dbusObjectManager->deleteLater();
        m_dbusObjectManager = Q_NULLPTR;
    }
#endif
}

void ObexManagerPrivate::serviceRegistered()
{
    qCDebug(BLUEZQT) << "Obex service registered";
    m_obexRunning = true;

    // Client1 and AgentManager1 objects are not ready by the time org.bluez.obex is registered
    // nor will the ObjectManager emits interfacesAdded for adding them...
    // So we delay the call to load() by 0.5s
    m_timer.start(500);
}

void ObexManagerPrivate::serviceUnregistered()
{
    qCDebug(BLUEZQT) << "Obex service unregistered";
    m_obexRunning = false;

    clear();
    Q_EMIT q->operationalChanged(false);
}

#if KF5BLUEZQT_BLUEZ_VERSION >= 5
void ObexManagerPrivate::interfacesAdded(const QDBusObjectPath &objectPath, const QVariantMapMap &interfaces)
{
    const QString &path = objectPath.path();
    QVariantMapMap::const_iterator it;

    for (it = interfaces.constBegin(); it != interfaces.constEnd(); ++it) {
        if (it.key() == Strings::orgBluezObexSession1()) {
            addSession(path, it.value());
        }
    }
}
#endif

#if KF5BLUEZQT_BLUEZ_VERSION >= 5
void ObexManagerPrivate::interfacesRemoved(const QDBusObjectPath &objectPath, const QStringList &interfaces)
{
    const QString &path = objectPath.path();

    Q_FOREACH (const QString &interface, interfaces) {
        if (interface == Strings::orgBluezObexSession1()) {
            removeSession(path);
        }
    }
}
#endif

void ObexManagerPrivate::addSession(const QString &sessionPath, const QVariantMap &properties)
{
    ObexSessionPtr session = ObexSessionPtr(new ObexSession(sessionPath, properties));
    session->d->q = session.toWeakRef();
    m_sessions.insert(sessionPath, session);

    Q_EMIT q->sessionAdded(session);
}

void ObexManagerPrivate::removeSession(const QString &sessionPath)
{
    ObexSessionPtr session = m_sessions.take(sessionPath);
    if (!session) {
        return;
    }

    Q_EMIT q->sessionRemoved(session);
}

void ObexManagerPrivate::dummy()
{
}

#if KF5BLUEZQT_BLUEZ_VERSION < 5
void ObexManagerPrivate::getSessionsFinished(QDBusPendingCallWatcher *watcher)
{
    const QDBusPendingReply<QVariantMapMap> &reply = *watcher;
    watcher->deleteLater();

    m_initializedSessions = true;

    if (!reply.isError()) {
        QVariantMapMap::const_iterator it;
        const QVariantMapMap &sessions = reply.value();

        for (it = sessions.constBegin(); it != sessions.constEnd(); ++it) {
            const QString &path = it.key();
            const QVariantMap &properties = it.value();

            addSession(path, properties);
        }
    } else if (reply.error().name() != QStringLiteral("org.freedesktop.DBus.Error.ServiceUnknown")) {
        // Error can be ignored as it occurs if registerAgent() has not yet been called on any
        // managers, which means this is probably the manager that will be used to register agents.
        qCWarning(BLUEZQT) << "Failed to find existing sessions:" << reply.error().message();
    }

    if (m_initializedSessions && m_initializedTransfers) {
        completeInit();
    }
}

void ObexManagerPrivate::getTransfersFinished(QDBusPendingCallWatcher *watcher)
{
    const QDBusPendingReply<QVariantMapMap> &reply = *watcher;
    watcher->deleteLater();

    m_initializedTransfers = true;

    if (!reply.isError()) {
        QVariantMapMap::const_iterator it;
        const QVariantMapMap &transfers = reply.value();

        for (it = transfers.constBegin(); it != transfers.constEnd(); ++it) {
            const QString &path = it.key();
            const QVariantMap &properties = it.value();

            if (!m_oppTransfers.contains(path)) {
                ObexTransferPtr transfer = ObexTransferPtr(new ObexTransfer(path, properties));
                transfer->d->q = transfer.toWeakRef();
                m_oppTransfers.insert(path, transfer);
            }
        }
    } else if (reply.error().name() != QStringLiteral("org.freedesktop.DBus.Error.ServiceUnknown")) {
        // Error can be ignored as it occurs if registerAgent() has not yet been called on any
        // managers, which means this is probably the manager that will be used to register agents.
        qCWarning(BLUEZQT) << "Failed to find existing transfers:" << reply.error().message();
    }

    if (m_initializedSessions && m_initializedTransfers) {
        completeInit();
    }
}

void ObexManagerPrivate::completeInit()
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

    // Complete initialization as per getManagedObjectsFinished()
    m_loaded = true;
    m_initialized = true;

    Q_EMIT q->operationalChanged(true);
    Q_EMIT initFinished();
}

ObexTransferPtr ObexManagerPrivate::newObjectPushTransfer(const QDBusObjectPath &transferPath, const QVariantMap &transferProperties, const QString &destinationAddress)
{
    ObexTransferPtr transfer = ObexTransferPtr(new ObexTransfer(transferPath.path(), transferProperties));
    transfer->d->q = transfer.toWeakRef();
    m_oppTransfers.insert(transferPath.path(), transfer);

    // Source, Channel and Target properties not added; not supportable in BlueZ 4.
    QVariantMap sessionProperties;
    sessionProperties.insert(QStringLiteral("Destination"), destinationAddress);
    sessionProperties.insert(QStringLiteral("Root"), QDir::home().absolutePath());
    sessionProperties.insert(QStringLiteral("org.kde.bluezqt.ObjectPushTransfer"), transferPath.path());

    QString sessionPath = QStringLiteral("/org/bluez/obex/server/session") + QString::number(m_sessions.count());
    addSession(sessionPath, sessionProperties);

    notifyObexManagers(QStringLiteral("objectPushTransferCreated"),
                       QVariantList() << transferPath.path() << transferProperties
                                      << sessionPath << sessionProperties);
    return transfer;
}

void ObexManagerPrivate::objectPushTransferCreated(const QString &transferPath, const QVariantMap &transferProperties,
                                                   const QString &sessionPath, const QVariantMap &sessionProperties)
{
    if (!m_oppTransfers.contains(transferPath)) {
        ObexTransferPtr transfer = ObexTransferPtr(new ObexTransfer(transferPath, transferProperties));
        transfer->d->q = transfer.toWeakRef();
        m_oppTransfers.insert(transferPath, transfer);
    }

    if (!m_sessions.contains(sessionPath)) {
        addSession(sessionPath, sessionProperties);
    }
}

void ObexManagerPrivate::objectPushTransferFinished(const QString &transferPath, const QString &sessionPath, bool success)
{
    Q_UNUSED(sessionPath);

    transferCompleted(QDBusObjectPath(transferPath), success);
}

void ObexManagerPrivate::transferStarted(const QDBusObjectPath &objectPath)
{
    ObexTransferPtr transfer = m_oppTransfers.value(objectPath.path());
    if (!transfer.isNull()) {
        transfer->setTransferProperty(QStringLiteral("Status"), QStringLiteral("active"));
    }
}

void ObexManagerPrivate::transferCompleted(const QDBusObjectPath &objectPath, bool success)
{
    ObexTransferPtr transfer = m_oppTransfers.value(objectPath.path());

    if (!transfer.isNull()) {
        transfer->setTransferProperty(QStringLiteral("Status"),
                                      success ? QStringLiteral("complete") : QStringLiteral("error"));
        m_oppTransfers.remove(objectPath.path());
    }

    QString sessionPath = sessionForObjectPushTransfer(objectPath);
    if (!sessionPath.isEmpty()) {
        removeSession(sessionPath);
    }
}

void ObexManagerPrivate::createSessionFinished(PendingCall *call)
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

    addSession(destination, args);
}

void ObexManagerPrivate::removeSessionFinished(PendingCall *call)
{
    if (call->error()) {
        return;
    }

    QString session = call->property("session").toString();
    if (session.isEmpty()) {
        qCWarning(BLUEZQT) << "No session path provided for removed session";
        return;
    }

    removeSession(session);
}

void ObexManagerPrivate::setTransferAborted(const QString &transferPath)
{
    // For BlueZ 4 OPP sessions are manually manged, and there is no way to detect when a transfer
    // is rejected by Authorize() and thus the session and transfer should be removed, so the
    // manager depends on this method being called in those cases.

    Q_FOREACH (ObexSessionPtr session, m_sessions) {
        if (session->d->m_oppTransferPath == transferPath) {
            notifyObexManagers(QStringLiteral("objectPushTransferFinished"),
                               QVariantList() << transferPath << session->d->m_sessionPath << false);
            break;
        }
    }
}

void ObexManagerPrivate::notifyObexManagers(const QString &signalName, const QVariantList &args)
{
    QDBusMessage message = QDBusMessage::createSignal("/", ObexManagerNotifier::interface(), signalName);
    message.setArguments(args);

    if (!ObexManagerNotifier::connection().send(message)) {
        qCWarning(BLUEZQT) << "Failed to notify OBEX managers of signal" << signalName
                           << "with args" << args;
    }
}

QVariantMapMap ObexManagerPrivate::sessionProperties() const
{
    QVariantMapMap properties;

    Q_FOREACH (ObexSessionPtr session, m_sessions) {
        properties.insert(session->d->m_sessionPath, session->d->m_properties);
    }

    return properties;
}

QVariantMapMap ObexManagerPrivate::transferProperties() const
{
    QVariantMapMap properties;
    QHash<QString, ObexTransferPtr>::const_iterator it;

    for (it = m_oppTransfers.constBegin(); it != m_oppTransfers.constEnd(); ++it) {
        const QString &path = it.key();
        const ObexTransferPtr &transfer = it.value();

        properties.insert(path, transfer->d->m_properties);
    }

    return properties;
}

QString ObexManagerPrivate::sessionForObjectPushTransfer(const QDBusObjectPath &transferPath)
{
    Q_FOREACH (ObexSessionPtr session, m_sessions) {
        if (session->d->m_oppTransferPath == transferPath.path()) {
            return session->d->m_sessionPath;
        }
    }
    return QString();
}
#endif

} // namespace BluezQt
