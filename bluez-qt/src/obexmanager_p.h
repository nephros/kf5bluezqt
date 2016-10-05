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

#ifndef BLUEZQT_OBEXMANAGER_P_H
#define BLUEZQT_OBEXMANAGER_P_H

#include <QObject>
#include <QTimer>

#include "types.h"
#if KF5BLUEZQT_BLUEZ_VERSION >= 5
#include "dbusobjectmanager.h"
#include "obexclient1.h"
#include "obexagentmanager1.h"
#else
#include "bluezqt_dbustypes.h"
#include "bluezobexclient.h"
#include "bluezobexmanager.h"
#include <QDBusAbstractAdaptor>
#endif

namespace BluezQt
{

#if KF5BLUEZQT_BLUEZ_VERSION >= 5
typedef org::bluez::obex::Client1 ObexClient;
typedef org::bluez::obex::AgentManager1 ObexAgentManager;
typedef org::freedesktop::DBus::ObjectManager DBusObjectManager;
#else
typedef org::bluez::obex::Client ObexClient;
typedef org::bluez::obex::Manager ObexAgentManager;
#endif

#if KF5BLUEZQT_BLUEZ_VERSION < 5
class ObexManagerPrivate;

class ObexManagerNotifier : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.bluezqt.obex.ObexManager")
public:
    ObexManagerNotifier(ObexManagerPrivate *parent);

    inline static QDBusConnection connection() { return QDBusConnection::sessionBus(); }
    inline static QString service() { return QStringLiteral("org.kde.bluezqt.obex"); }
    inline static QString objectPath() { return QStringLiteral("/org/kde/bluezqt/obex/ObexManager"); }
    inline static QString interface() { return QStringLiteral("org.kde.bluezqt.obex.ObexManager"); }

public Q_SLOTS:
    QMap<QString, QVariantMap> getSessions();
    QMap<QString, QVariantMap> getTransfers();
    void setTransferAborted(const QString &transferPath);

private:
    friend class ObexManagerPrivate;
    ObexManagerPrivate *m_manager;
};
#endif

class ObexManager;

class ObexManagerPrivate : public QObject
{
    Q_OBJECT

public:
    explicit ObexManagerPrivate(ObexManager *q);

    void init();
    void nameHasOwnerFinished(QDBusPendingCallWatcher *watcher);
    void load();
    void getManagedObjectsFinished(QDBusPendingCallWatcher *watcher);
    void clear();

    void serviceRegistered();
    void serviceUnregistered();
#if KF5BLUEZQT_BLUEZ_VERSION >= 5
    void interfacesAdded(const QDBusObjectPath &objectPath, const QVariantMapMap &interfaces);
    void interfacesRemoved(const QDBusObjectPath &objectPath, const QStringList &interfaces);
#endif

    void addSession(const QString &sessionPath, const QVariantMap &properties);
    void removeSession(const QString &sessionPath);

#if KF5BLUEZQT_BLUEZ_VERSION < 5
    void getSessionsFinished(QDBusPendingCallWatcher *watcher);
    void getTransfersFinished(QDBusPendingCallWatcher *watcher);
    void completeInit();

    ObexTransferPtr newObjectPushTransfer(const QDBusObjectPath &transferPath, const QVariantMap &transferProperties, const QString &destinationAddress);
    void transferStarted(const QDBusObjectPath &objectPath);
    void transferCompleted(const QDBusObjectPath &objectPath, bool success);
    void createSessionFinished(PendingCall *call);
    void removeSessionFinished(PendingCall *call);

    void setTransferAborted(const QString &transferPath);
    void notifyObexManagers(const QString &signalName, const QVariantList &args);
    QVariantMapMap sessionProperties() const;
    QVariantMapMap transferProperties() const;
    QString sessionForObjectPushTransfer(const QDBusObjectPath &transferPath);
#endif

    ObexManager *q;
    ObexClient *m_obexClient;
    ObexAgentManager *m_obexAgentManager;
#if KF5BLUEZQT_BLUEZ_VERSION >= 5
    DBusObjectManager *m_dbusObjectManager;
#else
    QHash<QString, ObexTransferPtr> m_oppTransfers;
    ObexManagerNotifier *m_managerNotifier;
    bool m_initializedSessions;
    bool m_initializedTransfers;
#endif

    QTimer m_timer;
    QHash<QString, ObexSessionPtr> m_sessions;

    bool m_initialized;
    bool m_obexRunning;
    bool m_loaded;

Q_SIGNALS:
    void initError(const QString &errorText);
    void initFinished();

private Q_SLOTS:
    void dummy();
#if KF5BLUEZQT_BLUEZ_VERSION < 5
    void objectPushTransferCreated(const QString &transferPath, const QVariantMap &transferProperties,
                                   const QString &sessionPath, const QVariantMap &sessionProperties);
    void objectPushTransferFinished(const QString &transferPath, const QString &sessionPath, bool success);
#endif
};

} // namespace BluezQt

#endif // BLUEZQT_OBEXMANAGER_P_H
