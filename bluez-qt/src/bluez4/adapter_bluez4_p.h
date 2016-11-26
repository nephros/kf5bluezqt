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

#ifndef BLUEZQT_ADAPTER_BLUEZ4_P_H
#define BLUEZQT_ADAPTER_BLUEZ4_P_H

#include "types.h"
#include "bluez4adapter.h"

namespace BluezQt
{

typedef org::bluez::Adapter Bluez4Adapter;

class ProxyAgent;
class AdapterPrivate;

class AdapterBluez4 : public QObject
{
    Q_OBJECT

public:
    explicit AdapterBluez4(AdapterPrivate *adapterPrivate, const QString &path, const QVariantMap &properties);

    QDBusPendingReply<> setDBusProperty(const QString &name, const QVariant &value);

    QDBusPendingReply<QDBusObjectPath> createPairedDevice(const QString &address);
    QDBusPendingReply<void> cancelDeviceCreation(const QString &address);

    ProxyAgent *createProxyForAgent(Agent *agent, const QString &proxyAgentPath);
    void setPairingAgent(Agent *agent);

    AdapterPrivate *m_adapterPrivate;
    Bluez4Adapter *m_bluez4Adapter;

Q_SIGNALS:
    void agentCreated(Agent *agent);

private:
    void adapterPropertyChanged(const QString &property, const QDBusVariant &value);

    Agent *m_pairingAgent;
};

} // namespace BluezQt

#endif // BLUEZQT_ADAPTER_BLUEZ4_P_H
