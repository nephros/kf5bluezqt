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

#include "manager_bluez4_p.h"
#include "adapter_bluez4_p.h"
#include "agentadaptor_bluez4_p.h"
#include "proxyagent_p.h"

#include "manager.h"
#include "manager_p.h"
#include "device_p.h"
#include "adapter.h"
#include "adapter_p.h"
#include "utils.h"
#include "debug.h"

namespace BluezQt
{

ManagerBluez4::ManagerBluez4(ManagerPrivate *parent)
    : QObject(parent)
    , m_managerPrivate(parent)
    , m_bluez4Manager(0)
    , m_loaded(false)
{
    m_bluez4Manager = new Bluez4Manager(Strings::orgBluez(), QStringLiteral("/"), DBusConnection::orgBluez(), this);
}

void ManagerBluez4::load()
{
    if (m_loaded) {
        return;
    }

    connect(m_bluez4Manager, &Bluez4Manager::DefaultAdapterChanged,
            this, &ManagerBluez4::managerDefaultAdapterChanged);
    connect(m_bluez4Manager, &Bluez4Manager::AdapterAdded,
            this, &ManagerBluez4::managerAdapterAdded);
    connect(m_bluez4Manager, &Bluez4Manager::AdapterRemoved,
            this, &ManagerBluez4::managerAdapterRemoved);

    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(m_bluez4Manager->DefaultAdapter(), this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, &ManagerBluez4::managerDefaultAdapterFinished);
}

void ManagerBluez4::addAdapter(AdapterPrivate *adapterPrivate)
{
    if (adapterPrivate->m_bluez4) {
        connect(adapterPrivate->m_bluez4->m_bluez4Adapter, &Bluez4Adapter::DeviceFound,
                this, &ManagerBluez4::deviceFound);
        connect(adapterPrivate->m_bluez4->m_bluez4Adapter, &Bluez4Adapter::DeviceCreated,
                this, &ManagerBluez4::addDeviceByPath);
        connect(adapterPrivate->m_bluez4->m_bluez4Adapter, &Bluez4Adapter::DeviceRemoved,
                this, &ManagerBluez4::deviceRemoved);
        connect(adapterPrivate->m_bluez4->m_bluez4Adapter, &Bluez4Adapter::PropertyChanged,
                this, &ManagerBluez4::adapterPropertyChanged);
        connect(adapterPrivate->m_bluez4, &AdapterBluez4::agentCreated,
                this, &ManagerBluez4::agentCreated);
    }
}

void ManagerBluez4::adapterRemoved(const AdapterPrivate *adapterPrivate)
{
    m_defaultAgents.remove(adapterPrivate->q.data()->ubi());
}

QDBusPendingReply<void> ManagerBluez4::requestDefaultAgent(AdapterPtr adapter, Agent *agent)
{
    if (adapter.isNull() || !adapter->d->m_bluez4 || !agent) {
        return QDBusMessage::createError(QDBusError::InvalidArgs, "Invalid adapter or agent");
    }

    // Register a proxy for this agent instead of registering it directly. This allows the same
    // agent to be used as the default agent as well as the agent for initiated pairing requests,
    // which is not supported behavior in BlueZ 4.
    ProxyAgent *proxyAgent = adapter->d->m_bluez4->createProxyForAgent(agent, "/bluez4_system_agent_proxy");
    QDBusPendingReply<void> reply = adapter->d->m_bluez4->m_bluez4Adapter->RegisterAgent(proxyAgent->objectPath(), ProxyAgent::capabilityToString(proxyAgent->capability()));

    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply, this);
    watcher->setProperty("proxyAgent", QVariant::fromValue(proxyAgent));
    watcher->setProperty("adapterAddress", adapter->address());
    connect(watcher, &QDBusPendingCallWatcher::finished, this, &ManagerBluez4::requestDefaultAgentFinished);

    return reply;
}

void ManagerBluez4::requestDefaultAgentFinished(QDBusPendingCallWatcher *watcher)
{
    const QDBusPendingReply<void> &reply = *watcher;
    ProxyAgent *proxyAgent = watcher->property("proxyAgent").value<ProxyAgent *>();
    QString adapterAddress = watcher->property("adapterAddress").toString();
    watcher->deleteLater();

    if (reply.isError()) {
        return;
    }

    AdapterPtr adapter = m_managerPrivate->q->adapterForAddress(adapterAddress);
    if (adapter && adapter->d->m_bluez4) {
        m_defaultAgents.insert(adapter->ubi(), proxyAgent);

        // Set the adapter to reuse the proxied agent when initiating pairings.
        adapter->d->m_bluez4->setPairingAgent(proxyAgent->agent());
    }
}

