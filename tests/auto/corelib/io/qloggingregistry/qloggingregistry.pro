TEMPLATE = app
TARGET = tst_qloggingregistry

CONFIG += testcase
QT = core core-private testlib

SOURCES += tst_qloggingregistry.cpp
OTHER_FILES += qtlogging.ini

android:!android-no-sdk: {
    RESOURCES += \
        android_testdata.qrc
}
