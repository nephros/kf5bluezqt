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

#include "manager_p.h"
#include "manager.h"
#include "device.h"
#include "device_p.h"
#include "adapter.h"
#include "adapter_p.h"
#include "debug.h"
#include "utils.h"

#include <QDBusReply>
#include <QDBusConnection>
#include <QDBusServiceWatcher>

#if KF5BLUEZQT_BLUEZ_VERSION < 5
#include "bluezadapter.h"
#include "agentadaptor.h"
#endif

namespace BluezQt
{

ManagerPrivate::ManagerPrivate(Manager *parent)
    : QObject(parent)
    , q(parent)
#if KF5BLUEZQT_BLUEZ_VERSION >= 5
    , m_dbusObjectManager(0)
    , m_bluezAgentManager(0)
    , m_bluezProfileManager(0)
#else
    , m_bluezManager(0)
#endif
    , m_usableAdapter(0)
    , m_initialized(false)
    , m_bluezRunning(false)
    , m_loaded(false)
    , m_adaptersLoaded(false)
{
    qDBusRegisterMetaType<DBusManagerStruct>();
    qDBusRegisterMetaType<QVariantMapMap>();

    m_rfkill = new Rfkill(this);
    m_bluetoothBlocked = rfkillBlocked();
    connect(m_rfkill, &Rfkill::stateChanged, this, &ManagerPrivate::rfkillStateChanged);

    connect(q, &Manager::adapterRemoved, this, &ManagerPrivate::adapterRemoved);
}

void ManagerPrivate::init()
{
    // Keep an eye on org.bluez service
    QDBusServiceWatcher *serviceWatcher = new QDBusServiceWatcher(Strings::orgBluez(), DBusConnection::orgBluez(),
            QDBusServiceWatcher::WatchForRegistration | QDBusServiceWatcher::WatchForUnregistration, this);

    connect(serviceWatcher, &QDBusServiceWatcher::serviceRegistered, this, &ManagerPrivate::serviceRegistered);
    connect(serviceWatcher, &QDBusServiceWatcher::serviceUnregistered, this, &ManagerPrivate::serviceUnregistered);

    // Update the current state of org.bluez service
    if (!DBusConnection::orgBluez().isConnected()) {
        Q_EMIT initError(QStringLiteral("DBus system bus is not connected!"));
        return;
    }

    QDBusMessage call = QDBusMessage::createMethodCall(Strings::orgFreedesktopDBus(),
                        QStringLiteral("/"),
                        Strings::orgFreedesktopDBus(),
                        QStringLiteral("NameHasOwner"));

    call << Strings::orgBluez();

    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(DBusConnection::orgBluez().asyncCall(call));
    connect(watcher, &QDBusPendingCallWatcher::finished, this, &ManagerPrivate::nameHasOwnerFinished);
}

void ManagerPrivate::nameHasOwnerFinished(QDBusPendingCallWatcher *watcher)
{
    const QDBusPendingReply<bool> &reply = *watcher;
    watcher->deleteLater();

    if (reply.isError()) {
        Q_EMIT initError(reply.error().message());
        return;
    }

    m_bluezRunning = reply.value();

    if (m_bluezRunning) {
        load();
    } else {
        m_initialized = true;
        Q_EMIT initFinished();
    }
}

void ManagerPrivate::load()
{
    if (!m_bluezRunning || m_loaded) {
        return;
    }

    // Force QDBus to cache owner of org.bluez - this will be the only blocking call on system connection
    DBusConnection::orgBluez().connect(Strings::orgBluez(),
                                       QStringLiteral("/"),
                                       Strings::orgFreedesktopDBus(),
                                       QStringLiteral("Dummy"),
                                       this,
                                       SLOT(dummy()));

#if KF5BLUEZQT_BLUEZ_VERSION >= 5
    m_dbusObjectManager = new DBusObjectManager(Strings::orgBluez(), QStringLiteral("/"),
            DBusConnection::orgBluez(), this);

    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(m_dbusObjectManager->GetManagedObjects(), this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, &ManagerPrivate::getManagedObjectsFinished);
#else
    m_bluezManager = new BluezManager(Strings::orgBluez(), QStringLiteral("/"), DBusConnection::orgBluez(), this);

    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(m_bluezManager->DefaultAdapter(), this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, &ManagerPrivate::managerDefaultAdapterFinished);

#endif
}

#if KF5BLUEZQT_BLUEZ_VERSION >= 5
void ManagerPrivate::getManagedObjectsFinished(QDBusPendingCallWatcher *watcher)
{
    const QDBusPendingReply<DBusManagerStruct> &reply = *watcher;
    watcher->deleteLater();

    if (reply.isError()) {
        Q_EMIT initError(reply.error().message());
        return;
    }

    DBusManagerStruct::const_iterator it;
    const DBusManagerStruct &managedObjects = reply.value();

    for (it = managedObjects.constBegin(); it != managedObjects.constEnd(); ++it) {
        const QString &path = it.key().path();
        const QVariantMapMap &interfaces = it.value();

        interfacesAdded(it.key(), interfaces);

        if (interfaces.contains(Strings::orgBluezAgentManager1())) {
            m_bluezAgentManager = new BluezAgentManager(Strings::orgBluez(), path, DBusConnection::orgBluez(), this);
        }
        if (interfaces.contains(Strings::orgBluezProfileManager1())) {
            m_bluezProfileManager = new BluezProfileManager(Strings::orgBluez(), path, DBusConnection::orgBluez(), this);
        }
    }

    if (!m_bluezAgentManager) {
        Q_EMIT initError(QStringLiteral("Cannot find org.bluez.AgentManager1 object!"));
        return;
    }

    if (!m_bluezProfileManager) {
        Q_EMIT initError(QStringLiteral("Cannot find org.bluez.ProfileManager1 object!"));
        return;
    }

    connect(m_dbusObjectManager, &DBusObjectManager::InterfacesAdded,
            this, &ManagerPrivate::interfacesAdded);
    connect(m_dbusObjectManager, &DBusObjectManager::InterfacesRemoved,
            this, &ManagerPrivate::interfacesRemoved);

    m_loaded = true;
    m_initialized = true;

    Q_EMIT q->operationalChanged(true);

    if (q->isBluetoothOperational()) {
        Q_EMIT q->bluetoothOperationalChanged(true);
    }

    Q_EMIT initFinished();
}
#endif

void ManagerPrivate::clear()
{
    m_loaded = false;

    // Delete all devices first
    while (!m_devices.isEmpty()) {
        DevicePtr device = m_devices.begin().value();
        m_devices.remove(m_devices.begin().key());
        device->adapter()->d->removeDevice(device);
    }

    // Delete all adapters
    while (!m_adapters.isEmpty()) {
        AdapterPtr adapter = m_adapters.begin().value();
        m_adapters.remove(m_adapters.begin().key());
        Q_EMIT adapter->adapterRemoved(adapter);

        if (m_adapters.isEmpty()) {
            Q_EMIT q->allAdaptersRemoved();
        }
    }

    // Delete all other objects
    m_usableAdapter.clear();

#if KF5BLUEZQT_BLUEZ_VERSION >= 5
    if (m_dbusObjectManager) {
        m_dbusObjectManager->deleteLater();
        m_dbusObjectManager = Q_NULLPTR;
    }

    if (m_bluezAgentManager) {
        m_bluezAgentManager->deleteLater();
        m_bluezAgentManager = Q_NULLPTR;
    }
#endif
}

AdapterPtr ManagerPrivate::findUsableAdapter() const
{
    Q_FOREACH (AdapterPtr adapter, m_adapters) {
        if (adapter->isPowered()) {
            return adapter;
        }
    }
    return AdapterPtr();
}

void ManagerPrivate::serviceRegistered()
{
    qCDebug(BLUEZQT) << "BlueZ service registered";
    m_bluezRunning = true;

    load();
}

void ManagerPrivate::serviceUnregistered()
{
    qCDebug(BLUEZQT) << "BlueZ service unregistered";

    bool wasBtOperational = q->isBluetoothOperational();
    m_bluezRunning = false;

    if (wasBtOperational) {
        Q_EMIT q->bluetoothOperationalChanged(false);
    }

    clear();
    Q_EMIT q->operationalChanged(false);
}

#if KF5BLUEZQT_BLUEZ_VERSION >= 5
void ManagerPrivate::interfacesAdded(const QDBusObjectPath &objectPath, const QVariantMapMap &interfaces)
{
    const QString &path = objectPath.path();
    QVariantMapMap::const_iterator it;

    for (it = interfaces.constBegin(); it != interfaces.constEnd(); ++it) {
        if (it.key() == Strings::orgBluezAdapter1()) {
            addAdapter(path, it.value());
        } else if (it.key() == Strings::orgBluezDevice1()) {
            addDevice(path, it.value());
        }
    }

    Q_FOREACH (DevicePtr device, m_devices.values()) {
        if (path.startsWith(device->ubi())) {
            device->d->interfacesAdded(path, interfaces);
            break;
        }
    }
}
#endif

#if KF5BLUEZQT_BLUEZ_VERSION >= 5
void ManagerPrivate::interfacesRemoved(const QDBusObjectPath &objectPath, const QStringList &interfaces)
{
    const QString &path = objectPath.path();

    Q_FOREACH (const QString &interface, interfaces) {
        if (interface == Strings::orgBluezAdapter1()) {
            removeAdapter(path);
        } else if (interface == Strings::orgBluezDevice1()) {
            removeDevice(path);
        }
    }

    Q_FOREACH (DevicePtr device, m_devices.values()) {
        if (path.startsWith(device->ubi())) {
            device->d->interfacesRemoved(path, interfaces);
            break;
        }
    }
}
#endif

void ManagerPrivate::adapterRemoved(const AdapterPtr &adapter)
{
#if KF5BLUEZQT_BLUEZ_VERSION < 5
    m_defaultAgents.remove(adapter->ubi());
#endif

    disconnect(adapter.data(), &Adapter::poweredChanged, this, &ManagerPrivate::adapterPoweredChanged);

    // Current usable adapter was removed
    if (adapter == m_usableAdapter) {
        setUsableAdapter(findUsableAdapter());
    }
}

void ManagerPrivate::adapterPoweredChanged(bool powered)
{
    Q_ASSERT(qobject_cast<Adapter*>(sender()));
    AdapterPtr adapter = static_cast<Adapter*>(sender())->toSharedPtr();

    // Current usable adapter was powered off
    if (m_usableAdapter == adapter && !powered) {
        setUsableAdapter(findUsableAdapter());
    }

    // Adapter was powered on, set it as usable
    if (!m_usableAdapter && powered) {
        setUsableAdapter(adapter);
    }
}

void ManagerPrivate::rfkillStateChanged(Rfkill::State state)
{
    Q_UNUSED(state)

    bool blocked = rfkillBlocked();
    bool wasBtOperational = q->isBluetoothOperational();

    if (m_bluetoothBlocked != blocked) {
        m_bluetoothBlocked = blocked;
        Q_EMIT q->bluetoothBlockedChanged(m_bluetoothBlocked);
        if (wasBtOperational != q->isBluetoothOperational()) {
            Q_EMIT q->bluetoothOperationalChanged(q->isBluetoothOperational());
        }
    }
}

void ManagerPrivate::addAdapter(const QString &adapterPath, const QVariantMap &properties)
{
    AdapterPtr adapter = AdapterPtr(new Adapter(adapterPath, properties));
    adapter->d->q = adapter.toWeakRef();
    m_adapters.insert(adapterPath, adapter);

    Q_EMIT q->adapterAdded(adapter);

    // Powered adapter was added, set it as usable
    if (!m_usableAdapter && adapter->isPowered()) {
        setUsableAdapter(adapter);
    }

    connect(adapter.data(), &Adapter::deviceAdded, q, &Manager::deviceAdded);
    connect(adapter.data(), &Adapter::adapterRemoved, q, &Manager::adapterRemoved);
    connect(adapter.data(), &Adapter::adapterChanged, q, &Manager::adapterChanged);
    connect(adapter.data(), &Adapter::poweredChanged, this, &ManagerPrivate::adapterPoweredChanged);

#if KF5BLUEZQT_BLUEZ_VERSION < 5
    connect(adapter->d->m_bluezAdapter, &BluezAdapter::DeviceFound,
            this, &ManagerPrivate::deviceFound);
    connect(adapter->d->m_bluezAdapter, &BluezAdapter::DeviceCreated,
            this, &ManagerPrivate::addDeviceByPath);
    connect(adapter->d->m_bluezAdapter, &BluezAdapter::DeviceRemoved,
            this, &ManagerPrivate::deviceRemoved);
    connect(adapter->d->m_bluezAdapter, &BluezAdapter::PropertyChanged,
            this, &ManagerPrivate::adapterPropertyChanged);
    connect(adapter.data()->d, &AdapterPrivate::agentCreated,
            this, &ManagerPrivate::agentCreated);
#endif
}

void ManagerPrivate::addDevice(const QString &devicePath, const QVariantMap &properties)
{
    AdapterPtr adapter = m_adapters.value(properties.value(QStringLiteral("Adapter")).value<QDBusObjectPath>().path());
    Q_ASSERT(adapter);
    DevicePtr device = DevicePtr(new Device(devicePath, properties, adapter));
    device->d->q = device.toWeakRef();
    m_devices.insert(devicePath, device);
    adapter->d->addDevice(device);

    connect(device.data(), &Device::deviceRemoved, q, &Manager::deviceRemoved);
    connect(device.data(), &Device::deviceChanged, q, &Manager::deviceChanged);
}

void ManagerPrivate::removeAdapter(const QString &adapterPath)
{
    AdapterPtr adapter = m_adapters.value(adapterPath);
    if (!adapter) {
        return;
    }

    // Make sure we always remove all devices before removing the adapter
    Q_FOREACH (const DevicePtr &device, adapter->devices()) {
        removeDevice(device->ubi());
    }

    m_adapters.remove(adapterPath);
    Q_EMIT adapter->adapterRemoved(adapter);

    if (m_adapters.isEmpty()) {
        Q_EMIT q->allAdaptersRemoved();
    }

    disconnect(adapter.data(), &Adapter::adapterChanged, q, &Manager::adapterChanged);
    disconnect(adapter.data(), &Adapter::poweredChanged, this, &ManagerPrivate::adapterPoweredChanged);
}

void ManagerPrivate::removeDevice(const QString &devicePath)
{
    DevicePtr device = m_devices.take(devicePath);
    if (!device) {
        return;
    }

    device->adapter()->d->removeDevice(device);

    disconnect(device.data(), &Device::deviceChanged, q, &Manager::deviceChanged);
}

bool ManagerPrivate::rfkillBlocked() const
{
    return m_rfkill->state() == Rfkill::SoftBlocked || m_rfkill->state() == Rfkill::HardBlocked;
}

void ManagerPrivate::setUsableAdapter(const AdapterPtr &adapter)
{
    if (m_usableAdapter == adapter) {
        return;
    }

    qCDebug(BLUEZQT) << "Setting usable adapter" << adapter;

    bool wasBtOperational = q->isBluetoothOperational();

    m_usableAdapter = adapter;
    Q_EMIT q->usableAdapterChanged(m_usableAdapter);

    if (wasBtOperational != q->isBluetoothOperational()) {
        Q_EMIT q->bluetoothOperationalChanged(q->isBluetoothOperational());
    }
}

void ManagerPrivate::dummy()
{
}

#if KF5BLUEZQT_BLUEZ_VERSION < 5
void ManagerPrivate::managerInitialized()
{
    if (!m_initialized) {
        // Set up initial state as per getManagedObjectsFinished()
        m_loaded = true;
        m_initialized = true;

        Q_EMIT q->operationalChanged(true);

        if (q->isBluetoothOperational()) {
            Q_EMIT q->bluetoothOperationalChanged(true);
        }

        Q_EMIT initFinished();
    }
}

void ManagerPrivate::managerDefaultAdapterFinished(QDBusPendingCallWatcher *watcher)
{
    const QDBusPendingReply<QDBusObjectPath> &reply = *watcher;
    watcher->deleteLater();

    if (reply.isError()) {
        Q_EMIT initError(reply.error().message());
        return;
    }

    const QDBusObjectPath &objectPath = reply.value();
    managerAdapterAdded(objectPath);

    connect(m_bluezManager, &BluezManager::AdapterAdded,
            this, &ManagerPrivate::managerAdapterAdded);
    connect(m_bluezManager, &BluezManager::AdapterRemoved,
            this, &ManagerPrivate::managerAdapterRemoved);
}

void ManagerPrivate::managerAdapterAdded(const QDBusObjectPath &objectPath)
{
    QDBusInterface *bluezAdapter = new QDBusInterface(Strings::orgBluez(), objectPath.path(),
            QStringLiteral("org.bluez.Adapter"), DBusConnection::orgBluez(), this);

    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(bluezAdapter->asyncCall(QStringLiteral("GetProperties")), this);
    watcher->setProperty("bluezAdapter", QVariant::fromValue(bluezAdapter));
    connect(watcher, &QDBusPendingCallWatcher::finished, this, &ManagerPrivate::adapterGetPropertiesFinished);
}

void ManagerPrivate::managerAdapterRemoved(const QDBusObjectPath &objectPath)
{
    removeAdapter(objectPath.path());
}

void ManagerPrivate::adapterGetPropertiesFinished(QDBusPendingCallWatcher *watcher)
{
    const QDBusPendingReply<QVariantMap> &reply = *watcher;
    const QVariantMap &properties = reply.value();
    watcher->deleteLater();

    QDBusInterface *bluezAdapter = watcher->property("bluezAdapter").value<QDBusInterface *>();
    bluezAdapter->deleteLater();

    if (reply.isError()) {
        if (!m_initialized) {
            Q_EMIT initError(reply.error().message());
        }
        return;
    }

    addAdapter(bluezAdapter->path(), properties);

    QVariant deviceList = properties.value(QStringLiteral("Devices"));
    if (deviceList.isValid()) {
        updateDeviceList(bluezAdapter->path(), deviceList);
    }

    if (m_pendingInitializationWatchers.isEmpty()) {
        managerInitialized();
    }
}

void ManagerPrivate::adapterPropertyChanged(const QString &property, const QDBusVariant &value)
{
    BluezAdapter *adapter = qobject_cast<BluezAdapter *>(sender());

    if (adapter && property == QStringLiteral("Devices")) {
        updateDeviceList(adapter->path(), value.variant());
    }
}

void ManagerPrivate::updateDeviceList(const QString &adapterPath, const QVariant &deviceList)
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

