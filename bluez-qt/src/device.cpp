/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * Copyright (C) 2014 David Rosca <nowrep@gmail.com>
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

#include "device.h"
#include "device_p.h"
#include "adapter.h"
#include "pendingcall.h"
#include "utils.h"

#if KF5BLUEZQT_BLUEZ_VERSION < 5
#include "bluez4/device_bluez4_p.h"
#endif

namespace BluezQt
{

Device::Device(const QString &path, const QVariantMap &properties, AdapterPtr adapter)
    : QObject()
    , d(new DevicePrivate(path, properties, adapter))
{
}

Device::~Device()
{
    delete d;
}

DevicePtr Device::toSharedPtr() const
{
    return d->q.toStrongRef();
}

QString Device::ubi() const
{
#if KF5BLUEZQT_BLUEZ_VERSION >= 5
    return d->m_bluezDevice->path();
#else
    return d->m_bluez4->m_bluez4Device->path();
#endif
}

QString Device::address() const
{
    return d->m_address;
}

QString Device::name() const
{
    return d->m_alias;
}

PendingCall *Device::setName(const QString &name)
{
    return new PendingCall(d->setDBusProperty(QStringLiteral("Alias"), name),
                           PendingCall::ReturnVoid, this);
}

QString Device::friendlyName() const
{
    if (name().isEmpty() || name() == remoteName()) {
        return name();
    }
    if (remoteName().isEmpty()) {
        return name();
    }
    return QStringLiteral("%1 (%2)").arg(name(), remoteName());
}

QString Device::remoteName() const
{
    return d->m_name;
}

quint32 Device::deviceClass() const
{
    return d->m_deviceClass;
}

Device::Type Device::type() const
{
    return classToType(d->m_deviceClass);
}

quint16 Device::appearance() const
{
    return d->m_appearance;
}

QString Device::icon() const
{
    switch (type()) {
    case Headset:
        return QStringLiteral("audio-headset");
    case Headphones:
        return QStringLiteral("audio-headphones");
    default:
        return d->m_icon.isEmpty() ? QStringLiteral("preferences-system-bluetooth") : d->m_icon;
    }
}

bool Device::isPaired() const
{
    return d->m_paired;
}

bool Device::isTrusted() const
{
    return d->m_trusted;
}

PendingCall *Device::setTrusted(bool trusted)
{
    return new PendingCall(d->setDBusProperty(QStringLiteral("Trusted"), trusted),
                           PendingCall::ReturnVoid, this);
}

bool Device::isBlocked() const
{
    return d->m_blocked;
}

PendingCall *Device::setBlocked(bool blocked)
{
    return new PendingCall(d->setDBusProperty(QStringLiteral("Blocked"), blocked),
                           PendingCall::ReturnVoid, this);
}

bool Device::hasLegacyPairing() const
{
    return d->m_legacyPairing;
}

qint16 Device::rssi() const
{
    return d->m_rssi;
}

bool Device::isConnected() const
{
    return d->m_connected;
}

QStringList Device::uuids() const
{
    return d->m_uuids;
}

QString Device::modalias() const
{
    return d->m_modalias;
}

InputPtr Device::input() const
{
    return d->m_input;
}

MediaPlayerPtr Device::mediaPlayer() const
{
    return d->m_mediaPlayer;
}

MediaTransportPtr Device::mediaTransport() const
{
    return d->m_mediaTransport;
}

AdapterPtr Device::adapter() const
{
    return d->m_adapter;
}

QString Device::typeToString(Device::Type type)
{
    switch (type) {
    case Device::Phone:
        return QStringLiteral("phone");
    case Device::Modem:
        return QStringLiteral("modem");
    case Device::Computer:
        return QStringLiteral("computer");
    case Device::Network:
        return QStringLiteral("network");
    case Device::Headset:
        return QStringLiteral("headset");
    case Device::Headphones:
        return QStringLiteral("headphones");
    case Device::AudioVideo:
        return QStringLiteral("audiovideo");
    case Device::Keyboard:
        return QStringLiteral("keyboard");
    case Device::Mouse:
        return QStringLiteral("mouse");
    case Device::Joypad:
        return QStringLiteral("joypad");
    case Device::Tablet:
        return QStringLiteral("tablet");
    case Device::Peripheral:
        return QStringLiteral("peripheral");
    case Device::Camera:
        return QStringLiteral("camera");
    case Device::Printer:
        return QStringLiteral("printer");
    case Device::Imaging:
        return QStringLiteral("imaging");
    case Device::Wearable:
        return QStringLiteral("wearable");
    case Device::Toy:
        return QStringLiteral("toy");
    case Device::Health:
        return QStringLiteral("health");
    default:
        return QStringLiteral("uncategorized");
    }
}

Device::Type Device::stringToType(const QString &typeString)
{
    if (typeString == QLatin1String("phone")) {
        return Device::Phone;
    } else if (typeString == QLatin1String("modem")) {
        return Device::Modem;
    } else if (typeString == QLatin1String("computer")) {
        return Device::Computer;
    } else if (typeString == QLatin1String("network")) {
        return Device::Network;
    } else if (typeString == QLatin1String("headset")) {
        return Device::Headset;
    } else if (typeString == QLatin1String("headphones")) {
        return Device::Headphones;
    } else if (typeString == QLatin1String("audio")) {
        return Device::AudioVideo;
    } else if (typeString == QLatin1String("keyboard")) {
        return Device::Keyboard;
    } else if (typeString == QLatin1String("mouse")) {
        return Device::Mouse;
    } else if (typeString == QLatin1String("joypad")) {
        return Device::Joypad;
    } else if (typeString == QLatin1String("tablet")) {
        return Device::Tablet;
    } else if (typeString == QLatin1String("peripheral")) {
        return Device::Peripheral;
    } else if (typeString == QLatin1String("camera")) {
        return Device::Camera;
    } else if (typeString == QLatin1String("printer")) {
        return Device::Printer;
    } else if (typeString == QLatin1String("imaging")) {
        return Device::Imaging;
    } else if (typeString == QLatin1String("wearable")) {
        return Device::Wearable;
    } else if (typeString == QLatin1String("toy")) {
        return Device::Toy;
    } else if (typeString == QLatin1String("health")) {
        return Device::Health;
    }
    return Device::Uncategorized;
}

PendingCall *Device::connectToDevice()
{
#if KF5BLUEZQT_BLUEZ_VERSION >= 5
    return new PendingCall(d->m_bluezDevice->Connect(), PendingCall::ReturnVoid, this);
#else
    // In BlueZ 4 there is no generic Connect(), so try connecting to the Input or Audio services.
    Type deviceType = type();
    if (deviceType >= Keyboard && deviceType <= Peripheral) {
        return new PendingCall(d->m_bluez4->orgBluezInput()->asyncCall(QStringLiteral("Connect")), PendingCall::ReturnVoid, this);
    } else {
        return new PendingCall(d->m_bluez4->orgBluezAudio()->asyncCall(QStringLiteral("Connect")), PendingCall::ReturnVoid, this);
    }
#endif
}

PendingCall *Device::disconnectFromDevice()
{
#if KF5BLUEZQT_BLUEZ_VERSION >= 5
    return new PendingCall(d->m_bluezDevice->Disconnect(), PendingCall::ReturnVoid, this);
#else
    return new PendingCall(d->m_bluez4->m_bluez4Device->Disconnect(), PendingCall::ReturnVoid, this);
#endif
}

PendingCall *Device::connectProfile(const QString &uuid)
{
#if KF5BLUEZQT_BLUEZ_VERSION >= 5
    return new PendingCall(d->m_bluezDevice->ConnectProfile(uuid), PendingCall::ReturnVoid, this);
#else
    Q_UNUSED(uuid)
    return new PendingCall(PendingCall::NotSupported, QStringLiteral("Device::connectProfile() not supported for BlueZ 4"));
#endif
}

PendingCall *Device::disconnectProfile(const QString &uuid)
{
#if KF5BLUEZQT_BLUEZ_VERSION >= 5
    return new PendingCall(d->m_bluezDevice->DisconnectProfile(uuid), PendingCall::ReturnVoid, this);
#else
    Q_UNUSED(uuid)
    return new PendingCall(PendingCall::NotSupported, QStringLiteral("Device::disconnectProfile() not supported for BlueZ 4"));
#endif
}

PendingCall *Device::pair()
{
#if KF5BLUEZQT_BLUEZ_VERSION >= 5
    return new PendingCall(d->m_bluezDevice->Pair(), PendingCall::ReturnVoid, this);
#else
    if (d->m_adapter.isNull()) {
        return new PendingCall(PendingCall::InternalError, QStringLiteral("Invalid adapter"), this);
    }
    return new PendingCall(d->m_bluez4->pair(d->m_adapter), PendingCall::ReturnObjectPath, this);
#endif
}

PendingCall *Device::cancelPairing()
{
#if KF5BLUEZQT_BLUEZ_VERSION >= 5
    return new PendingCall(d->m_bluezDevice->CancelPairing(), PendingCall::ReturnVoid, this);
#else
    if (d->m_adapter.isNull()) {
        return new PendingCall(PendingCall::InternalError, QStringLiteral("Invalid adapter"), this);
    }
    return new PendingCall(d->m_bluez4->cancelPairing(d->m_adapter), PendingCall::ReturnVoid, this);
#endif
}

} // namespace BluezQt
