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
