TARGET     = QtOpenGL
QT         = core-private gui-private widgets-private

DEFINES   += QT_NO_USING_NAMESPACE QT_NO_FOREACH

win32-msvc*|win32-icc:QMAKE_LFLAGS += /BASE:0x63000000
solaris-cc*:QMAKE_CXXFLAGS_RELEASE -= -O2
irix-cc*:QMAKE_CXXFLAGS += -no_prelink -ptused

QMAKE_DOCS = $$PWD/doc/qtopengl.qdocconf

qtConfig(opengl): CONFIG += opengl
qtConfig(opengles2): CONFIG += opengles2

HEADERS += qgl.h \
           qgl_p.h \
           qglcolormap.h \
           qglfunctions.h \
           qglpixelbuffer.h \
           qglpixelbuffer_p.h \
           qglframebufferobject.h  \
           qglframebufferobject_p.h  \
           qglpaintdevice_p.h \
           qglbuffer.h \
           qtopenglglobal.h

SOURCES += qgl.cpp \
           qglcolormap.cpp \
           qglfunctions.cpp \
           qglpixelbuffer.cpp \
           qglframebufferobject.cpp \
           qglpaintdevice.cpp \
           qglbuffer.cpp \

HEADERS +=  qglshaderprogram.h \
            gl2paintengineex/qglgradientcache_p.h \
            gl2paintengineex/qglengineshadermanager_p.h \
            gl2paintengineex/qgl2pexvertexarray_p.h \
            gl2paintengineex/qpaintengineex_opengl2_p.h \
            gl2paintengineex/qglengineshadersource_p.h \
            gl2paintengineex/qglcustomshaderstage_p.h \
            gl2paintengineex/qtextureglyphcache_gl_p.h \
            gl2paintengineex/qglshadercache_p.h

SOURCES +=  qglshaderprogram.cpp \
            gl2paintengineex/qglgradientcache.cpp \
            gl2paintengineex/qglengineshadermanager.cpp \
            gl2paintengineex/qgl2pexvertexarray.cpp \
            gl2paintengineex/qpaintengineex_opengl2.cpp \
            gl2paintengineex/qglcustomshaderstage.cpp \
            gl2paintengineex/qtextureglyphcache_gl.cpp

qtConfig(graphicseffect) {
    HEADERS += qgraphicsshadereffect_p.h
    SOURCES += qgraphicsshadereffect.cpp
}

load(qt_module)
