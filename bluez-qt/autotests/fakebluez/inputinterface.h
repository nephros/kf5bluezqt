/*
 * Copyright (C) 2015 David Rosca <nowrep@gmail.com>
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

#ifndef INPUTINTERFACE_H
#define INPUTINTERFACE_H

#include "object.h"

#include <QDBusAbstractAdaptor>

class QDBusMessage;
class QDBusObjectPath;

class InputObject : public QObject
{
public:
    explicit InputObject(const QDBusObjectPath &path, QObject *parent = Q_NULLPTR);
};

class InputInterface : public QDBusAbstractAdaptor, public Object
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.bluez.Input1")
    Q_PROPERTY(QString ReconnectMode READ reconnectMode)

public:
    explicit InputInterface(const QDBusObjectPath &path, const QVariantMap &properties, QObject *parent = Q_NULLPTR);

    QString reconnectMode() const;
};

#endif // INPUTINTERFACE_H
