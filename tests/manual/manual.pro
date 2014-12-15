TEMPLATE=subdirs

SUBDIRS = bearerex \
filetest \
gestures \
inputmethodhints \
keypadnavigation \
lance \
network_remote_stresstest \
network_stresstest \
qcursor \
qdesktopservices \
qdesktopwidget \
qgraphicsitem \
qgraphicsitemgroup \
qgraphicslayout/flicker \
qhttpnetworkconnection \
qimagereader \
qlayout \
qlocale \
qnetworkaccessmanager/qget \
qnetworkconfigurationmanager \
qnetworkconfiguration \
qnetworkreply \
qscreen \
qssloptions \
qsslsocket \
qtabletevent \
qtexteditlist \
qtbug-8933 \
qtouchevent \
touch \
qwidget_zorder \
repaint \
socketengine \
textrendering \
widgets \
windowflags \
windowgeometry \
windowmodality \
widgetgrab \
xembed-raster \
xembed-widgets \
shortcuts \
dialogs \
windowtransparency \
unc

!contains(QT_CONFIG, openssl):!contains(QT_CONFIG, openssl-linked):SUBDIRS -= qssloptions

contains(QT_CONFIG, opengl) {
    SUBDIRS += qopengltextureblitter
    contains(QT_CONFIG, egl): SUBDIRS += qopenglcontext
}

win32 {
    SUBDIRS -= network_remote_stresstest network_stresstest
    # disable some tests on wince because of missing dependencies
    wince*:SUBDIRS -= lance windowmodality
}

lessThan(QT_MAJOR_VERSION, 5): SUBDIRS -= bearerex lance qnetworkaccessmanager/qget qnetworkreply \
qpainfo qscreen  socketengine xembed-raster xembed-widgets windowtransparency
