SRC_DIR=$$PWD/../..

system(qdbusxml2cpp -c BluezAdapter -p $$SRC_DIR/bluezadapter.h:$$SRC_DIR/bluezadapter.cpp $$PWD/org.bluez.Adapter.xml)
system(qdbusxml2cpp -c BluezDevice -p $$SRC_DIR/bluezdevice.h:$$SRC_DIR/bluezdevice.cpp $$PWD/org.bluez.Device.xml -i $$SRC_DIR/bluezqt_dbustypes.h)
system(qdbusxml2cpp -c BluezManager -p $$SRC_DIR/bluezmanager.h:$$SRC_DIR/bluezmanager.cpp $$PWD/org.bluez.Manager.xml)
system(qdbusxml2cpp -c BluezMediaPlayer -p $$SRC_DIR/bluezmediaplayer.h:$$SRC_DIR/bluezmediaplayer.cpp $$PWD/org.bluez.MediaPlayer.xml)
system(qdbusxml2cpp -c BluezMediaTransport -p $$SRC_DIR/bluezmediatransport.h:$$SRC_DIR/bluezmediatransport.cpp $$PWD/org.bluez.MediaTransport.xml -i $$SRC_DIR/bluezqt_dbustypes.h)

system(qdbusxml2cpp -c BluezObexManager -p $$SRC_DIR/bluezobexmanager.h:$$SRC_DIR/bluezobexmanager.cpp $$PWD/org.bluez.obex.Manager.xml)
system(qdbusxml2cpp -c BluezObexClient -p $$SRC_DIR/bluezobexclient.h:$$SRC_DIR/bluezobexclient.cpp $$PWD/org.bluez.obex.Client.xml)
system(qdbusxml2cpp -c BluezObexFileTransfer -p $$SRC_DIR/bluezobexfiletransfer.h:$$SRC_DIR/bluezobexfiletransfer.cpp $$PWD/org.bluez.obex.FileTransfer.xml -i $$SRC_DIR/bluezqt_dbustypes.h)
system(qdbusxml2cpp -c BluezObexTransfer -p $$SRC_DIR/bluezobextransfer.h:$$SRC_DIR/bluezobextransfer.cpp $$PWD/org.bluez.obex.Transfer.xml)
system(qdbusxml2cpp -c BluezObexObjectPush -p $$SRC_DIR/bluezobexobjectpush.h:$$SRC_DIR/bluezobexobjectpush.cpp $$PWD/org.bluez.obex.ObjectPush.xml -i $$SRC_DIR/bluezqt_dbustypes.h)

DBUS_SOURCES += \
    $$SRC_DIR/bluezadapter.cpp \
    $$SRC_DIR/bluezdevice.cpp \
    $$SRC_DIR/bluezmanager.cpp \
    $$SRC_DIR/bluezmediaplayer.cpp \
    $$SRC_DIR/bluezmediatransport.cpp \
    $$SRC_DIR/bluezobexmanager.cpp \
    $$SRC_DIR/bluezobexclient.cpp \
    $$SRC_DIR/bluezobexfiletransfer.cpp \
    $$SRC_DIR/bluezobextransfer.cpp \
    $$SRC_DIR/bluezobexobjectpush.cpp

DBUS_HEADERS += \
    $$SRC_DIR/bluezadapter.h \
    $$SRC_DIR/bluezdevice.h \
    $$SRC_DIR/bluezmanager.h \
    $$SRC_DIR/bluezmediaplayer.h \
    $$SRC_DIR/bluezmediatransport.h \
    $$SRC_DIR/bluezobexmanager.h \
    $$SRC_DIR/bluezobexclient.h \
    $$SRC_DIR/bluezobexfiletransfer.h \
    $$SRC_DIR/bluezobextransfer.h \
    $$SRC_DIR/bluezobexobjectpush.h

SOURCES += \
    $$DBUS_SOURCES

HEADERS += \
    $$DBUS_HEADERS

OTHER_FILES += \
    $$PWD/*.xml

