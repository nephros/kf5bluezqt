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

#include "mediatransportsocketinfo.h"

namespace BluezQt
{

class MediaTransportSocketInfoPrivate
{
public:
    qintptr m_descriptor;
    quint16 m_readMTU;
    quint16 m_writeMTU;
};

MediaTransportSocketInfo::MediaTransportSocketInfo()
    : d(new MediaTransportSocketInfoPrivate)
{
    d->m_descriptor = -1;
    d->m_readMTU = 0;
    d->m_writeMTU = 0;
}

MediaTransportSocketInfo::MediaTransportSocketInfo(qintptr descriptor, quint64 readMTU, quint64 writeMTU)
    : d(new MediaTransportSocketInfoPrivate)
{
    d->m_descriptor = descriptor;
    d->m_readMTU = readMTU;
    d->m_writeMTU = writeMTU;
}

MediaTransportSocketInfo::~MediaTransportSocketInfo()
{
}

MediaTransportSocketInfo::MediaTransportSocketInfo(const MediaTransportSocketInfo &other)
    : d(other.d)
{
}

MediaTransportSocketInfo &MediaTransportSocketInfo::operator=(const MediaTransportSocketInfo &other)
{
    if (d != other.d) {
        d = other.d;
    }
    return *this;
}

bool MediaTransportSocketInfo::isValid() const
{
    return d->m_descriptor != -1;
}

qintptr MediaTransportSocketInfo::socketDescriptor() const
{
    return d->m_descriptor;
}

quint64 MediaTransportSocketInfo::readMTU() const
{
    return d->m_readMTU;
}

quint64 MediaTransportSocketInfo::writeMTU() const
{
    return d->m_writeMTU;
}

} // namespace BluezQt
