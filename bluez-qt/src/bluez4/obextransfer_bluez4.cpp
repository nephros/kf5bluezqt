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

#include "obextransfer_bluez4_p.h"
#include "macros_bluez4_p.h"

#include "obextransfer_p.h"
#include "utils.h"
#include "macros.h"

#include <QMimeDatabase>

namespace BluezQt
{

ObexTransferBluez4::ObexTransferBluez4(ObexTransferPrivate *obexTransferPrivate, const QString &path, const QVariantMap &properties)
    : QObject(obexTransferPrivate)
    , m_obexTransferPrivate(obexTransferPrivate)
{
    const bool clientOrigin = path.contains(QStringLiteral("session"));
    const QString &service = clientOrigin ? QStringLiteral("org.bluez.obex.client") : Strings::orgBluezObex();
    m_bluez4Transfer = new Bluez4Transfer(service, path, DBusConnection::orgBluezObex(), this);

    // Connect to the transfer signals depending on whether this is a client service.
    if (clientOrigin) {
        connect(m_bluez4Transfer, &Bluez4Transfer::PropertyChanged,
                this, &ObexTransferBluez4::orgBluezObexClientTransferPropertyChanged, Qt::QueuedConnection);
        connect(m_bluez4Transfer, &Bluez4Transfer::Complete,
                this, &ObexTransferBluez4::orgBluezObexClientTransferComplete, Qt::QueuedConnection);
        connect(m_bluez4Transfer, &Bluez4Transfer::Error,
                this, &ObexTransferBluez4::orgBluezObexClientTransferError, Qt::QueuedConnection);
    } else {
        connect(m_bluez4Transfer, &Bluez4Transfer::Progress,
                this, &ObexTransferBluez4::orgBluezObexTransferProgress, Qt::QueuedConnection);
    }

    // Ensure the properties contain the non-optional Status and Type properties, which are not
    // specified for transfers provided by the org.bluez.obex.client service in BlueZ 4. Also,
    // 'Progress' has been renamed to 'Transferred'.
    // This doesn't need to be done for transfers provided by the org.bluez.obex service (i.e. from
    // OBEX agents) as ObexAgentAdaptor already adds these in the AuthorizePush() handler.
    QVariantMap modifiedProperties = properties;
    if (clientOrigin) {
        static const QMimeDatabase mimeDatabase;
        const QString &name = properties.value(QStringLiteral("name")).toString();
        const QString &type = mimeDatabase.mimeTypeForFile(name).name();

        modifiedProperties.insert(QStringLiteral("Status"), QStringLiteral("queued"));
        modifiedProperties.insert(QStringLiteral("Type"), type);
        modifiedProperties.insert(QStringLiteral("Transferred"), properties.value(QStringLiteral("Progress")).toUInt());
    }

    obexTransferPrivate->init(modifiedProperties);
    m_properties = properties;
}

void ObexTransferBluez4::setTransferProperty(const QString &property, const QVariant &value)
{
    INVOKE_PROPERTIES_CHANGED(Strings::orgBluezObexTransfer1(), m_obexTransferPrivate, property, value);
}

void ObexTransferBluez4::orgBluezObexTransferProgress(qint32 total, qint32 transferred)
{
    Q_UNUSED(total)

    QVariant value = static_cast<quint64>(transferred);
    INVOKE_PROPERTIES_CHANGED(Strings::orgBluezObexTransfer1(), m_obexTransferPrivate, QStringLiteral("Progress"), value);
}

void ObexTransferBluez4::orgBluezObexClientTransferComplete()
{
    m_obexTransferPrivate->m_status = ObexTransfer::Complete;
    Q_EMIT m_obexTransferPrivate->q.data()->statusChanged(m_obexTransferPrivate->m_status);
}

void ObexTransferBluez4::orgBluezObexClientTransferError(const QString &code, const QString &message)
{
    Q_UNUSED(code)
    Q_UNUSED(message)

    m_obexTransferPrivate->m_status = ObexTransfer::Error;
    Q_EMIT m_obexTransferPrivate->q.data()->statusChanged(m_obexTransferPrivate->m_status);
}

void ObexTransferBluez4::orgBluezObexClientTransferPropertyChanged(const QString &property, const QDBusVariant &value)
{
    if (property == QStringLiteral("Progress")) {
        // 'Transferred' was 'Progress' in BlueZ 4
        INVOKE_PROPERTIES_CHANGED(Strings::orgBluezObexTransfer1(), m_obexTransferPrivate, QStringLiteral("Transferred"), value.variant());
    } else {
        INVOKE_PROPERTIES_CHANGED(Strings::orgBluezObexTransfer1(), m_obexTransferPrivate, property, value.variant());
    }
}

} // namespace BluezQt
