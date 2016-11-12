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