    for (it = m_devices.constBegin(); it != m_devices.constEnd(); ++it) {
        const QString &path = it.key();
        const DevicePtr &device = it.value();

        if (device->adapter()->ubi() == adapterPath && !devicePaths.contains(path)) {
            pathsToRemove.append(path);
        }
    }

    Q_FOREACH (const QString &devicePath, devicePaths) {
        if (!m_devices.contains(devicePath)) {
            QDBusPendingCallWatcher *watcher = addDeviceByPath(QDBusObjectPath(devicePath));
            if (!m_initialized && watcher) {
                m_pendingInitializationWatchers.insert(watcher);
            }
        }
    }

    Q_FOREACH (const QString &devicePath, pathsToRemove) {
        removeDevice(devicePath);
    }
}

QDBusPendingCallWatcher *ManagerPrivate::addDeviceByPath(const QDBusObjectPath &objectPath)
{
    if (!m_devices.contains(objectPath.path())) {
        QDBusInterface *bluezDevice = new QDBusInterface(Strings::orgBluez(), objectPath.path(),
                QStringLiteral("org.bluez.Device"), DBusConnection::orgBluez(), this);

        QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(
                    bluezDevice->asyncCall(QStringLiteral("GetProperties")), this);
        watcher->setProperty("bluezDevice", QVariant::fromValue(bluezDevice));
        connect(watcher, &QDBusPendingCallWatcher::finished,
                this, &ManagerPrivate::deviceGetPropertiesFinished);
        return watcher;
    }
    return 0;
}

