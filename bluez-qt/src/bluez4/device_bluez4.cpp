/****************************************************************************
**
** Copyright (C) 2016 Jolla Ltd.
** Contact: Bea Lam <bea.lam@jolla.com>
**
** This file is free software; you can redistribute it and/or
** modify it under the terms of the GNU Lesser General Public
** License as published by the Free Software Foundation; either
** version 2.1 of the License, or (at your option) any later version.
**
** This file is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public
** License along with this file; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
**
****************************************************************************/

#include "device_bluez4_p.h"
#include "adapter_bluez4_p.h"
#include "macros_bluez4_p.h"

#include "device_p.h"
#include "adapter.h"
#include "adapter_p.h"
#include "utils.h"
#include "macros.h"

namespace BluezQt
{

DeviceBluez4::DeviceBluez4(DevicePrivate *devicePrivate, const QString &path)
    : QObject(devicePrivate)
    , m_devicePrivate(devicePrivate)
    , m_inputInterface(0)
    , m_audioInterface(0)
{
    m_bluez4Device = new Bluez4Device(Strings::orgBluez(), path,
            DBusConnection::orgBluez(), this);
    connect(m_bluez4Device, &Bluez4Device::PropertyChanged,
            this, &DeviceBluez4::devicePropertyChanged);
}

QDBusInterface *DeviceBluez4::orgBluezInput()
{
    if (!m_inputInterface) {
        m_inputInterface = new QDBusInterface(Strings::orgBluez(), m_devicePrivate->q.data()->ubi(),
                QStringLiteral("org.bluez.Input"), DBusConnection::orgBluez(), this);
    }
    return m_inputInterface;
}

QDBusInterface *DeviceBluez4::orgBluezAudio()
{
    if (!m_audioInterface) {
        m_audioInterface = new QDBusInterface(Strings::orgBluez(), m_devicePrivate->q.data()->ubi(),
                QStringLiteral("org.bluez.Audio"), DBusConnection::orgBluez(), this);
    }
    return m_audioInterface;
}

QDBusPendingReply<QDBusObjectPath> DeviceBluez4::pair(AdapterPtr adapter)
{
    if (adapter.isNull() || !adapter->d->m_bluez4) {
        return QDBusMessage::createError(QDBusError::InvalidArgs, "Invalid adapter");
    }

    return adapter->d->m_bluez4->createPairedDevice(m_devicePrivate->m_address);
}

QDBusPendingReply<void> DeviceBluez4::cancelPairing(AdapterPtr adapter)
{
    if (adapter.isNull() || !adapter->d->m_bluez4) {
        return QDBusMessage::createError(QDBusError::InvalidArgs, "Invalid adapter");
    }

    return adapter->d->m_bluez4->cancelDeviceCreation(m_devicePrivate->m_address);
}

void DeviceBluez4::devicePropertyChanged(const QString &property, const QDBusVariant &value)
{
    INVOKE_PROPERTIES_CHANGED(Strings::orgBluezDevice1(), m_devicePrivate, property, value.variant());
}

} // namespace BluezQt
