TARGET = KF5BluezQt
CONFIG += c++11 create_pc create_prl no_install_prl

QT += dbus network 
QT -= gui

TEMPLATE = lib

DEFINES += BLUEZQT_LIBRARY

isEmpty(PREFIX) {
  PREFIX=/usr
}

include(../bluezqt_version.pri)

XML_DIR = $$PWD/interfaces
system(qdbusxml2cpp -c ObjectManager -p dbusobjectmanager.h:dbusobjectmanager.cpp $$XML_DIR/org.freedesktop.DBus.ObjectManager.xml -i $$PWD/bluezqt_dbustypes.h)
system(qdbusxml2cpp -c ObexFileTransfer -p obexfiletransfer1.h:obexfiletransfer1.cpp $$XML_DIR/org.bluez.obex.FileTransfer1.xml -i $$PWD/bluezqt_dbustypes.h)
system(qdbusxml2cpp -c Properties -p dbusproperties.h:dbusproperties.cpp $$XML_DIR/org.freedesktop.DBus.Properties.xml)
system(qdbusxml2cpp -c BluezAdapter -p bluezadapter1.h:bluezadapter1.cpp $$XML_DIR/org.bluez.Adapter1.xml)
system(qdbusxml2cpp -c BluezAgentManager -p bluezagentmanager1.h:bluezagentmanager1.cpp $$XML_DIR/org.bluez.AgentManager1.xml)
system(qdbusxml2cpp -c BluezProfileManager -p bluezprofilemanager1.h:bluezprofilemanager1.cpp $$XML_DIR/org.bluez.ProfileManager1.xml)
system(qdbusxml2cpp -c BluezDevice -p bluezdevice1.h:bluezdevice1.cpp $$XML_DIR/org.bluez.Device1.xml)
system(qdbusxml2cpp -c BluezMediaPlayer -p bluezmediaplayer1.h:bluezmediaplayer1.cpp $$XML_DIR/org.bluez.MediaPlayer1.xml)
system(qdbusxml2cpp -c ObexAgentManager -p obexagentmanager1.h:obexagentmanager1.cpp $$XML_DIR/org.bluez.obex.AgentManager1.xml)
system(qdbusxml2cpp -c ObexClient -p obexclient1.h:obexclient1.cpp $$XML_DIR/org.bluez.obex.Client1.xml)
system(qdbusxml2cpp -c ObexTransfer -p obextransfer1.h:obextransfer1.cpp $$XML_DIR/org.bluez.obex.Transfer1.xml)
system(qdbusxml2cpp -c ObexSession -p obexsession1.h:obexsession1.cpp $$XML_DIR/org.bluez.obex.Session1.xml)
system(qdbusxml2cpp -c ObexObjectPush -p obexobjectpush1.h:obexobjectpush1.cpp $$XML_DIR/org.bluez.obex.ObjectPush1.xml)

DBUS_SOURCES += \
    dbusobjectmanager.cpp \
    obexfiletransfer1.cpp \
    dbusproperties.cpp \
    bluezadapter1.cpp \
    bluezagentmanager1.cpp \
    bluezprofilemanager1.cpp \
    bluezdevice1.cpp \
    bluezmediaplayer1.cpp \
    obexagentmanager1.cpp \
    obexclient1.cpp \
    obextransfer1.cpp \
    obexsession1.cpp \
    obexobjectpush1.cpp

DBUS_HEADERS += \
    dbusobjectmanager.h \
    obexfiletransfer1.h \
    dbusproperties.h \
    bluezadapter1.h \
    bluezagentmanager1.h \
    bluezprofilemanager1.h \
    bluezdevice1.h \
    bluezmediaplayer1.h \
    obexagentmanager1.h \
    obexclient1.h \
    obextransfer1.h \
    obexsession1.h \
    obexobjectpush1.h

SOURCES += \
    $$DBUS_SOURCES \
    debug.cpp \
    manager.cpp \
    manager_p.cpp \
    adapter.cpp \
    adapter_p.cpp \
    device.cpp \
    device_p.cpp \
    input.cpp \
    mediaplayer.cpp \
    mediaplayer_p.cpp \
    mediaplayertrack.cpp \
    devicesmodel.cpp \
    job.cpp \
    initmanagerjob.cpp \
    initobexmanagerjob.cpp \
    utils.cpp \
    agent.cpp \
    agentadaptor.cpp \
    profile.cpp \
    profileadaptor.cpp \
    pendingcall.cpp \
    request.cpp \
    rfkill.cpp \
    obexmanager.cpp \
    obexmanager_p.cpp \
    obexagent.cpp \
    obexagentadaptor.cpp \
    obextransfer.cpp \
    obexsession.cpp \
    obexobjectpush.cpp \
    obexfiletransfer.cpp \
    obexfiletransferentry.cpp 

PUBLIC_HEADERS += \
    bluezqt_export.h \
    types.h \
    manager.h \
    adapter.h \
    device.h \
    input.h \
    mediaplayer.h \
    mediaplayertrack.h \
    devicesmodel.h \
    job.h \
    initmanagerjob.h \
    initobexmanagerjob.h \
    agent.h \
    profile.h \
    pendingcall.h \
    request.h \
    obexmanager.h \
    obexagent.h \
    obextransfer.h \
    obexsession.h \
    obexobjectpush.h \
    obexfiletransfer.h \
    obexfiletransferentry.h

HEADERS += \
    $$PUBLIC_HEADERS \
    $$DBUS_HEADERS \
    agentadaptor.h \
    obexagentadaptor.h \
    profileadaptor.h \
    adapter_p.h \
    device_p.h \
    input_p.h \
    job_p.h \
    manager_p.h \
    mediaplayer_p.h \
    obexmanager_p.h \
    obexsession_p.h \
    obextransfer_p.h \
    profile_p.h \
    debug.h \
    utils.h \
    rfkill.h

OTHER_FILES += \
    $$XML_DIR/*.xml

target.path = $$[QT_INSTALL_LIBS]

headers.files = $$PUBLIC_HEADERS
headers.path = $$INSTALL_ROOT$$PREFIX/include/KF5/BluezQt/bluezqt

pkgconfig.files = KF5BluezQt.pc
pkgconfig.path = $$[QT_INSTALL_LIBS]/pkgconfig

QMAKE_PKGCONFIG_NAME = $$TARGET
QMAKE_PKGCONFIG_DESTDIR = pkgconfig
QMAKE_PKGCONFIG_INCDIR = $$headers.path
QMAKE_PKGCONFIG_DESCRIPTION = Qt bindings for BlueZ 5
QMAKE_PKGCONFIG_PREFIX = $$PREFIX
QMAKE_PKGCONFIG_VERSION = $$VERSION

INSTALLS += target headers pkgconfig