void ManagerPrivate::deviceGetPropertiesFinished(QDBusPendingCallWatcher *watcher)
{
    m_pendingInitializationWatchers.remove(watcher);

    const QDBusPendingReply<QVariantMap> &reply = *watcher;
    watcher->deleteLater();

    QDBusInterface *bluezDevice = watcher->property("bluezDevice").value<QDBusInterface *>();
    bluezDevice->deleteLater();

    if (!reply.isError()) {
        DevicePtr device = m_devices.value(bluezDevice->path());
        if (device.isNull()) {
            addDevice(bluezDevice->path(), reply.value());
        } else {
            device->d->propertiesChanged(Strings::orgBluezDevice1(), reply.value(), QStringList());
        }
    }

    if (!m_initialized && m_pendingInitializationWatchers.isEmpty()) {
        managerInitialized();
    }
}

void ManagerPrivate::deviceFound(const QString &address, const QVariantMap &values)
{
    BluezAdapter *bluezAdapter = qobject_cast<BluezAdapter *>(sender());
    if (!bluezAdapter) {
        return;
    }

    QHash<QString, DevicePtr>::iterator it;

    for (it = m_devices.begin(); it != m_devices.end(); ++it) {
        const DevicePtr &device = it.value();

        if (device->address() == address) {
            device->d->propertiesChanged(Strings::orgBluezDevice1(), values, QStringList());
            return;
        }
    }

    // 'Adapter' is not included in the property map provided by DeviceFound(), but addDevice()
    // expects this property so it must be present.
    QString addressCopy = address;
    QString devicePath = QString("%1/dev_%2").arg(bluezAdapter->path()).arg(addressCopy.replace(':', '_'));
    QVariantMap properties = values;
    properties.insert(QStringLiteral("Adapter"), QVariant::fromValue(QDBusObjectPath(bluezAdapter->path())));

    addDevice(devicePath, properties);
}

