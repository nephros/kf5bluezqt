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


#ifndef BLUEZQT_OBEXSESSION_BLUEZ4_P_H
#define BLUEZQT_OBEXSESSION_BLUEZ4_P_H

#include <QVariantMap>
#include <QDBusObjectPath>

namespace BluezQt
{

class ObexSessionBluez4
{
public:
    ObexSessionBluez4();
    ObexSessionBluez4(const QString &path, const QVariantMap &properties);

    static QString objectPushTransferPathKey();

    bool m_valid;
    QDBusObjectPath m_sessionPath;
    QString m_oppTransferPath;
    QVariantMap m_properties;
};

} // namespace BluezQt

#endif // BLUEZQT_OBEXSESSION_BLUEZ4_P_H
