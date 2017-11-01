TARGET     = QtXcbQpa
CONFIG += no_module_headers internal_module
DEFINES += QT_NO_FOREACH

QT += \
    core-private gui-private \
    service_support-private theme_support-private \
    eventdispatcher_support-private fontdatabase_support-private

qtHaveModule(linuxaccessibility_support-private): \
    QT += linuxaccessibility_support-private

SOURCES = \
        qxcbclipboard.cpp \
        qxcbconnection.cpp \
        qxcbintegration.cpp \
        qxcbkeyboard.cpp \
        qxcbmime.cpp \
        qxcbdrag.cpp \
        qxcbscreen.cpp \
        qxcbwindow.cpp \
        qxcbbackingstore.cpp \
        qxcbwmsupport.cpp \
        qxcbnativeinterface.cpp \
        qxcbcursor.cpp \
        qxcbimage.cpp \
        qxcbxsettings.cpp \
        qxcbsystemtraytracker.cpp

HEADERS = \
        qxcbclipboard.h \
        qxcbconnection.h \
        qxcbintegration.h \
        qxcbkeyboard.h \
        qxcbdrag.h \
        qxcbmime.h \
        qxcbobject.h \
        qxcbscreen.h \
        qxcbwindow.h \
        qxcbbackingstore.h \
        qxcbwmsupport.h \
        qxcbnativeinterface.h \
        qxcbcursor.h \
        qxcbimage.h \
        qxcbxsettings.h \
        qxcbsystemtraytracker.h

load(qt_build_paths)

DEFINES += QT_BUILD_XCB_PLUGIN

qtConfig(xcb-xlib) {
    QMAKE_USE += xcb_xlib

    qtConfig(xinput2) {
        SOURCES += qxcbconnection_xi2.cpp
        QMAKE_USE += xinput2
    }
}

qtConfig(xcb-sm) {
    QMAKE_USE += x11sm
    SOURCES += qxcbsessionmanager.cpp
    HEADERS += qxcbsessionmanager.h
}

include(gl_integrations/gl_integrations.pri)

!qtConfig(system-xcb) {
    QMAKE_USE += xcb-static xcb
} else {
    qtConfig(xkb): QMAKE_USE += xcb_xkb
    qtConfig(xcb-render): QMAKE_USE += xcb_render
    QMAKE_USE += xcb_syslibs
}

# libxkbcommon
!qtConfig(xkbcommon-system) {
    qtConfig(xkb) {
        include(../../../3rdparty/xkbcommon-x11.pri)
    } else {
        include(../../../3rdparty/xkbcommon.pri)
    }
} else {
    QMAKE_USE += xkbcommon
}

load(qt_module)
