######################################################################
# Automatically generated by qmake (2.01a) Fri Mar 9 11:37:29 2007
######################################################################

TEMPLATE = app
TARGET = bin/AppConnectExample
DEPENDPATH += . src
INCLUDEPATH += . src

CONFIG += qt
# Input
HEADERS += src/mainUI.h src/TCPStream.h
SOURCES += src/main.cpp src/mainUI.cpp src/TCPStream.cpp
OBJECTS_DIR += src/obj/
MOC_DIR += src/moc/

win32 {
	CONFIG += windows
	LIBS += -lwsock32
}
