/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * Copyright (C) 2014-2015 David Rosca <nowrep@gmail.com>
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

#ifndef BLUEZQT_OBEXSESSION_P_H
#define BLUEZQT_OBEXSESSION_P_H

#if KF5BLUEZQT_BLUEZ_VERSION >= 5
#include "obexsession1.h"
#else
#include "obexsession.h"
#include "bluez4/obexsession_bluez4_p.h"
#endif

namespace BluezQt
{

class ObexSession;

#if KF5BLUEZQT_BLUEZ_VERSION >= 5
typedef org::bluez::obex::Session1 BluezSession;
#endif

class ObexSessionPrivate : public QObject
{
    Q_OBJECT

public:
    explicit ObexSessionPrivate(const QString &path, const QVariantMap &properties);

    void init(const QVariantMap &properties);

    QWeakPointer<ObexSession> q;
#if KF5BLUEZQT_BLUEZ_VERSION >= 5
    BluezSession *m_bluezSession;
#endif

    QString m_source;
    QString m_destination;
    quint8 m_channel;
    QString m_target;
    QString m_root;

#if KF5BLUEZQT_BLUEZ_VERSION < 5
    ObexSessionBluez4 m_bluez4;
#endif
};

} // namespace BluezQt

#endif // BLUEZQT_OBEXSESSION_P_H
