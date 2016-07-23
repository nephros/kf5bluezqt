/*
 * BluezQt - Asynchronous BlueZ wrapper library
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

#ifndef BLUEZQT_MEDIATRANSPORTSOCKETINFO_H
#define BLUEZQT_MEDIATRANSPORTSOCKETINFO_H

#include <QString>
#include <QSharedPointer>

#include "bluezqt_export.h"

namespace BluezQt
{

/**
 * Media transport socket information.
 */
class BLUEZQT_EXPORT MediaTransportSocketInfo
{
public:
    /**
     * Creates a new invalid MediaTransportSocketInfo object.
     */
    explicit MediaTransportSocketInfo();

    /**
     * Destroys an MediaTransportSocketInfo object.
     */
    virtual ~MediaTransportSocketInfo();

    /**
     * Copy constructor.
     *
     * @param other
     */
    MediaTransportSocketInfo(const MediaTransportSocketInfo &other);

    /**
     * Copy assignment operator.
     *
     * @param other
     */
    MediaTransportSocketInfo &operator=(const MediaTransportSocketInfo &other);

    /**
     * Returns whether this socket info is valid.
     *
     * @return true if socket info is valid
     */
    bool isValid() const;

    /**
     * Returns the native socket descriptor of the transport, or -1
     * if this object is invalid.
     *
     * @return transport file descriptor
     */
    qintptr socketDescriptor() const;

    /**
     * Returns the read MTU of the transport socket.
     *
     * @return read MTU of transport
     */
    quint64 readMTU() const;

    /**
     * Returns the write MTU of the transport socket.
     *
     * @return write MTU of transport
     */
    quint64 writeMTU() const;

private:
    explicit MediaTransportSocketInfo(qintptr descriptor, quint64 readMTU, quint64 writeMTU);

    QSharedPointer<class MediaTransportSocketInfoPrivate> d;

    friend class PendingCallPrivate;
};

} // namespace BluezQt

Q_DECLARE_METATYPE(BluezQt::MediaTransportSocketInfo)

#endif // BLUEZQT_MEDIATRANSPORTSOCKETINFO_H
