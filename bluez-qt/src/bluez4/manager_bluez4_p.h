/****************************************************************************
**
** Copyright (C) 2016 Jolla Ltd.
** Contact: Bea Lam <bea.lam@jolla.com>
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
****************************************************************************/

#ifndef BLUEZQT_MANAGER_BLUEZ4_P_H
#define BLUEZQT_MANAGER_BLUEZ4_P_H

#include <QSet>

#include "types.h"
#include "bluez4manager.h"

namespace BluezQt
{

typedef org::bluez::Manager Bluez4Manager;

class ManagerPrivate;
class AdapterPrivate;
class ProxyAgent;
class Agent;

class ManagerBluez4 : public QObject
{
    Q_OBJECT

public:
    explicit ManagerBluez4(ManagerPrivate *parent);

    void load();

    void addAdapter(AdapterPrivate *adapterPrivate);
    void adapterRemoved(const AdapterPrivate *adapterPrivate);

    QDBusPendingReply<void> requestDefaultAgent(AdapterPtr adapter, Agent *agent);
    QDBusPendingReply<void> unregisterDefaultAgent(AdapterPtr adapter);
    AdapterPtr findAdapterForDefaultAgent(Agent *agent);

    QDBusPendingReply<QDBusObjectPath> createPairedDevice(AdapterPtr adapter, const QString &address);

    ManagerPrivate *m_managerPrivate;
    Bluez4Manager *m_bluez4Manager;

signals:
    void loaded(bool success, const QString &errorMessage);

private:
    void emitLoaded(bool success, const QString &errorMessage = QString());
    void managerDefaultAdapterFinished(QDBusPendingCallWatcher *watcher);
    void managerDefaultAdapterChanged(const QDBusObjectPath &objectPath);
    void managerAdapterAdded(const QDBusObjectPath &objectPath);
    void managerAdapterRemoved(const QDBusObjectPath &objectPath);

    void adapterGetPropertiesFinished(QDBusPendingCallWatcher *watcher);
    void adapterPropertyChanged(const QString &property, const QDBusVariant &value);

    void updateDeviceList(const QString &adapterPath, const QVariant &deviceList);
    QDBusPendingCallWatcher *addDeviceByPath(const QDBusObjectPath &objectPath);
    void deviceGetPropertiesFinished(QDBusPendingCallWatcher *watcher);
    void deviceFound(const QString &address, const QVariantMap &values);
    void deviceRemoved(const QDBusObjectPath &objectPath);

    void requestDefaultAgentFinished(QDBusPendingCallWatcher *watcher);
    void unregisterAgentFinished(QDBusPendingCallWatcher *watcher);
    void agentCreated(Agent *agent);

    bool m_loaded;    
    QSet<QDBusPendingCallWatcher *> m_pendingInitializationWatchers;
    QHash<QString, ProxyAgent *> m_defaultAgents;
};

} // namespace BluezQt

#endif // BLUEZQT_MANAGER_BLUEZ4_P_H
