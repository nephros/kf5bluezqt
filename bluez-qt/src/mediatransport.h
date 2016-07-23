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

#ifndef BLUEZQT_MEDIATRANSPORT_H
#define BLUEZQT_MEDIATRANSPORT_H

#include <QObject>

#include "types.h"
#include "bluezqt_export.h"

namespace BluezQt
{

class PendingCall;

/**
 * Media transport.
 *
 * This class represents a media transport interface.
 */
class BLUEZQT_EXPORT MediaTransport : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString uuid READ uuid NOTIFY uuidChanged)
    Q_PROPERTY(quint8 codec READ codec NOTIFY codecChanged)
    Q_PROPERTY(QByteArray configuration READ configuration NOTIFY configurationChanged)
    Q_PROPERTY(State state READ state NOTIFY stateChanged)
    Q_PROPERTY(quint16 delay READ delay WRITE setDelay NOTIFY delayChanged)
    Q_PROPERTY(quint16 volume READ volume WRITE setVolume NOTIFY volumeChanged)

public:
    /** Stream state. */
    enum State {
        /* Not streaming. */
        StreamIdle,
        /* Streaming but not acquired. */
        StreamPending,        
        /* Streaming and acquired. */
        StreamActive
    };

    /**
     * Destroys a MediaTransport object.
     */
    ~MediaTransport();

    /**
     * Returns a shared pointer from this.
     *
     * @return MediaTransportPtr
     */
    MediaTransportPtr toSharedPtr() const;

    /**
     * Returns the UUID of the transport's profile.
     *
     * @return UUID of profile
     */
    QString uuid() const;

    /**
     * Returns the assigned number of the codec that the transport
     * supports, according to the transport's profile.
     *
     * @return codec assigned number
     */
    quint8 codec() const;

    /**
     * Returns configuration blob data for the transport
     *
     * @return configuration blob data
     */
    QByteArray configuration() const;

    /**
     * Returns the current transport state.
     *
     * @return current transport state
     */
    State state() const;

    /**
     * Returns the transport delay.
     *
     * @return transport delay
     */
    quint16 delay() const;

    /**
     * Sets the transport delay, in units of 1/10 of a millisecond.
     *
     * The transport delay is only writeable when the transport was
     * acquired by the sender.
     *
     * @param delay transport delay
     * @return void pending call
     */
    PendingCall *setDelay(quint16 delay);

    /**
     * Returns the transport volume.
     *
     * @return transport volume
     */
    quint16 volume() const;

    /**
     * Sets the transport volume, within a range of 0-127.
     *
     * The transport volume is only writeable when the transport was
     * acquired by the sender.
     *
     * @param volume transport volume
     * @return void pending call
     */
    PendingCall *setVolume(quint16 volume);

public Q_SLOTS:

    /**
     * Acquires a media transport stream.
     *
     * Returns a MediaTransportSocketInfo object.
     *
     * Possible errors: org.bluez.Error.NotAuthorized, org.bluez.Error.Failed
     *
     * @return void pending call
     */
    PendingCall *acquire();

    /**
     * Acquires a media transport stream if the transport is in the
     * StreamPending state. Otherwise, this function errors with
     * PendingCall::NotAvailable.
     *
     * Returns a MediaTransportSocketInfo object.
     *
     * Possible errors: PendingCall::InvalidArguments, PendingCall::Failed, PendingCall::NotAvailable
     *
     * @return void pending call
     */
    PendingCall *tryAcquire();

    /**
     * Releases the transport file descriptor.
     *
     * @return void pending call
     */
    PendingCall *release();

Q_SIGNALS:
    /**
     * Indicates that the transport's UUID changed.
     */
    void uuidChanged(const QString &uuid);

    /**
     * Indicates that the transport's codec changed.
     */
    void codecChanged(short codec);

    /**
     * Indicates that the transport's configuration changed.
     */
    void configurationChanged(const QByteArray &configuration);

    /**
     * Indicates that the transport's state changed.
     */
    void stateChanged(MediaTransport::State state);

    /**
     * Indicates that the transport's delay changed.
     */
    void delayChanged(quint16 delay);

    /**
     * Indicates that the transport's volume changed.
     */
    void volumeChanged(quint16 volume);

private:
    explicit MediaTransport(const QString &path, const QVariantMap &properties);

    class MediaTransportPrivate *const d;

    friend class MediaTransportPrivate;
    friend class DevicePrivate;
};

} // namespace BluezQt

#endif // BLUEZQT_MEDIATRANSPORT_H
