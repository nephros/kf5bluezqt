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

#ifndef BLUEZQT_MANAGER_P_H
#define BLUEZQT_MANAGER_P_H

#include <QObject>
#include <QHash>

#include "types.h"
#include "rfkill.h"
#if KF5BLUEZQT_BLUEZ_VERSION >= 5
#include "dbusobjectmanager.h"
#include "bluezagentmanager1.h"
#include "bluezprofilemanager1.h"
#else
#include "bluezmanager.h"
#include "agent.h"
#include <QSet>
#endif

namespace BluezQt
{

#if KF5BLUEZQT_BLUEZ_VERSION >= 5
typedef org::freedesktop::DBus::ObjectManager DBusObjectManager;
typedef org::bluez::AgentManager1 BluezAgentManager;
typedef org::bluez::ProfileManager1 BluezProfileManager;
#else
typedef org::bluez::Manager BluezManager;
#endif

class Manager;
class Adapter;
class Device;
class AdapterPrivate;

class ManagerPrivate : public QObject
{
    Q_OBJECT

public:
    explicit ManagerPrivate(Manager *parent);

    void init();
    void nameHasOwnerFinished(QDBusPendingCallWatcher *watcher);
    void load();

#if KF5BLUEZQT_BLUEZ_VERSION >= 5
    void getManagedObjectsFinished(QDBusPendingCallWatcher *watcher);
#endif
    void clear();

    AdapterPtr findUsableAdapter() const;

    void serviceRegistered();
    void serviceUnregistered();
#if KF5BLUEZQT_BLUEZ_VERSION >= 5
    void interfacesAdded(const QDBusObjectPath &objectPath, const QVariantMapMap &interfaces);
    void interfacesRemoved(const QDBusObjectPath &objectPath, const QStringList &interfaces);
#endif
    void adapterRemoved(const AdapterPtr &adapter);
    void adapterPoweredChanged(bool powered);
    void rfkillStateChanged(Rfkill::State state);

    void addAdapter(const QString &adapterPath, const QVariantMap &properties);
    void addDevice(const QString &devicePath, const QVariantMap &properties);
    void removeAdapter(const QString &adapterPath);
    void removeDevice(const QString &devicePath);

    bool rfkillBlocked() const;
    void setUsableAdapter(const AdapterPtr &adapter);

#if KF5BLUEZQT_BLUEZ_VERSION < 5
    void managerInitialized();
    void managerDefaultAdapterFinished(QDBusPendingCallWatcher *watcher);
    void managerAdapterAdded(const QDBusObjectPath &objectPath);
    void managerAdapterRemoved(const QDBusObjectPath &objectPath);

    void adapterGetPropertiesFinished(QDBusPendingCallWatcher *watcher);
    void adapterPropertyChanged(const QString &property, const QDBusVariant &value);

    void updateDeviceList(const QString &adapterPath, const QVariant &deviceList);
    QDBusPendingCallWatcher *addDeviceByPath(const QDBusObjectPath &objectPath);
    void deviceGetPropertiesFinished(QDBusPendingCallWatcher *watcher);
    void deviceFound(const QString &address, const QVariantMap &values);
    void deviceRemoved(const QDBusObjectPath &objectPath);

    QDBusPendingReply<void> requestDefaultAgent(AdapterPtr adapter, Agent *agent);
    void requestDefaultAgentFinished(QDBusPendingCallWatcher *watcher);
    QDBusPendingReply<void> unregisterDefaultAgent(AdapterPtr adapter);
    void unregisterAgentFinished(QDBusPendingCallWatcher *watcher);
    AdapterPtr findAdapterForDefaultAgent(Agent *agent);
    void agentCreated(Agent *agent);
#endif

    Manager *q;
    Rfkill *m_rfkill;
#if KF5BLUEZQT_BLUEZ_VERSION >= 5
    DBusObjectManager *m_dbusObjectManager;
    BluezAgentManager *m_bluezAgentManager;
    BluezProfileManager *m_bluezProfileManager;
#else
    BluezManager *m_bluezManager;
    QSet<QDBusPendingCallWatcher *> m_pendingInitializationWatchers;
    QHash<QString, ProxyAgent *> m_defaultAgents;
#endif

    QHash<QString, AdapterPtr> m_adapters;
    QHash<QString, DevicePtr> m_devices;
    AdapterPtr m_usableAdapter;

    bool m_initialized;
    bool m_bluezRunning;
    bool m_loaded;
    bool m_adaptersLoaded;
    bool m_bluetoothBlocked;

Q_SIGNALS:
    void initError(const QString &errorText);
    void initFinished();

private Q_SLOTS:
    void dummy();

};

} // namespace BluezQt

#endif // BLUEZQT_MANAGER_P_H
