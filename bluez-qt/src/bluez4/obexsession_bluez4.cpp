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

#include "obexsession_bluez4_p.h"

#include <QDBusObjectPath>

namespace BluezQt
{

ObexSessionBluez4::ObexSessionBluez4()
    : m_valid(false)
{
}

ObexSessionBluez4::ObexSessionBluez4(const QString &path, const QVariantMap &properties)
    : m_valid(true)
{
    m_sessionPath = QDBusObjectPath(path);
    m_oppTransferPath = properties.value(objectPushTransferPathKey()).toString();
    m_properties = properties;
}

QString ObexSessionBluez4::objectPushTransferPathKey()
{
    return QStringLiteral("org.kde.bluezqt.ObexTransfer.ObjectPush.Path");
}

} // namespace BluezQt
