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
