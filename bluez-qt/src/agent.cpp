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

#include "agent.h"

#if KF5BLUEZQT_BLUEZ_VERSION < 5
#include <QDBusObjectPath>
#endif

namespace BluezQt
{

Agent::Agent(QObject *parent)
    : QObject(parent)
{
}

Agent::Capability Agent::capability() const
{
    return DisplayYesNo;
}

void Agent::requestPinCode(DevicePtr device, const Request<QString> &request)
{
    Q_UNUSED(device)

    request.cancel();
}

void Agent::displayPinCode(DevicePtr device, const QString &pinCode)
{
    Q_UNUSED(device)
    Q_UNUSED(pinCode)
}

void Agent::requestPasskey(DevicePtr device, const Request<quint32> &request)
{
    Q_UNUSED(device)

    request.cancel();
}

void Agent::displayPasskey(DevicePtr device, const QString &passkey, const QString &entered)
{
    Q_UNUSED(device)
    Q_UNUSED(passkey)
    Q_UNUSED(entered)
}

void Agent::requestConfirmation(DevicePtr device, const QString &passkey, const Request<> &request)
{
    Q_UNUSED(device)
    Q_UNUSED(passkey)

    request.cancel();
}

void Agent::requestAuthorization(DevicePtr device, const Request<> &request)
{
    Q_UNUSED(device)

    request.cancel();
}

void Agent::authorizeService(DevicePtr device, const QString &uuid, const Request<> &request)
{
    Q_UNUSED(device)
    Q_UNUSED(uuid)

    request.cancel();
}

void Agent::cancel()
{
}

void Agent::release()
{
}

#if KF5BLUEZQT_BLUEZ_VERSION < 5
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
#endif

} // namespace BluezQt