QDBusPendingReply<void> ManagerBluez4::unregisterDefaultAgent(AdapterPtr adapter)
{
    if (adapter.isNull() || !adapter->d->m_bluez4) {
        return QDBusMessage::createError(QDBusError::InvalidArgs, "Invalid adapter");
    }

    Q_ASSERT(adapter);

    if (!m_defaultAgents.contains(adapter->ubi())) {
        return QDBusPendingReply<void>(QDBusMessage::createError(QDBusError::InternalError,
                QStringLiteral("Agent has not been registered with this adapter")));
    }

    ProxyAgent *proxyAgent = m_defaultAgents.value(adapter->ubi());
    QDBusPendingReply<void> reply = adapter->d->m_bluez4->m_bluez4Adapter->UnregisterAgent(proxyAgent->objectPath());

    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply, this);
    watcher->setProperty("adapterPath", adapter->ubi());
    connect(watcher, &QDBusPendingCallWatcher::finished, this, &ManagerBluez4::unregisterAgentFinished);

    return reply;
}

void ManagerBluez4::unregisterAgentFinished(QDBusPendingCallWatcher *watcher)
{
    const QDBusPendingReply<void> &reply = *watcher;
    QString adapterPath = watcher->property("adapterPath").toString();
    watcher->deleteLater();

    if (reply.isError()) {
        return;
    }

    m_defaultAgents.remove(adapterPath);
}

AdapterPtr ManagerBluez4::findAdapterForDefaultAgent(Agent *agent)
{
    QHash<QString, ProxyAgent *>::const_iterator i;

    for (i = m_defaultAgents.constBegin(); i != m_defaultAgents.constEnd(); ++i) {
        const QString &adapterPath = i.key();
        ProxyAgent *proxyAgent = i.value();

        if (proxyAgent == agent) {
            return m_managerPrivate->q->adapterForUbi(adapterPath);
        }
    }

    return AdapterPtr();
}

QDBusPendingReply<QDBusObjectPath> ManagerBluez4::createPairedDevice(AdapterPtr adapter, const QString &address)
{
    if (adapter.isNull() || !adapter->d->m_bluez4) {
        return QDBusMessage::createError(QDBusError::InvalidArgs, "Invalid adapter");
    }

    return adapter->d->m_bluez4->createPairedDevice(address);
}

void ManagerBluez4::emitLoaded(bool success, const QString &errorMessage)
{
    if (!m_loaded) {
        m_loaded = true;
        Q_EMIT loaded(success, errorMessage);
    }
}

void ManagerBluez4::managerDefaultAdapterFinished(QDBusPendingCallWatcher *watcher)
{
    const QDBusPendingReply<QDBusObjectPath> &reply = *watcher;
    watcher->deleteLater();

    if (reply.isError()) {
        emitLoaded(false, reply.error().message());
        return;
    }

    const QDBusObjectPath &objectPath = reply.value();
    managerAdapterAdded(objectPath);
}

void ManagerBluez4::managerDefaultAdapterChanged(const QDBusObjectPath &objectPath)
{
    QDBusInterface *bluez4Adapter = new QDBusInterface(Strings::orgBluez(), objectPath.path(),
            QStringLiteral("org.bluez.Adapter"), DBusConnection::orgBluez(), this);

    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(bluez4Adapter->asyncCall(QStringLiteral("GetProperties")), this);
    watcher->setProperty("bluez4Adapter", QVariant::fromValue(bluez4Adapter));
    connect(watcher, &QDBusPendingCallWatcher::finished, this, &ManagerBluez4::adapterGetPropertiesFinished);
}

void ManagerBluez4::managerAdapterAdded(const QDBusObjectPath &objectPath)
{
    QDBusInterface *bluez4Adapter = new QDBusInterface(Strings::orgBluez(), objectPath.path(),
            QStringLiteral("org.bluez.Adapter"), DBusConnection::orgBluez(), this);

    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(bluez4Adapter->asyncCall(QStringLiteral("GetProperties")), this);
    watcher->setProperty("bluez4Adapter", QVariant::fromValue(bluez4Adapter));
    connect(watcher, &QDBusPendingCallWatcher::finished, this, &ManagerBluez4::adapterGetPropertiesFinished);
}

void ManagerBluez4::managerAdapterRemoved(const QDBusObjectPath &objectPath)
{
    m_managerPrivate->removeAdapter(objectPath.path());
}

void ManagerBluez4::adapterGetPropertiesFinished(QDBusPendingCallWatcher *watcher)
{
    const QDBusPendingReply<QVariantMap> &reply = *watcher;
    const QVariantMap &properties = reply.value();
    watcher->deleteLater();

    QDBusInterface *bluez4Adapter = watcher->property("bluez4Adapter").value<QDBusInterface *>();
    bluez4Adapter->deleteLater();

    if (reply.isError()) {
        if (m_pendingInitializationWatchers.isEmpty()) {
            emitLoaded(false, reply.error().message());
        }
        return;
    }

    m_managerPrivate->addAdapter(bluez4Adapter->path(), properties);

    QVariant deviceList = properties.value(QStringLiteral("Devices"));
    if (deviceList.isValid()) {
        updateDeviceList(bluez4Adapter->path(), deviceList);
    }

    if (m_pendingInitializationWatchers.isEmpty()) {
        emitLoaded(true);
    }
}

