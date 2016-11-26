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

#ifndef BLUEZQT_PROXYAGENT_H
#define BLUEZQT_PROXYAGENT_H

#include <QObject>

#include "agent.h"

namespace BluezQt
{

class ProxyAgent : public Agent
{
    Q_OBJECT

public:
    ProxyAgent(Agent *agent, const QString &pathSuffix, QObject *parent = Q_NULLPTR);
    ~ProxyAgent();

    Agent *agent() const;

    QDBusObjectPath objectPath() const;
    Capability capability() const;

    void requestPinCode(BluezQt::DevicePtr device, const BluezQt::Request<QString> &request);
    void displayPinCode(BluezQt::DevicePtr device, const QString &pinCode);
    void requestPasskey(BluezQt::DevicePtr device, const BluezQt::Request<quint32> &request);
    void displayPasskey(BluezQt::DevicePtr device, const QString &passkey, const QString &entered);
    void requestConfirmation(BluezQt::DevicePtr device, const QString &passkey, const BluezQt::Request<> &request);
    void requestAuthorization(BluezQt::DevicePtr device, const BluezQt::Request<> &request);
    void authorizeService(BluezQt::DevicePtr device, const QString &uuid, const BluezQt::Request<> &request);
    void cancel();
    void release();

    static QString capabilityToString(Agent::Capability capability);

Q_SIGNALS:
    void agentReleased();

private:
    Agent *m_agent;
    QString m_pathSuffix;
};

} // namespace BluezQt

#endif // BLUEZQT_PROXYAGENT_H
