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

#ifndef BLUEZQT_OBEXMANAGER_BLUEZ4_P_H
#define BLUEZQT_OBEXMANAGER_BLUEZ4_P_H

#include <QObject>
#include <QDBusAbstractAdaptor>

#include "types.h"
#include "bluezqt_dbustypes.h"
#include "bluez4obexclient.h"
#include "bluez4obexmanager.h"

namespace BluezQt
{

typedef org::bluez::obex::Client Bluez4ObexClient;
typedef org::bluez::obex::Manager Bluez4ObexManager;

class ObexManagerPrivate;
class ObexManagerBluez4;

class ObexManagerNotifier : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.bluezqt.obex.ObexManagerNotifier")
    Q_CLASSINFO("D-Bus Introspection", ""
"  <interface name=\"org.kde.bluezqt.obex.ObexManagerNotifier\">\n"
"    <method name=\"getSessions\">\n"
"       <arg type=\"aa{sv}\" direction=\"out\"/>\n"
"       <annotation name=\"org.qtproject.QtDBus.QtTypeName.Out0\" value=\"QVariantMapList\"/>\n"
"    </method>\n"
"    <method name=\"getTransfers\">\n"
"       <arg type=\"aa{sv}\" direction=\"out\"/>\n"
"       <annotation name=\"org.qtproject.QtDBus.QtTypeName.Out0\" value=\"QVariantMapList\"/>\n"
"    </method>\n"
"    <method name=\"setTransferAborted\">\n"
"       <arg type=\"s\"/>\n"
"    </method>\n"
"  </interface>\n"
        "")

public:
    ObexManagerNotifier(ObexManagerBluez4 *parent);

    inline static QDBusConnection connection() { return QDBusConnection::sessionBus(); }
    inline static QString service() { return QStringLiteral("org.kde.bluezqt.obex"); }
    inline static QString objectPath() { return QStringLiteral("/ObexManagerNotifier"); }
    inline static QString interface() { return QStringLiteral("org.kde.bluezqt.obex.ObexManagerNotifier"); }

    static void notifyTransferAborted(const QString &transferPath);

public Q_SLOTS:
    QVariantMapList getSessions();
    QVariantMapList getTransfers();
    Q_NOREPLY void setTransferAborted(const QString &transferPath);

private:
    friend class ObexManagerBluez4;
    ObexManagerBluez4 *m_managerBluez4;
};


class ObexManagerBluez4 : public QObject
{
    Q_OBJECT

public:
    explicit ObexManagerBluez4(ObexManagerPrivate *obexManagerPrivate);

    void load();

    QDBusPendingReply<void> registerAgent(const QDBusObjectPath &agentPath);
    QDBusPendingReply<void> unregisterAgent(const QDBusObjectPath &agentPath);

    PendingCall *createSession(const QString &destination, const QVariantMap &args);
    PendingCall *removeSession(const QDBusObjectPath &session);

    ObexTransferPtr newObjectPushTransfer(const QDBusObjectPath &transferPath,
                                          const QVariantMap &transferProperties,
                                          const QString &destinationAddress);

    void setTransferAborted(const QString &transferPath);
    QList<QVariantMap> sessionProperties() const;
    QList<QVariantMap> transferProperties() const;
    ObexSessionPtr sessionForObjectPushTransfer(const QDBusObjectPath &transferPath);

    ObexManagerPrivate *m_obexManagerPrivate;
    Bluez4ObexClient *m_bluez4ObexClient;
    Bluez4ObexManager *m_bluez4ObexManager;

Q_SIGNALS:
    void loaded();

private Q_SLOTS:
    void objectPushTransferCreated(const QString &transferPath, const QVariantMap &transferProperties,
                                   const QString &sessionPath, const QVariantMap &sessionProperties);
    void objectPushTransferFinished(const QString &transferPath, const QString &sessionPath, bool success);

private:
    void getSessionsFinished(QDBusPendingCallWatcher *watcher);
    void getTransfersFinished(QDBusPendingCallWatcher *watcher);
    void completeLoading();

    void createSessionFinished(PendingCall *call);
    void removeSessionFinished(PendingCall *call);

    void transferStarted(const QDBusObjectPath &objectPath);
    void transferCompleted(const QDBusObjectPath &objectPath, bool success);

    void notifyObexManagers(const QString &signalName, const QVariantList &args);

    static QString obexSessionPathKey();
    static QString obexTransferPathKey();

    QHash<QString, ObexTransferPtr> m_oppTransfers;
    ObexManagerNotifier *m_managerNotifier;
    bool m_initializedSessions;
    bool m_initializedTransfers;
};

} // namespace BluezQt

#endif // BLUEZQT_OBEXMANAGER_BLUEZ4_P_H
