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
