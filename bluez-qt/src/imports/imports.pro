TARGET = bluezqtextensionplugin
TEMPLATE = lib
CONFIG += c++11 plugin
QMAKE_CXXFLAGS += -Wno-unused-parameter -Wno-psabi

QT_VERSION=$$[QT_VERSION]

QT += qml
QT -= gui
LIBS += -L$$PWD/../ -lKF5BluezQt

INCLUDEPATH += $$PWD/../

HEADERS += \
    declarativemanager.h \
    declarativeadapter.h \
    declarativedevice.h \
    declarativeinput.h \
    declarativemediaplayer.h \
    declarativedevicesmodel.h \
    bluezqtextensionplugin.h

SOURCES = \
    declarativemanager.cpp \
    declarativeadapter.cpp \
    declarativedevice.cpp \
    declarativeinput.cpp \
    declarativemediaplayer.cpp \
    declarativedevicesmodel.cpp \
    bluezqtextensionplugin.cpp

QML_FILES += \
    qmldir \
    *.qml

OTHER_FILES += $$QML_FILES

target.path = $$[QT_INSTALL_QML]/org/kde/bluezqt
qml.path = $$[QT_INSTALL_QML]/org/kde/bluezqt
qml.files += $$QML_FILES

INSTALLS += target qml
