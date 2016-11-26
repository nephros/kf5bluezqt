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
class QDBusPendingCallWatcher;
#endif

namespace BluezQt
{

#if KF5BLUEZQT_BLUEZ_VERSION >= 5
typedef org::bluez::obex::Client1 ObexClient;
typedef org::bluez::obex::AgentManager1 ObexAgentManager;
typedef org::freedesktop::DBus::ObjectManager DBusObjectManager;
#endif

class ObexManager;

#if KF5BLUEZQT_BLUEZ_VERSION < 5
class ObexManagerBluez4;
#endif

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
    void interfacesAdded(const QDBusObjectPath &objectPath, const QVariantMapMap &interfaces);
    void interfacesRemoved(const QDBusObjectPath &objectPath, const QStringList &interfaces);

    void addSession(const QString &sessionPath, const QVariantMap &properties);
    void removeSession(const QString &sessionPath);

    ObexManager *q;
#if KF5BLUEZQT_BLUEZ_VERSION >= 5
    ObexClient *m_obexClient;
    ObexAgentManager *m_obexAgentManager;
    DBusObjectManager *m_dbusObjectManager;
#endif

    QTimer m_timer;
    QHash<QString, ObexSessionPtr> m_sessions;

    bool m_initialized;
    bool m_obexRunning;
    bool m_loaded;

#if KF5BLUEZQT_BLUEZ_VERSION < 5
    void bluez4ObexManagerLoaded();
    ObexManagerBluez4 *m_bluez4;
#endif

Q_SIGNALS:
    void initError(const QString &errorText);
    void initFinished();

private Q_SLOTS:
    void dummy();
    void interfacesAddedSlot(const QDBusObjectPath &objectPath);
    void getInterfacesManagedObjectsFinished(QDBusPendingCallWatcher *watcher);
};

} // namespace BluezQt

#endif // BLUEZQT_OBEXMANAGER_P_H
