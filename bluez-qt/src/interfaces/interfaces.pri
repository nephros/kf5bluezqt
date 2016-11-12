SRC_DIR=$$PWD/..

system(qdbusxml2cpp -c ObjectManager -p $$SRC_DIR/dbusobjectmanager.h:$$SRC_DIR/dbusobjectmanager.cpp $$PWD/org.freedesktop.DBus.ObjectManager.xml -i $$SRC_DIR/bluezqt_dbustypes.h)
system(qdbusxml2cpp -c ObexFileTransfer -p $$SRC_DIR/obexfiletransfer1.h:$$SRC_DIR/obexfiletransfer1.cpp $$PWD/org.bluez.obex.FileTransfer1.xml -i $$SRC_DIR/bluezqt_dbustypes.h)
system(qdbusxml2cpp -c Properties -p $$SRC_DIR/dbusproperties.h:$$SRC_DIR/dbusproperties.cpp $$PWD/org.freedesktop.DBus.Properties.xml)
system(qdbusxml2cpp -c BluezAdapter1 -p $$SRC_DIR/bluezadapter1.h:$$SRC_DIR/bluezadapter1.cpp $$PWD/org.bluez.Adapter1.xml)
system(qdbusxml2cpp -c BluezAgentManager1 -p $$SRC_DIR/bluezagentmanager1.h:$$SRC_DIR/bluezagentmanager1.cpp $$PWD/org.bluez.AgentManager1.xml)
system(qdbusxml2cpp -c BluezProfileManager1 -p $$SRC_DIR/bluezprofilemanager1.h:$$SRC_DIR/bluezprofilemanager1.cpp $$PWD/org.bluez.ProfileManager1.xml)
system(qdbusxml2cpp -c BluezDevice1 -p $$SRC_DIR/bluezdevice1.h:$$SRC_DIR/bluezdevice1.cpp $$PWD/org.bluez.Device1.xml)
system(qdbusxml2cpp -c BluezMediaPlayer1 -p $$SRC_DIR/bluezmediaplayer1.h:$$SRC_DIR/bluezmediaplayer1.cpp $$PWD/org.bluez.MediaPlayer1.xml)
system(qdbusxml2cpp -c BluezMediaTransport1 -p $$SRC_DIR/bluezmediatransport1.h:$$SRC_DIR/bluezmediatransport1.cpp $$PWD/org.bluez.MediaTransport1.xml)
system(qdbusxml2cpp -c ObexAgentManager1 -p $$SRC_DIR/obexagentmanager1.h:$$SRC_DIR/obexagentmanager1.cpp $$PWD/org.bluez.obex.AgentManager1.xml)
system(qdbusxml2cpp -c ObexClient1 -p $$SRC_DIR/obexclient1.h:$$SRC_DIR/obexclient1.cpp $$PWD/org.bluez.obex.Client1.xml)
system(qdbusxml2cpp -c ObexTransfer1 -p $$SRC_DIR/obextransfer1.h:$$SRC_DIR/obextransfer1.cpp $$PWD/org.bluez.obex.Transfer1.xml)
system(qdbusxml2cpp -c ObexSession1 -p $$SRC_DIR/obexsession1.h:$$SRC_DIR/obexsession1.cpp $$PWD/org.bluez.obex.Session1.xml)
system(qdbusxml2cpp -c ObexObjectPush1 -p $$SRC_DIR/obexobjectpush1.h:$$SRC_DIR/obexobjectpush1.cpp $$PWD/org.bluez.obex.ObjectPush1.xml)

DBUS_SOURCES += \
    $$SRC_DIR/dbusobjectmanager.cpp \
    $$SRC_DIR/obexfiletransfer1.cpp \
    $$SRC_DIR/dbusproperties.cpp \
    $$SRC_DIR/bluezadapter1.cpp \
    $$SRC_DIR/bluezagentmanager1.cpp \
    $$SRC_DIR/bluezprofilemanager1.cpp \
    $$SRC_DIR/bluezdevice1.cpp \
    $$SRC_DIR/bluezmediaplayer1.cpp \
    $$SRC_DIR/bluezmediatransport1.cpp \
    $$SRC_DIR/obexagentmanager1.cpp \
    $$SRC_DIR/obexclient1.cpp \
    $$SRC_DIR/obextransfer1.cpp \
    $$SRC_DIR/obexsession1.cpp \
    $$SRC_DIR/obexobjectpush1.cpp

DBUS_HEADERS += \
    $$SRC_DIR/dbusobjectmanager.h \
    $$SRC_DIR/obexfiletransfer1.h \
    $$SRC_DIR/dbusproperties.h \
    $$SRC_DIR/bluezadapter1.h \
    $$SRC_DIR/bluezagentmanager1.h \
    $$SRC_DIR/bluezprofilemanager1.h \
    $$SRC_DIR/bluezdevice1.h \
    $$SRC_DIR/bluezmediaplayer1.h \
    $$SRC_DIR/bluezmediatransport1.h \
    $$SRC_DIR/obexagentmanager1.h \
    $$SRC_DIR/obexclient1.h \
    $$SRC_DIR/obextransfer1.h \
    $$SRC_DIR/obexsession1.h \
    $$SRC_DIR/obexobjectpush1.h

SOURCES += \
    $$DBUS_SOURCES

HEADERS += \
    $$DBUS_HEADERS

OTHER_FILES += \
    $$PWD/*.xml

