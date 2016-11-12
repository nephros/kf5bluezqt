/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * Copyright (C) 2016 Bea Lam <bea.lam@jollamobile.com>
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

#ifndef BLUEZQT_MEDIATRANSPORT_P_H
#define BLUEZQT_MEDIATRANSPORT_P_H

#include <QObject>
#if KF5BLUEZQT_BLUEZ_VERSION < 5
#include <QDBusPendingReply>
#endif

#include "mediatransport.h"
#if KF5BLUEZQT_BLUEZ_VERSION >= 5
#include "bluezmediatransport1.h"
#include "dbusproperties.h"
#endif

namespace BluezQt
{

#if KF5BLUEZQT_BLUEZ_VERSION >= 5
typedef org::bluez::MediaTransport1 BluezMediaTransport;
typedef org::freedesktop::DBus::Properties DBusProperties;
#else
class MediaTransportBluez4;
#endif

class MediaTransportPrivate : public QObject
{
    Q_OBJECT

public:
    explicit MediaTransportPrivate(const QString &path, const QVariantMap &properties);

    void init(const QVariantMap &properties);

    QDBusPendingReply<> setDBusProperty(const QString &name, const QVariant &value);
    void propertiesChanged(const QString &interface, const QVariantMap &changed, const QStringList &invalidated);

    QWeakPointer<MediaTransport> q;
#if KF5BLUEZQT_BLUEZ_VERSION >= 5
    BluezMediaTransport *m_bluezMediaTransport;
    DBusProperties *m_dbusProperties;
#endif

    QString m_uuid;
    quint8 m_codec;
    QByteArray m_configuration;
    MediaTransport::State m_state;
    quint16 m_delay;
    quint16 m_volume;

#if KF5BLUEZQT_BLUEZ_VERSION < 5
    MediaTransportBluez4 *m_bluez4;
#endif
};

} // namespace BluezQt

#endif // BLUEZQT_MEDIATRANSPORT_P_H
