/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * Copyright (C) 2015 David Rosca <nowrep@gmail.com>
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

#include "mediatransport_bluez4_p.h"
#include "macros_bluez4_p.h"

#include "utils.h"
#include "mediatransport_p.h"

namespace BluezQt
{

MediaTransportBluez4::MediaTransportBluez4(MediaTransportPrivate *mediaTransportPrivate, const QString &path)
    : QObject(mediaTransportPrivate)
    , m_mediaTransportPrivate(mediaTransportPrivate)
{
    m_bluez4MediaTransport = new Bluez4MediaTransport(Strings::orgBluez(), path, DBusConnection::orgBluez(), this);

    connect(m_bluez4MediaTransport, &Bluez4MediaTransport::PropertyChanged,
            this, &MediaTransportBluez4::mediaTransportPropertyChanged);
}

void MediaTransportBluez4::mediaTransportPropertyChanged(const QString &property, const QDBusVariant &value)
{
    INVOKE_PROPERTIES_CHANGED(Strings::orgBluezMediaTransport1(), m_mediaTransportPrivate, property, value.variant());
}

} // namespace BluezQt
