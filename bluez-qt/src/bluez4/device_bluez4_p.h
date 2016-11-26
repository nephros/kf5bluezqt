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

#ifndef BLUEZQT_DEVICE_BLUEZ4_P_H
#define BLUEZQT_DEVICE_BLUEZ4_P_H

#include "types.h"
#include "bluez4device.h"

namespace BluezQt
{

typedef org::bluez::Device Bluez4Device;

class DevicePrivate;
class AdapterPrivate;

class DeviceBluez4 : public QObject
{
    Q_OBJECT

public:
    explicit DeviceBluez4(DevicePrivate *devicePrivate, const QString &path);

    QDBusInterface *orgBluezInput();
    QDBusInterface *orgBluezAudio();

    QDBusPendingReply<QDBusObjectPath> pair(AdapterPtr adapter);
    QDBusPendingReply<void> cancelPairing(AdapterPtr adapter);

    DevicePrivate *m_devicePrivate;
    Bluez4Device *m_bluez4Device;

private:
    void devicePropertyChanged(const QString &property, const QDBusVariant &value);

    QDBusInterface *m_inputInterface;
    QDBusInterface *m_audioInterface;
};

} // namespace BluezQt

#endif // BLUEZQT_DEVICE_BLUEZ4_P_H
