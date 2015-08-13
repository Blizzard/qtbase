CONFIG += installed
include(../common/common.pri)

winrt: LIBS_PRIVATE += -ld3d11

LIBS_PRIVATE += -ldxguid -L$$QT_BUILD_TREE/lib -l$$qtLibraryTarget(libGLESv2)

DEFINES += GL_APICALL= GL_GLEXT_PROTOTYPES= EGLAPI= LIBEGL_IMPLEMENTATION

HEADERS += \
    $$ANGLE_DIR/src/libEGL/resource.h

SOURCES += \
    $$ANGLE_DIR/src/libEGL/libEGL.cpp

!static {
    DEF_FILE = $$ANGLE_DIR/src/libEGL/$${TARGET}.def
    mingw:equals(QT_ARCH, i386): DEF_FILE = $$ANGLE_DIR/src/libEGL/$${TARGET}_mingw32.def
}

egl_headers.files = \
    $$ANGLE_DIR/include/EGL/egl.h \
    $$ANGLE_DIR/include/EGL/eglext.h \
    $$ANGLE_DIR/include/EGL/eglplatform.h
egl_headers.path = $$[QT_INSTALL_HEADERS]/QtANGLE/EGL
INSTALLS += egl_headers