void ManagerPrivate::deviceRemoved(const QDBusObjectPath &objectPath)
{
    removeDevice(objectPath.path());
}

QDBusPendingReply<void> ManagerPrivate::requestDefaultAgent(AdapterPtr adapter, Agent *agent)
{
    Q_ASSERT(adapter);
    Q_ASSERT(agent);

    // Register a proxy for this agent instead of registering it directly. This allows the same
    // agent to be used as the default agent as well as the agent for initiated pairing requests,
    // which is not supported behavior in BlueZ 4.
    ProxyAgent *proxyAgent = adapter->d->createProxyForAgent(agent, "/bluez4_system_agent_proxy");
    QDBusPendingReply<void> reply = adapter->d->m_bluezAdapter->RegisterAgent(proxyAgent->objectPath(), ProxyAgent::capabilityToString(proxyAgent->capability()));

    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply, this);
    watcher->setProperty("proxyAgent", QVariant::fromValue(proxyAgent));
    watcher->setProperty("adapterAddress", adapter->address());
    connect(watcher, &QDBusPendingCallWatcher::finished, this, &ManagerPrivate::requestDefaultAgentFinished);

    return reply;
}

void ManagerPrivate::requestDefaultAgentFinished(QDBusPendingCallWatcher *watcher)
{
    const QDBusPendingReply<void> &reply = *watcher;
    ProxyAgent *proxyAgent = watcher->property("proxyAgent").value<ProxyAgent *>();
    QString adapterAddress = watcher->property("adapterAddress").toString();
    watcher->deleteLater();

    if (reply.isError()) {
        return;
    }

    AdapterPtr adapter = q->adapterForAddress(adapterAddress);
    if (adapter) {
        m_defaultAgents.insert(adapter->ubi(), proxyAgent);

        // Set the adapter to reuse the proxied agent when initiating pairings.
        adapter->d->m_pairingAgent = proxyAgent->agent();
    }
}

