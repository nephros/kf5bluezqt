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