void ManagerBluez4::adapterPropertyChanged(const QString &property, const QDBusVariant &value)
{
    Bluez4Adapter *adapter = qobject_cast<Bluez4Adapter *>(sender());

    if (adapter && property == QStringLiteral("Devices")) {
        updateDeviceList(adapter->path(), value.variant());
    }
}

void ManagerBluez4::updateDeviceList(const QString &adapterPath, const QVariant &deviceList)
{
    QStringList devicePaths;
    const QDBusArgument &dbusArgument = deviceList.value<QDBusArgument>();
    dbusArgument.beginArray();
    while (!dbusArgument.atEnd()) {
        QDBusObjectPath path;
        dbusArgument >> path;
        devicePaths << path.path();
    }
    dbusArgument.endArray();

    QHash<QString, DevicePtr>::const_iterator it;
    QStringList pathsToRemove;

    for (it = m_managerPrivate->m_devices.constBegin(); it != m_managerPrivate->m_devices.constEnd(); ++it) {
        const QString &path = it.key();
        const DevicePtr &device = it.value();

        if (device->adapter()->ubi() == adapterPath && !devicePaths.contains(path)) {
            pathsToRemove.append(path);
        }
    }

    Q_FOREACH (const QString &devicePath, devicePaths) {
        if (!m_managerPrivate->m_devices.contains(devicePath)) {
            QDBusPendingCallWatcher *watcher = addDeviceByPath(QDBusObjectPath(devicePath));
            if (!m_loaded && watcher) {
                m_pendingInitializationWatchers.insert(watcher);
            }
        }
    }

    Q_FOREACH (const QString &devicePath, pathsToRemove) {
        m_managerPrivate->removeDevice(devicePath);
    }
}

QDBusPendingCallWatcher *ManagerBluez4::addDeviceByPath(const QDBusObjectPath &objectPath)
{
    if (!m_managerPrivate->m_devices.contains(objectPath.path())) {
        QDBusInterface *bluezDevice = new QDBusInterface(Strings::orgBluez(), objectPath.path(),
                QStringLiteral("org.bluez.Device"), DBusConnection::orgBluez(), this);

        QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(
                    bluezDevice->asyncCall(QStringLiteral("GetProperties")), this);
        watcher->setProperty("bluezDevice", QVariant::fromValue(bluezDevice));
        connect(watcher, &QDBusPendingCallWatcher::finished,
                this, &ManagerBluez4::deviceGetPropertiesFinished);
        return watcher;
    }
    return 0;
}

void ManagerBluez4::deviceGetPropertiesFinished(QDBusPendingCallWatcher *watcher)
{
    m_pendingInitializationWatchers.remove(watcher);

    const QDBusPendingReply<QVariantMap> &reply = *watcher;
    watcher->deleteLater();

    QDBusInterface *bluezDevice = watcher->property("bluezDevice").value<QDBusInterface *>();
    bluezDevice->deleteLater();

    if (!reply.isError()) {
        DevicePtr device = m_managerPrivate->m_devices.value(bluezDevice->path());
        if (device.isNull()) {
            m_managerPrivate->addDevice(bluezDevice->path(), reply.value());
        } else {
            device->d->propertiesChanged(Strings::orgBluezDevice1(), reply.value(), QStringList());
        }
    }

    if (m_pendingInitializationWatchers.isEmpty()) {
        emitLoaded(true);
    }
}

void ManagerBluez4::deviceFound(const QString &address, const QVariantMap &values)
{
    Bluez4Adapter *bluez4Adapter = qobject_cast<Bluez4Adapter *>(sender());
    if (!bluez4Adapter) {
        return;
    }

    QHash<QString, DevicePtr>::iterator it;

    for (it = m_managerPrivate->m_devices.begin(); it != m_managerPrivate->m_devices.end(); ++it) {
        const DevicePtr &device = it.value();

        if (device->address() == address) {
            device->d->propertiesChanged(Strings::orgBluezDevice1(), values, QStringList());
            return;
        }
    }

    // 'Adapter' is not included in the property map provided by DeviceFound(), but addDevice()
    // expects this property so it must be present.
    QString addressCopy = address;
    QString devicePath = QString("%1/dev_%2").arg(bluez4Adapter->path()).arg(addressCopy.replace(':', '_'));
    QVariantMap properties = values;
    properties.insert(QStringLiteral("Adapter"), QVariant::fromValue(QDBusObjectPath(bluez4Adapter->path())));

    m_managerPrivate->addDevice(devicePath, properties);
}

void ManagerBluez4::deviceRemoved(const QDBusObjectPath &objectPath)
{
    m_managerPrivate->removeDevice(objectPath.path());
}

void ManagerBluez4::agentCreated(Agent *agent)
{
    // Initialize the agent as per Manager::registerAgent()

    new AgentAdaptorBluez4(agent, m_managerPrivate->q);

    if (!DBusConnection::orgBluez().registerObject(agent->objectPath().path(), agent)) {
        qCWarning(BLUEZQT) << "Cannot register object" << agent->objectPath().path();
    }
}

} // namespace BluezQt
