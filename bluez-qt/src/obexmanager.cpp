/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * Copyright (C) 2014 David Rosca <nowrep@gmail.com>
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

#include "obexmanager.h"
#include "obexmanager_p.h"
#include "initobexmanagerjob.h"
#include "debug.h"
#include "pendingcall.h"
#include "obexagent.h"
#include "obexagentadaptor.h"
#include "obexsession.h"
#include "utils.h"

#include <QDBusServiceWatcher>

namespace BluezQt
{

ObexManager::ObexManager(QObject *parent)
    : QObject(parent)
    , d(new ObexManagerPrivate(this))
{
    Instance::setObexManager(this);
}

ObexManager::~ObexManager()
{
    delete d;
}

InitObexManagerJob *ObexManager::init()
{
    return new InitObexManagerJob(this);
}

bool ObexManager::isInitialized() const
{
    return d->m_initialized;
}

bool ObexManager::isOperational() const
{
    return d->m_initialized && d->m_obexRunning && d->m_loaded;
}

QList<ObexSessionPtr> ObexManager::sessions() const
{
    return d->m_sessions.values();
}

ObexSessionPtr ObexManager::sessionForPath(const QDBusObjectPath &path) const
{
    Q_FOREACH (ObexSessionPtr session, d->m_sessions) {
        if (path.path().startsWith(session->objectPath().path())) {
            return session;
        }
    }
#if KF5BLUEZQT_BLUEZ_VERSION < 5
    // In BlueZ 5, sessionForPath() can be used to find a session for a transfer, as transfer paths
    // start with the associated session path. This is not the case in BlueZ 4, so mimic this
    // feature by searching transfer paths here.
    QString sessionPath = d->sessionForObjectPushTransfer(path);
    if (!sessionPath.isEmpty()) {
        if (!d->m_sessions.contains(sessionPath)) {
            qCWarning(BLUEZQT) << "Cannot find matching session" << sessionPath << "for transfer" << path.path();
            return ObexSessionPtr();
        }
        return d->m_sessions.value(sessionPath);
    }
#endif

    return ObexSessionPtr();
}

PendingCall *ObexManager::startService()
{
    QDBusMessage msg = QDBusMessage::createMethodCall(Strings::orgFreedesktopDBus(),
                                                      QStringLiteral("/org/freedesktop/DBus"),
                                                      Strings::orgFreedesktopDBus(),
                                                      QStringLiteral("StartServiceByName"));
    msg << Strings::orgBluezObex();
    msg << quint32(0);

    return new PendingCall(DBusConnection::orgBluezObex().asyncCall(msg), PendingCall::ReturnUint32);
}

PendingCall *ObexManager::registerAgent(ObexAgent *agent)
{
    Q_ASSERT(agent);

    if (!d->m_obexAgentManager) {
        return new PendingCall(PendingCall::InternalError, QStringLiteral("ObexManager not operational!"));
    }

#if KF5BLUEZQT_BLUEZ_VERSION < 5
    if (!d->m_managerNotifier) {
        d->m_managerNotifier = new ObexManagerNotifier(d);
        if (ObexManagerNotifier::connection().registerObject(ObexManagerNotifier::objectPath(), d)
                && ObexManagerNotifier::connection().registerService(ObexManagerNotifier::service())) {
            qCDebug(BLUEZQT) << "Registered system OBEX manager notifier";
        }
    }
#endif

    new ObexAgentAdaptor(agent, this);

    if (!DBusConnection::orgBluezObex().registerObject(agent->objectPath().path(), agent)) {
        qCDebug(BLUEZQT) << "Cannot register object" << agent->objectPath().path();
    }

    return new PendingCall(d->m_obexAgentManager->RegisterAgent(agent->objectPath()),
                           PendingCall::ReturnVoid, this);
}

PendingCall *ObexManager::unregisterAgent(ObexAgent *agent)
{
    Q_ASSERT(agent);

    if (!d->m_obexAgentManager) {
        return new PendingCall(PendingCall::InternalError, QStringLiteral("ObexManager not operational!"));
    }

    DBusConnection::orgBluezObex().unregisterObject(agent->objectPath().path());

    return new PendingCall(d->m_obexAgentManager->UnregisterAgent(agent->objectPath()),
                           PendingCall::ReturnVoid, this);
}

PendingCall *ObexManager::createSession(const QString &destination, const QVariantMap &args)
{
    if (!d->m_obexClient) {
        return new PendingCall(PendingCall::InternalError, QStringLiteral("ObexManager not operational!"));
    }

#if KF5BLUEZQT_BLUEZ_VERSION >= 5
    return new PendingCall(d->m_obexClient->CreateSession(destination, args),
                           PendingCall::ReturnObjectPath, this);
#else
    // Manually add/remove sessions as DBusObjectManager is not available in BlueZ 4 to detect when
    // they are created/removed.
    PendingCall *call = new PendingCall(d->m_obexClient->CreateSession(destination, args),
                                        PendingCall::ReturnObjectPath, this);
    call->setProperty("destination", destination);
    call->setProperty("args", args);
    connect(call, &PendingCall::finished, d, &ObexManagerPrivate::createSessionFinished);
    return call;
#endif
}

PendingCall *ObexManager::removeSession(const QDBusObjectPath &session)
{
    if (!d->m_obexClient) {
        return new PendingCall(PendingCall::InternalError, QStringLiteral("ObexManager not operational!"));
    }

#if KF5BLUEZQT_BLUEZ_VERSION >= 5
    return new PendingCall(d->m_obexClient->RemoveSession(session),
                           PendingCall::ReturnVoid, this);
#else
    // Manually add/remove sessions as DBusObjectManager is not available in BlueZ 4 to detect when
    // they are created/removed.
    PendingCall *call = new PendingCall(d->m_obexClient->RemoveSession(session),
                                        PendingCall::ReturnVoid, this);
    call->setProperty("session", session.path());
    connect(call, &PendingCall::finished, d, &ObexManagerPrivate::removeSessionFinished);
    return call;
#endif
}

} // namespace BluezQt
