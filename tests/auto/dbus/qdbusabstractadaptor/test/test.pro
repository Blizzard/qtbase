CONFIG += testcase
SOURCES += ../tst_qdbusabstractadaptor.cpp
HEADERS += ../myobject.h
TARGET = ../tst_qdbusabstractadaptor

QT = core core-private dbus testlib
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0

macx:CONFIG += insignificant_test # QTBUG-37469
