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

#ifndef BLUEZQT_OBEXAGENTADAPTOR_H
#define BLUEZQT_OBEXAGENTADAPTOR_H

#include <QDBusAbstractAdaptor>

#include "types.h"
#include "request.h"

class QDBusObjectPath;
class QDBusPendingCallWatcher;

namespace BluezQt
{

class ObexAgent;
class ObexManager;

class ObexAgentAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
#if KF5BLUEZQT_BLUEZ_VERSION >= 5
    Q_CLASSINFO("D-Bus Interface", "org.bluez.obex.Agent1")
#else
    Q_CLASSINFO("D-Bus Interface", "org.bluez.obex.Agent")
#endif

public:
    explicit ObexAgentAdaptor(ObexAgent *parent, ObexManager *manager);

public Q_SLOTS:
#if KF5BLUEZQT_BLUEZ_VERSION >= 5
    QString AuthorizePush(const QDBusObjectPath &transfer, const QDBusMessage &msg);
#else
    QString Authorize(const QDBusObjectPath &transfer, const QString &bt_address,
                      const QString &name, const QString &type, qint32 length,
                      qint32 time, const QDBusMessage &msg);
#endif

    Q_NOREPLY void Cancel();
    Q_NOREPLY void Release();

private Q_SLOTS:
#if KF5BLUEZQT_BLUEZ_VERSION >= 5
    void getPropertiesFinished(QDBusPendingCallWatcher *watcher);
#else
    void getPropertiesFinished(ObexTransferPtr transfer);
#endif

private:
    ObexAgent *m_agent;
    ObexManager *m_manager;
    QString m_transferPath;
    Request<QString> m_transferRequest;
};

} // namespace BluezQt

#endif // BLUEZQT_OBEXAGENTADAPTOR_H
