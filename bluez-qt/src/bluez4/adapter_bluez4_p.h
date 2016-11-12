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
