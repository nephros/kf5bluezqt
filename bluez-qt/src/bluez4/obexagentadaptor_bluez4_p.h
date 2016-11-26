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

#ifndef BLUEZQT_OBEXAGENTADAPTOR_BLUEZ4_H
#define BLUEZQT_OBEXAGENTADAPTOR_BLUEZ4_H

#include <QDBusAbstractAdaptor>

#include "types.h"
#include "request.h"

class QDBusObjectPath;

namespace BluezQt
{

class ObexAgent;
class ObexManagerBluez4;

class ObexAgentAdaptorBluez4 : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.bluez.obex.Agent")

public:
    explicit ObexAgentAdaptorBluez4(ObexAgent *parent, ObexManagerBluez4 *manager);

public Q_SLOTS:
    QString Authorize(const QDBusObjectPath &transfer, const QString &bt_address,
                      const QString &name, const QString &type, qint32 length,
                      qint32 time, const QDBusMessage &msg);

    Q_NOREPLY void Cancel();
    Q_NOREPLY void Release();

private Q_SLOTS:
    void authorizeTransfer(const QString &transferPath,
                           const QVariantMap &transferProperties,
                           const QString &destinationAddress);

private:
    ObexAgent *m_agent;
    ObexManagerBluez4 *m_manager;
    QString m_transferPath;
    Request<QString> m_transferRequest;
};

} // namespace BluezQt

#endif // BLUEZQT_OBEXAGENTADAPTOR_BLUEZ4_H
