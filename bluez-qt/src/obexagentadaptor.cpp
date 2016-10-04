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

#include "obexagentadaptor.h"
#include "obexagent.h"
#include "obexmanager.h"
#include "obextransfer.h"
#include "obextransfer_p.h"
#if KF5BLUEZQT_BLUEZ_VERSION >= 5
#include "dbusproperties.h"
#else
#include "obexmanager_p.h"
#endif
#include "utils.h"

#include <QDBusObjectPath>

namespace BluezQt
{

#if KF5BLUEZQT_BLUEZ_VERSION >= 5
typedef org::freedesktop::DBus::Properties DBusProperties;
#endif

ObexAgentAdaptor::ObexAgentAdaptor(ObexAgent *parent, ObexManager *manager)
    : QDBusAbstractAdaptor(parent)
    , m_agent(parent)
    , m_manager(manager)
{
}

#if KF5BLUEZQT_BLUEZ_VERSION >= 5
QString ObexAgentAdaptor::AuthorizePush(const QDBusObjectPath &transfer, const QDBusMessage &msg)
#else
QString ObexAgentAdaptor::Authorize(const QDBusObjectPath &transfer, const QString &bt_address,
                                    const QString &name, const QString &type, qint32 length,
                                    qint32 time, const QDBusMessage &msg)
#endif
{
    msg.setDelayedReply(true);
    m_transferRequest = Request<QString>(OrgBluezObexAgent, msg);
    m_transferPath = transfer.path();

#if KF5BLUEZQT_BLUEZ_VERSION >= 5
    DBusProperties dbusProperties(Strings::orgBluezObex(), m_transferPath, DBusConnection::orgBluezObex(), this);

    const QDBusPendingReply<QVariantMap> &call = dbusProperties.GetAll(Strings::orgBluezObexTransfer1());
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, &ObexAgentAdaptor::getPropertiesFinished);
#else
    m_transferRequest.setObjectPushTransferPath(transfer.path());

    // Assemble the map of property values. In BlueZ 4, transfer objects provided by the org.bluez.obex
    // service for OBEX agents (as opposed to those provided by the org.bluez.obex.client service
    // for OBEX clients) don't have persistent properties provided by a GetProperties() function;
    // instead, all values are provided by the AuthorizePush() arguments, so initialize them here.
    // Note 'Filename' is not added here as it not available in BlueZ 4 and is optional in BlueZ 5.
    QVariantMap properties;
    properties.insert(QStringLiteral("Status"), QStringLiteral("queued"));
    properties.insert(QStringLiteral("Name"), name);
    properties.insert(QStringLiteral("Type"), type);
    if (time > 0) {
        properties.insert(QStringLiteral("Time"), static_cast<quint64>(time));
    }
    if (length > 0) {
        properties.insert(QStringLiteral("Size"), static_cast<quint64>(length));
    }
    properties.insert(QStringLiteral("Transferred"), 0);

    // For BlueZ 4 there is only FTP session support, so manually add per-transfer OPP sessions.
    ObexTransferPtr transferPtr = m_manager->d->newObjectPushTransfer(transfer, properties, bt_address);
    getPropertiesFinished(transferPtr);
#endif

    return QString();
}

void ObexAgentAdaptor::Cancel()
{
    m_agent->cancel();
}

void ObexAgentAdaptor::Release()
{
    m_agent->release();
}

#if KF5BLUEZQT_BLUEZ_VERSION >= 5
void ObexAgentAdaptor::getPropertiesFinished(QDBusPendingCallWatcher *watcher)
{
    const QDBusPendingReply<QVariantMap> &reply = *watcher;
    watcher->deleteLater();

    if (reply.isError()) {
        m_transferRequest.cancel();
        return;
    }

    ObexTransferPtr transfer = ObexTransferPtr(new ObexTransfer(m_transferPath, reply.value()));
    transfer->d->q = transfer.toWeakRef();
#else
void ObexAgentAdaptor::getPropertiesFinished(ObexTransferPtr transfer)
{
#endif

    ObexSessionPtr session = m_manager->sessionForPath(transfer->objectPath());
    Q_ASSERT(session);

    if (!session) {
        m_transferRequest.cancel();
        return;
    }

    m_agent->authorizePush(transfer, session, m_transferRequest);
}

} // namespace BluezQt
