# Qt network access module

HEADERS += \
    access/qhttpnetworkheader_p.h \
    access/qhttpnetworkrequest_p.h \
    access/qhttpnetworkreply_p.h \
    access/qhttpnetworkconnection_p.h \
    access/qhttpnetworkconnectionchannel_p.h \
    access/qabstractprotocolhandler_p.h \
    access/qhttpprotocolhandler_p.h \
    access/qspdyprotocolhandler_p.h \
    access/qnetworkaccessauthenticationmanager_p.h \
    access/qnetworkaccessmanager.h \
    access/qnetworkaccessmanager_p.h \
    access/qnetworkaccesscache_p.h \
    access/qnetworkaccessbackend_p.h \
    access/qnetworkaccessdebugpipebackend_p.h \
    access/qnetworkaccessfilebackend_p.h \
    access/qnetworkaccesscachebackend_p.h \
    access/qnetworkcookie.h \
    access/qnetworkcookie_p.h \
    access/qnetworkcookiejar.h \
    access/qnetworkcookiejar_p.h \
    access/qnetworkrequest.h \
    access/qnetworkrequest_p.h \
    access/qnetworkreply.h \
    access/qnetworkreply_p.h \
    access/qnetworkreplyimpl_p.h \
    access/qnetworkreplydataimpl_p.h \
    access/qnetworkreplyhttpimpl_p.h \
    access/qnetworkreplyfileimpl_p.h \
    access/qabstractnetworkcache_p.h \
    access/qabstractnetworkcache.h \
    access/qhttpthreaddelegate_p.h \
    access/qhttpmultipart.h \
    access/qhttpmultipart_p.h \
    access/qnetworkfile_p.h \
    access/qhttp2protocolhandler_p.h \
    access/qhsts_p.h \
    access/qhstspolicy.h

SOURCES += \
    access/qhttpnetworkheader.cpp \
    access/qhttpnetworkrequest.cpp \
    access/qhttpnetworkreply.cpp \
    access/qhttpnetworkconnection.cpp \
    access/qhttpnetworkconnectionchannel.cpp \
    access/qabstractprotocolhandler.cpp \
    access/qhttpprotocolhandler.cpp \
    access/qspdyprotocolhandler.cpp \
    access/qnetworkaccessauthenticationmanager.cpp \
    access/qnetworkaccessmanager.cpp \
    access/qnetworkaccesscache.cpp \
    access/qnetworkaccessbackend.cpp \
    access/qnetworkaccessdebugpipebackend.cpp \
    access/qnetworkaccessfilebackend.cpp \
    access/qnetworkaccesscachebackend.cpp \
    access/qnetworkcookie.cpp \
    access/qnetworkcookiejar.cpp \
    access/qnetworkrequest.cpp \
    access/qnetworkreply.cpp \
    access/qnetworkreplyimpl.cpp \
    access/qnetworkreplydataimpl.cpp \
    access/qnetworkreplyhttpimpl.cpp \
    access/qnetworkreplyfileimpl.cpp \
    access/qabstractnetworkcache.cpp \
    access/qhttpthreaddelegate.cpp \
    access/qhttpmultipart.cpp \
    access/qnetworkfile.cpp \
    access/qhttp2protocolhandler.cpp \
    access/qhsts.cpp \
    access/qhstspolicy.cpp

qtConfig(ftp) {
    HEADERS += \
        access/qftp_p.h \
        access/qnetworkaccessftpbackend_p.h

    SOURCES += \
        access/qftp.cpp \
        access/qnetworkaccessftpbackend.cpp
}

qtConfig(networkdiskcache) {
    HEADERS += \
        access/qnetworkdiskcache_p.h \
        access/qnetworkdiskcache.h

    SOURCES += access/qnetworkdiskcache.cpp
}

mac: LIBS_PRIVATE += -framework Security

include($$PWD/../../3rdparty/zlib_dependency.pri)
include($$PWD/http2/http2.pri)
