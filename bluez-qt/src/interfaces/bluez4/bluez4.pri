SRC_DIR=$$PWD/../..

system(qdbusxml2cpp -c Bluez4Adapter -p $$SRC_DIR/bluez4adapter.h:$$SRC_DIR/bluez4adapter.cpp $$PWD/org.bluez.Adapter.xml)
system(qdbusxml2cpp -c Bluez4Device -p $$SRC_DIR/bluez4device.h:$$SRC_DIR/bluez4device.cpp $$PWD/org.bluez.Device.xml -i $$SRC_DIR/bluez4/bluezqt_dbustypes_bluez4_p.h)
system(qdbusxml2cpp -c Bluez4Manager -p $$SRC_DIR/bluez4manager.h:$$SRC_DIR/bluez4manager.cpp $$PWD/org.bluez.Manager.xml)
system(qdbusxml2cpp -c Bluez4MediaPlayer -p $$SRC_DIR/bluez4mediaplayer.h:$$SRC_DIR/bluez4mediaplayer.cpp $$PWD/org.bluez.MediaPlayer.xml)
system(qdbusxml2cpp -c Bluez4MediaTransport -p $$SRC_DIR/bluez4mediatransport.h:$$SRC_DIR/bluez4mediatransport.cpp $$PWD/org.bluez.MediaTransport.xml -i $$SRC_DIR/bluez4/bluezqt_dbustypes_bluez4_p.h)

system(qdbusxml2cpp -c Bluez4ObexManager -p $$SRC_DIR/bluez4obexmanager.h:$$SRC_DIR/bluez4obexmanager.cpp $$PWD/org.bluez.obex.Manager.xml)
system(qdbusxml2cpp -c Bluez4ObexClient -p $$SRC_DIR/bluez4obexclient.h:$$SRC_DIR/bluez4obexclient.cpp $$PWD/org.bluez.obex.Client.xml)
system(qdbusxml2cpp -c Bluez4ObexFileTransfer -p $$SRC_DIR/bluez4obexfiletransfer.h:$$SRC_DIR/bluez4obexfiletransfer.cpp $$PWD/org.bluez.obex.FileTransfer.xml -i $$SRC_DIR/bluez4/bluezqt_dbustypes_bluez4_p.h -i $$SRC_DIR/bluezqt_dbustypes.h)
system(qdbusxml2cpp -c Bluez4ObexTransfer -p $$SRC_DIR/bluez4obextransfer.h:$$SRC_DIR/bluez4obextransfer.cpp $$PWD/org.bluez.obex.Transfer.xml)
system(qdbusxml2cpp -c Bluez4ObexObjectPush -p $$SRC_DIR/bluez4obexobjectpush.h:$$SRC_DIR/bluez4obexobjectpush.cpp $$PWD/org.bluez.obex.ObjectPush.xml -i $$SRC_DIR/bluez4/bluezqt_dbustypes_bluez4_p.h)

BLUEZ4_DBUS_SOURCES += \
    $$SRC_DIR/bluez4adapter.cpp \
    $$SRC_DIR/bluez4device.cpp \
    $$SRC_DIR/bluez4manager.cpp \
    $$SRC_DIR/bluez4mediaplayer.cpp \
    $$SRC_DIR/bluez4mediatransport.cpp \
    $$SRC_DIR/bluez4obexmanager.cpp \
    $$SRC_DIR/bluez4obexclient.cpp \
    $$SRC_DIR/bluez4obexfiletransfer.cpp \
    $$SRC_DIR/bluez4obextransfer.cpp \
    $$SRC_DIR/bluez4obexobjectpush.cpp

BLUEZ4_DBUS_HEADERS += \
    $$SRC_DIR/bluez4adapter.h \
    $$SRC_DIR/bluez4device.h \
    $$SRC_DIR/bluez4manager.h \
    $$SRC_DIR/bluez4mediaplayer.h \
    $$SRC_DIR/bluez4mediatransport.h \
    $$SRC_DIR/bluez4obexmanager.h \
    $$SRC_DIR/bluez4obexclient.h \
    $$SRC_DIR/bluez4obexfiletransfer.h \
    $$SRC_DIR/bluez4obextransfer.h \
    $$SRC_DIR/bluez4obexobjectpush.h

SOURCES += \
    $$BLUEZ4_DBUS_SOURCES

HEADERS += \
    $$BLUEZ4_DBUS_HEADERS

OTHER_FILES += \
    $$PWD/*.xml

