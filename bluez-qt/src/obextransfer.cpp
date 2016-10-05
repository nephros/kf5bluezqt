/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * Copyright (C) 2014 David Rosca <nowrep@gmail.com>
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

#include "obextransfer.h"
#include "obextransfer_p.h"
#include "pendingcall.h"
#include "obexmanager.h"
#include "obexsession.h"
#include "utils.h"
#include "macros.h"

#if KF5BLUEZQT_BLUEZ_VERSION < 5
#include <QMimeDatabase>
#endif

namespace BluezQt
{

static ObexTransfer::Status stringToStatus(const QString &status)
{
    if (status == QLatin1String("queued")) {
        return ObexTransfer::Queued;
    } else if (status == QLatin1String("active")) {
        return ObexTransfer::Active;
    } else if (status == QLatin1String("suspended")) {
        return ObexTransfer::Suspended;
    } else if (status == QLatin1String("complete")) {
        return ObexTransfer::Complete;
    } else if (status == QLatin1String("error")) {
        return ObexTransfer::Error;
    }
    return ObexTransfer::Unknown;
}

ObexTransferPrivate::ObexTransferPrivate(const QString &path, const QVariantMap &properties)
    : QObject()
#if KF5BLUEZQT_BLUEZ_VERSION >= 5
    , m_dbusProperties(0)
#else
    , m_clientOrigin(path.contains(QStringLiteral("session")))
#endif
    , m_status(ObexTransfer::Error)
    , m_time(0)
    , m_size(0)
    , m_transferred(0)
    , m_suspendable(false)
{
#if KF5BLUEZQT_BLUEZ_VERSION >= 5
    m_bluezTransfer = new BluezTransfer(Strings::orgBluezObex(), path, DBusConnection::orgBluezObex(), this);
#else
    const QString &service = m_clientOrigin ? QStringLiteral("org.bluez.obex.client") : Strings::orgBluezObex();
    m_bluezTransfer = new BluezTransfer(service, path, DBusConnection::orgBluezObex(), this);
#endif

    if (Instance::obexManager()) {
        connect(Instance::obexManager(), &ObexManager::sessionRemoved, this, &ObexTransferPrivate::sessionRemoved);
    }

#if KF5BLUEZQT_BLUEZ_VERSION >= 5
    init(properties);
#else
    // Ensure the properties contain the non-optional Status and Type properties, which are not
    // specified for transfers provided by the org.bluez.obex.client service in BlueZ 4. Also,
    // 'Progress' has been renamed to 'Transferred'.
    // This doesn't need to be done for transfers provided by the org.bluez.obex service (i.e. from
    // OBEX agents) as ObexAgentAdaptor already adds these in the AuthorizePush() handler.
    QVariantMap modifiedProperties = properties;
    if (m_clientOrigin) {
        static const QMimeDatabase mimeDatabase;
        const QString &name = properties.value(QStringLiteral("name")).toString();
        const QString &type = mimeDatabase.mimeTypeForFile(name).name();

        modifiedProperties.insert(QStringLiteral("Status"), QStringLiteral("queued"));
        modifiedProperties.insert(QStringLiteral("Type"), type);
        modifiedProperties.insert(QStringLiteral("Transferred"), properties.value(QStringLiteral("Progress")).toUInt());
    }
    init(modifiedProperties);
    m_properties = properties;
#endif
}

void ObexTransferPrivate::init(const QVariantMap &properties)
{
#if KF5BLUEZQT_BLUEZ_VERSION >= 5
    m_dbusProperties = new DBusProperties(Strings::orgBluezObex(), m_bluezTransfer->path(),
                                          DBusConnection::orgBluezObex(), this);

    connect(m_dbusProperties, &DBusProperties::PropertiesChanged,
            this, &ObexTransferPrivate::propertiesChanged, Qt::QueuedConnection);
#else
    if (m_clientOrigin) {
        connect(m_bluezTransfer, &BluezTransfer::PropertyChanged,
                this, &ObexTransferPrivate::orgBluezObexClientTransferPropertyChanged, Qt::QueuedConnection);
        connect(m_bluezTransfer, &BluezTransfer::Complete,
                this, &ObexTransferPrivate::orgBluezObexClientTransferComplete, Qt::QueuedConnection);
        connect(m_bluezTransfer, &BluezTransfer::Error,
                this, &ObexTransferPrivate::orgBluezObexClientTransferError, Qt::QueuedConnection);
    } else {
        connect(m_bluezTransfer, &BluezTransfer::Progress,
                this, &ObexTransferPrivate::orgBluezObexTransferProgress, Qt::QueuedConnection);
    }
#endif

    // Init properties
    m_status = stringToStatus(properties.value(QStringLiteral("Status")).toString());
    m_name = properties.value(QStringLiteral("Name")).toString();
    m_type = properties.value(QStringLiteral("Type")).toString();
    m_time = properties.value(QStringLiteral("Time")).toUInt();
    m_size = properties.value(QStringLiteral("Size")).toUInt();
    m_transferred = properties.value(QStringLiteral("Transferred")).toUInt();
    m_fileName = properties.value(QStringLiteral("Filename")).toString();
}

void ObexTransferPrivate::propertiesChanged(const QString &interface, const QVariantMap &changed, const QStringList &invalidated)
{
    Q_UNUSED(invalidated)

#if KF5BLUEZQT_BLUEZ_VERSION >= 5
    if (interface != Strings::orgBluezObexTransfer1()) {
        return;
    }
#else
    Q_UNUSED(interface)
#endif

    QVariantMap::const_iterator i;
    for (i = changed.constBegin(); i != changed.constEnd(); ++i) {
        const QVariant &value = i.value();
        const QString &property = i.key();

        if (property == QLatin1String("Status")) {
            PROPERTY_CHANGED2(m_status, stringToStatus(value.toString()), statusChanged);
        } else if (property == QLatin1String("Transferred")) {
            PROPERTY_CHANGED(m_transferred, toUInt, transferredChanged);
        } else if (property == QLatin1String("Filename")) {
            PROPERTY_CHANGED(m_fileName, toString, fileNameChanged);
        }

#if KF5BLUEZQT_BLUEZ_VERSION < 5
        // 'Transferred' was 'Progress' in BlueZ 4
        if (property == QLatin1String("Progress")) {
            PROPERTY_CHANGED(m_transferred, toUInt, transferredChanged);
        }
#endif
    }
}

void ObexTransferPrivate::sessionRemoved(const ObexSessionPtr &session)
{
    if (!m_bluezTransfer->path().startsWith(session->objectPath().path())) {
        return;
    }

    // Change status to Error if org.bluez.obex crashes
    if (m_status != ObexTransfer::Complete && m_status != ObexTransfer::Error) {
        m_status = ObexTransfer::Error;
        Q_EMIT q.data()->statusChanged(m_status);
    }
}

#if KF5BLUEZQT_BLUEZ_VERSION < 5
void ObexTransferPrivate::orgBluezObexTransferProgress(qint32 total, qint32 transferred)
{
    Q_UNUSED(total)

    QVariant value = static_cast<quint64>(transferred);
    INVOKE_PROPERTIES_CHANGED(QStringLiteral("org.bluez.Transfer"), this, QStringLiteral("Progress"), value);
}

void ObexTransferPrivate::orgBluezObexClientTransferComplete()
{
    QVariant value = QStringLiteral("complete");
    PROPERTY_CHANGED2(m_status, stringToStatus(value.toString()), statusChanged);
}

void ObexTransferPrivate::orgBluezObexClientTransferError(const QString &code, const QString &message)
{
    QVariant value = QStringLiteral("error");
    PROPERTY_CHANGED2(m_status, stringToStatus(value.toString()), statusChanged);
}

void ObexTransferPrivate::orgBluezObexClientTransferPropertyChanged(const QString &property, const QDBusVariant &value)
{
    INVOKE_PROPERTIES_CHANGED(QStringLiteral("org.bluez.obex.Transfer"), this, property, value.variant());
}
#endif

ObexTransfer::ObexTransfer(const QString &path, const QVariantMap &properties)
    : QObject()
    , d(new ObexTransferPrivate(path, properties))
{
}

ObexTransfer::~ObexTransfer()
{
    delete d;
}

ObexTransferPtr ObexTransfer::toSharedPtr() const
{
    return d->q.toStrongRef();
}

QDBusObjectPath ObexTransfer::objectPath() const
{
    return QDBusObjectPath(d->m_bluezTransfer->path());
}

ObexTransfer::Status ObexTransfer::status() const
{
    return d->m_status;
}

QString ObexTransfer::name() const
{
    return d->m_name;
}

QString ObexTransfer::type() const
{
    return d->m_type;
}

quint64 ObexTransfer::time() const
{
    return d->m_time;
}

quint64 ObexTransfer::size() const
{
    return d->m_size;
}

quint64 ObexTransfer::transferred() const
{
    return d->m_transferred;
}

QString ObexTransfer::fileName() const
{
    return d->m_fileName;
}

bool ObexTransfer::isSuspendable() const
{
#if KF5BLUEZQT_BLUEZ_VERSION >= 5
    return d->m_suspendable;
#else
    return false;
#endif
}

PendingCall *ObexTransfer::cancel()
{
    return new PendingCall(d->m_bluezTransfer->Cancel(), PendingCall::ReturnVoid, this);
}

PendingCall *ObexTransfer::suspend()
{
#if KF5BLUEZQT_BLUEZ_VERSION >= 5
    return new PendingCall(d->m_bluezTransfer->Suspend(), PendingCall::ReturnVoid, this);
#else
    return new PendingCall(PendingCall::NotSupported, QStringLiteral("ObexTransfer::suspend() not available in BlueZ 4!"), this);
#endif
}

PendingCall *ObexTransfer::resume()
{
#if KF5BLUEZQT_BLUEZ_VERSION >= 5
    return new PendingCall(d->m_bluezTransfer->Resume(), PendingCall::ReturnVoid, this);
#else
    return new PendingCall(PendingCall::NotSupported, QStringLiteral("ObexTransfer::resume() not available in BlueZ 4!"), this);
#endif
}

#if KF5BLUEZQT_BLUEZ_VERSION < 5
void ObexTransfer::setTransferProperty(const QString &property, const QVariant &value)
{
    INVOKE_PROPERTIES_CHANGED(QStringLiteral("org.bluez.Transfer"), d, property, value);
}
#endif

} // namespace BluezQt
