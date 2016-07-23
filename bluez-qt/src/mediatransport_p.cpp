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

#include "mediatransport_p.h"
#include "utils.h"
#include "macros.h"

namespace BluezQt
{

static MediaTransport::State stringToState(const QString &state)
{
    if (state == QLatin1String("idle")) {
        return MediaTransport::StreamIdle;
    } else if (state == QLatin1String("pending")) {
        return MediaTransport::StreamPending;
    }
    return MediaTransport::StreamActive;
}

MediaTransportPrivate::MediaTransportPrivate(const QString &path, const QVariantMap &properties)
    : QObject()
    , m_dbusProperties(0)
    , m_codec(0)
    , m_state(MediaTransport::StreamIdle)
    , m_delay(0)
    , m_volume(0)
{
    m_bluezMediaTransport = new BluezMediaTransport(Strings::orgBluez(), path, DBusConnection::orgBluez(), this);

    init(properties);
}

void MediaTransportPrivate::init(const QVariantMap &properties)
{
    m_dbusProperties = new DBusProperties(Strings::orgBluez(), m_bluezMediaTransport->path(),
                                          DBusConnection::orgBluez(), this);

    connect(m_dbusProperties, &DBusProperties::PropertiesChanged,
            this, &MediaTransportPrivate::propertiesChanged, Qt::QueuedConnection);

    // Init properties
    m_uuid = properties.value(QStringLiteral("UUID")).toString();
    m_codec = properties.value(QStringLiteral("Codec")).toUInt();
    m_configuration = properties.value(QStringLiteral("Configuration")).toByteArray();
    m_state = stringToState(properties.value(QStringLiteral("State")).toString());
    m_delay = properties.value(QStringLiteral("Delay")).toUInt();
    m_volume = properties.value(QStringLiteral("Volume")).toUInt();
}

QDBusPendingReply<> MediaTransportPrivate::setDBusProperty(const QString &name, const QVariant &value)
{
    return m_dbusProperties->Set(Strings::orgBluezMediaTransport1(), name, QDBusVariant(value));
}

void MediaTransportPrivate::propertiesChanged(const QString &interface, const QVariantMap &changed, const QStringList &invalidated)
{
    if (interface != Strings::orgBluezMediaTransport1()) {
        return;
    }

    QVariantMap::const_iterator i;
    for (i = changed.constBegin(); i != changed.constEnd(); ++i) {
        const QVariant &value = i.value();
        const QString &property = i.key();

        if (property == QLatin1String("UUID")) {
            PROPERTY_CHANGED(m_uuid, toString, uuidChanged);
        } else if (property == QLatin1String("Codec")) {
            PROPERTY_CHANGED(m_codec, toUInt, codecChanged);
        } else if (property == QLatin1String("Configuration")) {
            PROPERTY_CHANGED(m_configuration, toByteArray, configurationChanged);
        } else if (property == QLatin1String("State")) {
            PROPERTY_CHANGED2(m_state, stringToState(value.toString()), stateChanged);
        } else if (property == QLatin1String("Delay")) {
            PROPERTY_CHANGED(m_delay, toUInt, delayChanged);
        } else if (property == QLatin1String("Volume")) {
            PROPERTY_CHANGED(m_volume, toUInt, volumeChanged);
        }
    }

    Q_FOREACH (const QString &property, invalidated) {
        if (property == QLatin1String("UUID")) {
            PROPERTY_INVALIDATED(m_uuid, QString(), uuidChanged);
        } else if (property == QLatin1String("Codec")) {
            PROPERTY_INVALIDATED(m_codec, 0, codecChanged);
        } else if (property == QLatin1String("Configuration")) {
            PROPERTY_INVALIDATED(m_configuration, QByteArray(), configurationChanged);
        } else if (property == QLatin1String("State")) {
            PROPERTY_INVALIDATED(m_state, MediaTransport::StreamIdle, stateChanged);
        } else if (property == QLatin1String("Delay")) {
            PROPERTY_INVALIDATED(m_delay, 0, delayChanged);
        } else if (property == QLatin1String("Volume")) {
            PROPERTY_INVALIDATED(m_volume, 0, volumeChanged);
        }
    }
}

} // namespace BluezQt