QDBusPendingReply<void> ManagerPrivate::unregisterDefaultAgent(AdapterPtr adapter)
{
    Q_ASSERT(adapter);

    if (!m_defaultAgents.contains(adapter->ubi())) {
        return QDBusPendingReply<void>(QDBusMessage::createError(QDBusError::InternalError,
                QStringLiteral("Agent has not been registered with this adapter")));
    }

    ProxyAgent *proxyAgent = m_defaultAgents.value(adapter->ubi());
    QDBusPendingReply<void> reply = adapter->d->m_bluezAdapter->UnregisterAgent(proxyAgent->objectPath());

    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply, this);
    watcher->setProperty("adapterPath", adapter->ubi());
    connect(watcher, &QDBusPendingCallWatcher::finished, this, &ManagerPrivate::unregisterAgentFinished);

    return reply;
}

void ManagerPrivate::unregisterAgentFinished(QDBusPendingCallWatcher *watcher)
{
    const QDBusPendingReply<void> &reply = *watcher;
    QString adapterPath = watcher->property("adapterPath").toString();
    watcher->deleteLater();

    if (reply.isError()) {
        return;
    }

    m_defaultAgents.remove(adapterPath);
}

AdapterPtr ManagerPrivate::findAdapterForDefaultAgent(Agent *agent)
{
    QHash<QString, ProxyAgent *>::const_iterator i;

    for (i = m_defaultAgents.constBegin(); i != m_defaultAgents.constEnd(); ++i) {
        const QString &adapterPath = i.key();
        ProxyAgent *proxyAgent = i.value();

        if (proxyAgent == agent) {
            return q->adapterForUbi(adapterPath);
        }
    }

    return AdapterPtr();
}

void ManagerPrivate::agentCreated(Agent *agent)
{
    // Initialize the agent as per Manager::registerAgent()

    new AgentAdaptor(agent, q);

    if (!DBusConnection::orgBluez().registerObject(agent->objectPath().path(), agent)) {
        qCWarning(BLUEZQT) << "Cannot register object" << agent->objectPath().path();
    }
}
#endif

} // namespace BluezQt
