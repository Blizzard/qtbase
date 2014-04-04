TARGET = qdirect2d

PLUGIN_TYPE = platforms
PLUGIN_CLASS_NAME = QWindowsDirect2DIntegrationPlugin
load(qt_plugin)

QT *= core-private
QT *= gui-private
QT *= platformsupport-private

LIBS *= -ld2d1 -ld3d11 -ldwrite

include(../windows/windows.pri)

SOURCES += \
    qwindowsdirect2dpaintengine.cpp \
    qwindowsdirect2dpaintdevice.cpp \
    qwindowsdirect2dplatformpixmap.cpp \
    qwindowsdirect2dcontext.cpp \
    qwindowsdirect2dbitmap.cpp \
    qwindowsdirect2dbackingstore.cpp \
    qwindowsdirect2dintegration.cpp \
    qwindowsdirect2dplatformplugin.cpp \
    qwindowsdirect2ddevicecontext.cpp \
    qwindowsdirect2dnativeinterface.cpp \
    qwindowsdirect2dwindow.cpp

HEADERS += \
    qwindowsdirect2dpaintengine.h \
    qwindowsdirect2dpaintdevice.h \
    qwindowsdirect2dplatformpixmap.h \
    qwindowsdirect2dcontext.h \
    qwindowsdirect2dhelpers.h \
    qwindowsdirect2dbitmap.h \
    qwindowsdirect2dbackingstore.h \
    qwindowsdirect2dintegration.h \
    qwindowsdirect2ddevicecontext.h \
    qwindowsdirect2dnativeinterface.h \
    qwindowsdirect2dwindow.h

OTHER_FILES += direct2d.json
