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

#ifndef BLUEZQT_DBUSTYPES_BLUEZ4_H
#define BLUEZQT_DBUSTYPES_BLUEZ4_H

#include <QVariantMap>
#include <QDBusObjectPath>

typedef QMap<quint32, QString> DeviceServiceMap;
Q_DECLARE_METATYPE(DeviceServiceMap)

typedef QPair<QDBusObjectPath, QVariantMap> ObexTransferInfo;
Q_DECLARE_METATYPE(ObexTransferInfo)

#endif // BLUEZQT_DBUSTYPES_H
