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

#include "mediatransport.h"
#include "mediatransport_p.h"
#include "pendingcall.h"

namespace BluezQt
{

MediaTransport::MediaTransport(const QString &path, const QVariantMap &properties)
    : QObject()
    , d(new MediaTransportPrivate(path, properties))
{
}

MediaTransport::~MediaTransport()
{
    delete d;
}

MediaTransportPtr MediaTransport::toSharedPtr() const
{
    return d->q.toStrongRef();
}

QString MediaTransport::uuid() const
{
    return d->m_uuid;
}

quint8 MediaTransport::codec() const
{
    return d->m_codec;
}

QByteArray MediaTransport::configuration() const
{
    return d->m_configuration;
}

MediaTransport::State MediaTransport::state() const
{
    return d->m_state;
}

quint16 MediaTransport::delay() const
{
    return d->m_delay;
}

PendingCall *MediaTransport::setDelay(quint16 delay)
{
    return new PendingCall(d->setDBusProperty(QStringLiteral("Delay"), delay),
                           PendingCall::ReturnVoid, this);
}

quint16 MediaTransport::volume() const
{
    return d->m_volume;
}

PendingCall *MediaTransport::setVolume(quint16 volume)
{
    return new PendingCall(d->setDBusProperty(QStringLiteral("Volume"), volume),
                           PendingCall::ReturnVoid, this);
}

PendingCall *MediaTransport::acquire()
{
    return new PendingCall(d->m_bluezMediaTransport->Acquire(),
                           PendingCall::ReturnMediaTransportSocketInfo, this);
}

PendingCall *MediaTransport::tryAcquire()
{
    return new PendingCall(d->m_bluezMediaTransport->TryAcquire(),
                           PendingCall::ReturnMediaTransportSocketInfo, this);
}

PendingCall *MediaTransport::release()
{
    return new PendingCall(d->m_bluezMediaTransport->Release(),
                           PendingCall::ReturnVoid, this);
}

} // namespace BluezQt
