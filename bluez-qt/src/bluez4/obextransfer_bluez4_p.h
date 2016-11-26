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

#ifndef BLUEZQT_OBEXTRANSFER_BLUEZ4_P_H
#define BLUEZQT_OBEXTRANSFER_BLUEZ4_P_H

#include "bluez4obextransfer.h"

namespace BluezQt
{

typedef org::bluez::obex::Transfer Bluez4Transfer;

class ObexTransferPrivate;

class ObexTransferBluez4 : public QObject
{
    Q_OBJECT

public:
    explicit ObexTransferBluez4(ObexTransferPrivate *obexTransferPrivate, const QString &path, const QVariantMap &properties);

    void init(const QVariantMap &properties);
    void setTransferProperty(const QString &property, const QVariant &value);

    ObexTransferPrivate *m_obexTransferPrivate;
    Bluez4Transfer *m_bluez4Transfer;
    QVariantMap m_properties;

private:
    // from org.bluez.obex service
    void orgBluezObexTransferProgress(qint32 total, qint32 transferred);

    // from org.bluez.obex.client service
    void orgBluezObexClientTransferPropertyChanged(const QString &property, const QDBusVariant &value);
    void orgBluezObexClientTransferComplete();
    void orgBluezObexClientTransferError(const QString &code, const QString &message);
};

} // namespace BluezQt

#endif // BLUEZQT_OBEXTRANSFER_BLUEZ4_P_H
