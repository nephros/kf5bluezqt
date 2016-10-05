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

equals(KF5BLUEZQT_BLUEZ_VERSION, 5) {
    DEFINES += "KF5BLUEZQT_BLUEZ_VERSION=5"
    include(interfaces/interfaces.pri)
} else {
    DEFINES += "KF5BLUEZQT_BLUEZ_VERSION=4"
    include(interfaces/bluez4/bluez4.pri)
}

SOURCES += \
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
    mediatransport.cpp \
    mediatransport_p.cpp \
    mediatransportsocketinfo.cpp \
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
    mediatransport.h \
    mediatransportsocketinfo.h \
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
    agentadaptor.h \
    obexagentadaptor.h \
    profileadaptor.h \
    adapter_p.h \
    device_p.h \
    input_p.h \
    job_p.h \
    manager_p.h \
    mediaplayer_p.h \
    mediatransport_p.h \
    obexmanager_p.h \
    obexsession_p.h \
    obextransfer_p.h \
    profile_p.h \
    debug.h \
    utils.h \
    rfkill.h

target.path = $$[QT_INSTALL_LIBS]

headers.files = $$PUBLIC_HEADERS
headers.path = $$INSTALL_ROOT$$PREFIX/include/KF5/BluezQt/bluezqt

pkgconfig.files = KF5BluezQt.pc
pkgconfig.path = $$[QT_INSTALL_LIBS]/pkgconfig

QMAKE_PKGCONFIG_NAME = $$TARGET
QMAKE_PKGCONFIG_DESTDIR = pkgconfig
QMAKE_PKGCONFIG_INCDIR = $$headers.path
QMAKE_PKGCONFIG_DESCRIPTION = Qt bindings for BlueZ 5 (with partial support for BlueZ 4 backends)
QMAKE_PKGCONFIG_PREFIX = $$PREFIX
QMAKE_PKGCONFIG_VERSION = $$VERSION

INSTALLS += target headers pkgconfig
