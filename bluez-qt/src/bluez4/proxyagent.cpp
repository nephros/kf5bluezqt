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

#include "proxyagent_p.h"

#include <QDBusObjectPath>

namespace BluezQt
{

ProxyAgent::ProxyAgent(Agent *agent, const QString &pathSuffix, QObject *parent)
    : Agent(parent)
    , m_agent(agent)
    , m_pathSuffix(pathSuffix)
{
}

ProxyAgent::~ProxyAgent()
{
}

QString ProxyAgent::capabilityToString(Agent::Capability capability)
{
    QString capabilityString;

    switch (capability) {
    case Agent::DisplayOnly:
        capabilityString = QStringLiteral("DisplayOnly");
        break;
    case Agent::DisplayYesNo:
        capabilityString = QStringLiteral("DisplayYesNo");
        break;
    case Agent::KeyboardOnly:
        capabilityString = QStringLiteral("KeyboardOnly");
        break;
    case Agent::NoInputNoOutput:
        capabilityString = QStringLiteral("NoInputNoOutput");
        break;
    default:
        capabilityString = QStringLiteral("DisplayYesNo");
        break;
    }

    return capabilityString;
}

Agent *ProxyAgent::agent() const
{
    return m_agent;
}

QDBusObjectPath ProxyAgent::objectPath() const
{
    if (!m_agent) {
        return QDBusObjectPath();
    }
    return QDBusObjectPath(m_agent->objectPath().path() + m_pathSuffix);
}

BluezQt::Agent::Capability ProxyAgent::capability() const
{
    if (!m_agent) {
        return BluezQt::Agent::DisplayOnly;
    }
    return m_agent->capability();
}

void ProxyAgent::requestPinCode(BluezQt::DevicePtr device, const BluezQt::Request<QString> &request)
{
    if (!m_agent) {
        request.reject();
        return;
    }
    m_agent->requestPinCode(device, request);
}

void ProxyAgent::displayPinCode(BluezQt::DevicePtr device, const QString &pinCode)
{
    if (!m_agent) {
        return;
    }
    m_agent->displayPinCode(device, pinCode);
}

void ProxyAgent::requestPasskey(BluezQt::DevicePtr device, const BluezQt::Request<quint32> &request)
{
    if (!m_agent) {
        request.reject();
        return;
    }
    m_agent->requestPasskey(device, request);
}

void ProxyAgent::displayPasskey(BluezQt::DevicePtr device, const QString &passkey, const QString &entered)
{
    if (!m_agent) {
        return;
    }
    m_agent->displayPasskey(device, passkey, entered);
}

void ProxyAgent::requestConfirmation(BluezQt::DevicePtr device, const QString &passkey, const BluezQt::Request<> &request)
{
    if (!m_agent) {
        request.reject();
        return;
    }
    m_agent->requestConfirmation(device, passkey, request);
}

void ProxyAgent::requestAuthorization(BluezQt::DevicePtr device, const BluezQt::Request<> &request)
{
    if (!m_agent) {
        request.reject();
        return;
    }
    m_agent->requestAuthorization(device, request);
}

void ProxyAgent::authorizeService(BluezQt::DevicePtr device, const QString &uuid, const BluezQt::Request<> &request)
{
    if (!m_agent) {
        request.reject();
        return;
    }
    m_agent->authorizeService(device, uuid, request);
}

void ProxyAgent::cancel()
{
    if (!m_agent) {
        return;
    }
    m_agent->cancel();
}

void ProxyAgent::release()
{
    if (m_agent) {
        m_agent->release();
    }

    emit agentReleased();
}

} // namespace BluezQt
