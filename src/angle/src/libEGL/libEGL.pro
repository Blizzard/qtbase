CONFIG += installed
include(../common/common.pri)

angle_d3d11: \
    LIBS_PRIVATE += -ld3d11
!winrt: \
    LIBS_PRIVATE += -ld3d9

LIBS_PRIVATE += -ldxguid -L$$QT_BUILD_TREE/lib -l$$qtLibraryTarget(libGLESv2)

HEADERS += \
    $$ANGLE_DIR/src/libEGL/Config.h \
    $$ANGLE_DIR/src/libEGL/Display.h \
    $$ANGLE_DIR/src/libEGL/main.h \
    $$ANGLE_DIR/src/libEGL/resource.h \
    $$ANGLE_DIR/src/libEGL/ShaderCache.h \
    $$ANGLE_DIR/src/libEGL/Surface.h

SOURCES += \
    $$ANGLE_DIR/src/libEGL/Config.cpp \
    $$ANGLE_DIR/src/libEGL/Display.cpp \
    $$ANGLE_DIR/src/libEGL/libEGL.cpp \
    $$ANGLE_DIR/src/libEGL/main.cpp \
    $$ANGLE_DIR/src/libEGL/Surface.cpp

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
