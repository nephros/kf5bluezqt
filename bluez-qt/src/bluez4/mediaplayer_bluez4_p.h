/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
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

#ifndef BLUEZQT_MEDIAPLAYER_BLUEZ4_P_H
#define BLUEZQT_MEDIAPLAYER_BLUEZ4_P_H

#include <QObject>

#include "bluez4mediaplayer.h"

namespace BluezQt
{

typedef org::bluez::MediaPlayer Bluez4MediaPlayer;

class MediaPlayerPrivate;

class MediaPlayerBluez4 : public QObject
{
    Q_OBJECT

public:
    explicit MediaPlayerBluez4(MediaPlayerPrivate *mediaPlayerPrivate, const QString &path);

    void mediaPlayerPropertyChanged(const QString &property, const QDBusVariant &value);

    MediaPlayerPrivate *m_mediaPlayerPrivate;
    Bluez4MediaPlayer *m_bluez4MediaPlayer;
};

} // namespace BluezQt

#endif // BLUEZQT_MEDIAPLAYER_BLUEZ4_P_H
