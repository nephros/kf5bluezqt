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

#include "adapter_bluez4_p.h"
#include "macros_bluez4_p.h"
#include "proxyagent_p.h"

#include "adapter.h"
#include "adapter_p.h"
#include "utils.h"
#include "macros.h"

namespace BluezQt
{

AdapterBluez4::AdapterBluez4(AdapterPrivate *adapterPrivate, const QString &path, const QVariantMap &properties)
    : QObject(adapterPrivate)
    , m_adapterPrivate(adapterPrivate)
    , m_bluez4Adapter(0)
    , m_pairingAgent(0)
{
    m_bluez4Adapter = new Bluez4Adapter(Strings::orgBluez(), path, DBusConnection::orgBluez(), this);
    connect(m_bluez4Adapter, &Bluez4Adapter::PropertyChanged,
            this, &AdapterBluez4::adapterPropertyChanged);

    // In BlueZ 4 there is no "Alias" property, so set Adapter::alias() to return the "Name" value.
    adapterPrivate->init(properties);
    adapterPrivate->m_alias = properties.value(QStringLiteral("Name")).toString();
}

QDBusPendingReply<> AdapterBluez4::setDBusProperty(const QString &name, const QVariant &value)
{
    if (name == QStringLiteral("Alias")) {
        return m_bluez4Adapter->SetProperty(QStringLiteral("Name"), QDBusVariant(value));
    } else {
        return m_bluez4Adapter->SetProperty(name, QDBusVariant(value));
    }
}

QDBusPendingReply<QDBusObjectPath> AdapterBluez4::createPairedDevice(const QString &address)
{
    if (!m_pairingAgent) {
        return QDBusMessage::createError(QStringLiteral("org.bluez.Error.Failed"),
                                         QStringLiteral("No pairing agent set!"));
    }

    // Create a single-use agent that is a proxy of the default agent.
    ProxyAgent *proxyAgent = createProxyForAgent(m_pairingAgent, "/bluez4_pairing_request_agent_proxy");
    connect(proxyAgent, &ProxyAgent::agentReleased, proxyAgent, &ProxyAgent::deleteLater);

    return m_bluez4Adapter->CreatePairedDevice(address, proxyAgent->objectPath(),
            ProxyAgent::capabilityToString(proxyAgent->capability()));
}

QDBusPendingReply<void> AdapterBluez4::cancelDeviceCreation(const QString &address)
{
    return m_bluez4Adapter->CancelDeviceCreation(address);
}

ProxyAgent *AdapterBluez4::createProxyForAgent(Agent *agent, const QString &proxyAgentPath)
{
    Q_ASSERT(agent);

    ProxyAgent *proxyAgent = new ProxyAgent(agent, proxyAgentPath, m_adapterPrivate->q.data());
    emit agentCreated(proxyAgent);

    return proxyAgent;
}

void AdapterBluez4::setPairingAgent(Agent *agent)
{
    m_pairingAgent = agent;
}

void AdapterBluez4::adapterPropertyChanged(const QString &property, const QDBusVariant &value)
{
    INVOKE_PROPERTIES_CHANGED(Strings::orgBluezAdapter1(), m_adapterPrivate, property, value.variant());

    // In BlueZ 4 there is no "Alias" property, so set Adapter::alias() to return the "Name" value.
    if (property == QStringLiteral("Name")) {
        INVOKE_PROPERTIES_CHANGED(Strings::orgBluezAdapter1(), m_adapterPrivate, QStringLiteral("Alias"), value.variant());
    }
}

} // namespace BluezQt
