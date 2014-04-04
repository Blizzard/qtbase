/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtGui module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qopenglfunctions.h"
#include "qopenglextensions_p.h"
#include "qdebug.h"
#include "QtGui/private/qopenglcontext_p.h"
#include "QtGui/private/qopengl_p.h"

QT_BEGIN_NAMESPACE

/*!
    \class QOpenGLFunctions
    \brief The QOpenGLFunctions class provides cross-platform access to the OpenGL/ES 2.0 API.
    \since 5.0
    \ingroup painting-3D
    \inmodule QtGui

    OpenGL/ES 2.0 defines a subset of the OpenGL specification that is
    common across many desktop and embedded OpenGL implementations.
    However, it can be difficult to use the functions from that subset
    because they need to be resolved manually on desktop systems.

    QOpenGLFunctions provides a guaranteed API that is available on all
    OpenGL systems and takes care of function resolution on systems
    that need it.  The recommended way to use QOpenGLFunctions is by
    direct inheritance:

    \code
    class MyGLWindow : public QWindow, protected QOpenGLFunctions
    {
        Q_OBJECT
    public:
        MyGLWindow(QScreen *screen = 0);

    protected:
        void initializeGL();
        void paintGL();

        QOpenGLContext *m_context;
    };

    MyGLWindow(QScreen *screen)
      : QWindow(screen), QOpenGLWidget(parent)
    {
        setSurfaceType(OpenGLSurface);
        create();

        // Create an OpenGL context
        m_context = new QOpenGLContext;
        m_context->create();

        // Setup scene and render it
        initializeGL();
        paintGL()
    }

    void MyGLWindow::initializeGL()
    {
        m_context->makeCurrent(this);
        initializeOpenGLFunctions();
    }
    \endcode

    The \c{paintGL()} function can then use any of the OpenGL/ES 2.0
    functions without explicit resolution, such as glActiveTexture()
    in the following example:

    \code
    void MyGLWindow::paintGL()
    {
        m_context->makeCurrent(this);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, textureId);
        ...
        m_context->swapBuffers(this);
        m_context->doneCurrent();
    }
    \endcode

    QOpenGLFunctions can also be used directly for ad-hoc invocation
    of OpenGL/ES 2.0 functions on all platforms:

    \code
    QOpenGLFunctions glFuncs(QOpenGLContext::currentContext());
    glFuncs.glActiveTexture(GL_TEXTURE1);
    \endcode

    QOpenGLFunctions provides wrappers for all OpenGL/ES 2.0 functions,
    except those like \c{glDrawArrays()}, \c{glViewport()}, and
    \c{glBindTexture()} that don't have portability issues.

    The hasOpenGLFeature() and openGLFeatures() functions can be used
    to determine if the OpenGL implementation has a major OpenGL/ES 2.0
    feature.  For example, the following checks if non power of two
    textures are available:

    \code
    QOpenGLFunctions funcs(QOpenGLContext::currentContext());
    bool npot = funcs.hasOpenGLFeature(QOpenGLFunctions::NPOTTextures);
    \endcode
*/

/*!
    \enum QOpenGLFunctions::OpenGLFeature
    This enum defines OpenGL and OpenGL ES features whose presence
    may depend on the implementation.

    \value Multitexture glActiveTexture() function is available.
    \value Shaders Shader functions are available.
    \value Buffers Vertex and index buffer functions are available.
    \value Framebuffers Framebuffer object functions are available.
    \value BlendColor glBlendColor() is available.
    \value BlendEquation glBlendEquation() is available.
    \value BlendEquationSeparate glBlendEquationSeparate() is available.
    \value BlendFuncSeparate glBlendFuncSeparate() is available.
    \value BlendSubtract Blend subtract mode is available.
    \value CompressedTextures Compressed texture functions are available.
    \value Multisample glSampleCoverage() function is available.
    \value StencilSeparate Separate stencil functions are available.
    \value NPOTTextures Non power of two textures are available.
    \value NPOTTextureRepeat Non power of two textures can use GL_REPEAT as wrap parameter.
    \value FixedFunctionPipeline The fixed function pipeline is available.
*/

// Hidden private fields for additional extension data.
struct QOpenGLFunctionsPrivateEx : public QOpenGLExtensionsPrivate, public QOpenGLSharedResource
{
    QOpenGLFunctionsPrivateEx(QOpenGLContext *context)
        : QOpenGLExtensionsPrivate(context)
        , QOpenGLSharedResource(context->shareGroup())
        , m_features(-1)
        , m_extensions(-1)
    {}

    void invalidateResource()
    {
        m_features = -1;
        m_extensions = -1;
    }

    void freeResource(QOpenGLContext *)
    {
        // no gl resources to free
    }

    int m_features;
    int m_extensions;
};

Q_GLOBAL_STATIC(QOpenGLMultiGroupSharedResource, qt_gl_functions_resource)

static QOpenGLFunctionsPrivateEx *qt_gl_functions(QOpenGLContext *context = 0)
{
    if (!context)
        context = QOpenGLContext::currentContext();
    Q_ASSERT(context);
    QOpenGLFunctionsPrivateEx *funcs =
        qt_gl_functions_resource()->value<QOpenGLFunctionsPrivateEx>(context);
    return funcs;
}

/*!
    Constructs a default function resolver. The resolver cannot
    be used until initializeOpenGLFunctions() is called to specify
    the context.

    \sa initializeOpenGLFunctions()
*/
QOpenGLFunctions::QOpenGLFunctions()
    : d_ptr(0)
{
}

/*!
    Constructs a function resolver for \a context.  If \a context
    is null, then the resolver will be created for the current QOpenGLContext.

    The context or another context in the group must be current.

    An object constructed in this way can only be used with \a context
    and other contexts that share with it.  Use initializeOpenGLFunctions()
    to change the object's context association.

    \sa initializeOpenGLFunctions()
*/
QOpenGLFunctions::QOpenGLFunctions(QOpenGLContext *context)
    : d_ptr(0)
{
    if (context && QOpenGLContextGroup::currentContextGroup() == context->shareGroup())
        d_ptr = qt_gl_functions(context);
    else
        qWarning() << "QOpenGLFunctions created with non-current context";
}

QOpenGLExtensions::QOpenGLExtensions()
    : QOpenGLFunctions()
{
}

QOpenGLExtensions::QOpenGLExtensions(QOpenGLContext *context)
    : QOpenGLFunctions(context)
{
}

/*!
    \fn QOpenGLFunctions::~QOpenGLFunctions()

    Destroys this function resolver.
*/

static int qt_gl_resolve_features()
{
    if (QOpenGLFunctions::platformGLType() == QOpenGLFunctions::GLES2) {
        int features = QOpenGLFunctions::Multitexture |
            QOpenGLFunctions::Shaders |
            QOpenGLFunctions::Buffers |
            QOpenGLFunctions::Framebuffers |
            QOpenGLFunctions::BlendColor |
            QOpenGLFunctions::BlendEquation |
            QOpenGLFunctions::BlendEquationSeparate |
            QOpenGLFunctions::BlendFuncSeparate |
            QOpenGLFunctions::BlendSubtract |
            QOpenGLFunctions::CompressedTextures |
            QOpenGLFunctions::Multisample |
            QOpenGLFunctions::StencilSeparate;
        QOpenGLExtensionMatcher extensions;
        if (extensions.match("GL_IMG_texture_npot"))
            features |= QOpenGLFunctions::NPOTTextures;
        if (extensions.match("GL_OES_texture_npot"))
            features |= QOpenGLFunctions::NPOTTextures |
                QOpenGLFunctions::NPOTTextureRepeat;
        return features;
    } else if (QOpenGLFunctions::platformGLType() == QOpenGLFunctions::GLES1) {
        int features = QOpenGLFunctions::Multitexture |
            QOpenGLFunctions::Buffers |
            QOpenGLFunctions::CompressedTextures |
            QOpenGLFunctions::Multisample;
        QOpenGLExtensionMatcher extensions;
        if (extensions.match("GL_OES_framebuffer_object"))
            features |= QOpenGLFunctions::Framebuffers;
        if (extensions.match("GL_OES_blend_equation_separate"))
            features |= QOpenGLFunctions::BlendEquationSeparate;
        if (extensions.match("GL_OES_blend_func_separate"))
            features |= QOpenGLFunctions::BlendFuncSeparate;
        if (extensions.match("GL_OES_blend_subtract"))
            features |= QOpenGLFunctions::BlendSubtract;
        if (extensions.match("GL_OES_texture_npot"))
            features |= QOpenGLFunctions::NPOTTextures;
        if (extensions.match("GL_IMG_texture_npot"))
            features |= QOpenGLFunctions::NPOTTextures;
        return features;
    } else {
        int features = 0;
        QSurfaceFormat format = QOpenGLContext::currentContext()->format();
        QOpenGLExtensionMatcher extensions;

        // Recognize features by extension name.
        if (extensions.match("GL_ARB_multitexture"))
            features |= QOpenGLFunctions::Multitexture;
        if (extensions.match("GL_ARB_shader_objects"))
            features |= QOpenGLFunctions::Shaders;
        if (extensions.match("GL_EXT_framebuffer_object") ||
            extensions.match("GL_ARB_framebuffer_object"))
            features |= QOpenGLFunctions::Framebuffers;
        if (extensions.match("GL_EXT_blend_color"))
            features |= QOpenGLFunctions::BlendColor;
        if (extensions.match("GL_EXT_blend_equation_separate"))
            features |= QOpenGLFunctions::BlendEquationSeparate;
        if (extensions.match("GL_EXT_blend_func_separate"))
            features |= QOpenGLFunctions::BlendFuncSeparate;
        if (extensions.match("GL_EXT_blend_subtract"))
            features |= QOpenGLFunctions::BlendSubtract;
        if (extensions.match("GL_ARB_texture_compression"))
            features |= QOpenGLFunctions::CompressedTextures;
        if (extensions.match("GL_ARB_multisample"))
            features |= QOpenGLFunctions::Multisample;
        if (extensions.match("GL_ARB_texture_non_power_of_two"))
            features |= QOpenGLFunctions::NPOTTextures |
                QOpenGLFunctions::NPOTTextureRepeat;

        // assume version 2.0 or higher
        features |= QOpenGLFunctions::BlendColor |
            QOpenGLFunctions::BlendEquation |
            QOpenGLFunctions::Multitexture |
            QOpenGLFunctions::CompressedTextures |
            QOpenGLFunctions::Multisample |
            QOpenGLFunctions::BlendFuncSeparate |
            QOpenGLFunctions::Buffers |
            QOpenGLFunctions::Shaders |
            QOpenGLFunctions::StencilSeparate |
            QOpenGLFunctions::BlendEquationSeparate |
            QOpenGLFunctions::NPOTTextures |
            QOpenGLFunctions::NPOTTextureRepeat;

        if (format.majorVersion() >= 3)
            features |= QOpenGLFunctions::Framebuffers;

        const QPair<int, int> version = format.version();
        if (version < qMakePair(3, 0)
            || (version == qMakePair(3, 0) && format.testOption(QSurfaceFormat::DeprecatedFunctions))
            || (version == qMakePair(3, 1) && extensions.match("GL_ARB_compatibility"))
            || (version >= qMakePair(3, 2) && format.profile() == QSurfaceFormat::CompatibilityProfile)) {
            features |= QOpenGLFunctions::FixedFunctionPipeline;
        }
        return features;
    }
}

static int qt_gl_resolve_extensions()
{
    int extensions = 0;
    QOpenGLExtensionMatcher extensionMatcher;
    if (extensionMatcher.match("GL_EXT_bgra"))
        extensions |= QOpenGLExtensions::BGRATextureFormat;

    if (QOpenGLFunctions::isES()) {
        if (extensionMatcher.match("GL_OES_mapbuffer"))
            extensions |= QOpenGLExtensions::MapBuffer;
        if (extensionMatcher.match("GL_OES_packed_depth_stencil"))
            extensions |= QOpenGLExtensions::PackedDepthStencil;
        if (extensionMatcher.match("GL_OES_element_index_uint"))
            extensions |= QOpenGLExtensions::ElementIndexUint;
        if (extensionMatcher.match("GL_OES_depth24"))
            extensions |= QOpenGLExtensions::Depth24;
        // TODO: Consider matching GL_APPLE_texture_format_BGRA8888 as well, but it needs testing.
        if (extensionMatcher.match("GL_IMG_texture_format_BGRA8888") || extensionMatcher.match("GL_EXT_texture_format_BGRA8888"))
            extensions |= QOpenGLExtensions::BGRATextureFormat;
    } else {
        QSurfaceFormat format = QOpenGLContext::currentContext()->format();
        extensions |= QOpenGLExtensions::ElementIndexUint | QOpenGLExtensions::MapBuffer;

        // Recognize features by extension name.
        if (format.majorVersion() >= 3
            || extensionMatcher.match("GL_ARB_framebuffer_object"))
        {
            extensions |= QOpenGLExtensions::FramebufferMultisample |
                QOpenGLExtensions::FramebufferBlit |
                QOpenGLExtensions::PackedDepthStencil;
        } else {
            if (extensionMatcher.match("GL_EXT_framebuffer_multisample"))
                extensions |= QOpenGLExtensions::FramebufferMultisample;
            if (extensionMatcher.match("GL_EXT_framebuffer_blit"))
                extensions |= QOpenGLExtensions::FramebufferBlit;
            if (extensionMatcher.match("GL_EXT_packed_depth_stencil"))
                extensions |= QOpenGLExtensions::PackedDepthStencil;
        }
    }
    return extensions;
}

/*!
    Returns the set of features that are present on this system's
    OpenGL implementation.

    It is assumed that the QOpenGLContext associated with this function
    resolver is current.

    \sa hasOpenGLFeature()
*/
QOpenGLFunctions::OpenGLFeatures QOpenGLFunctions::openGLFeatures() const
{
    QOpenGLFunctionsPrivateEx *d = static_cast<QOpenGLFunctionsPrivateEx *>(d_ptr);
    if (!d)
        return 0;
    if (d->m_features == -1)
        d->m_features = qt_gl_resolve_features();
    return QOpenGLFunctions::OpenGLFeatures(d->m_features);
}

/*!
    Returns \c true if \a feature is present on this system's OpenGL
    implementation; false otherwise.

    It is assumed that the QOpenGLContext associated with this function
    resolver is current.

    \sa openGLFeatures()
*/
bool QOpenGLFunctions::hasOpenGLFeature(QOpenGLFunctions::OpenGLFeature feature) const
{
    QOpenGLFunctionsPrivateEx *d = static_cast<QOpenGLFunctionsPrivateEx *>(d_ptr);
    if (!d)
        return false;
    if (d->m_features == -1)
        d->m_features = qt_gl_resolve_features();
    return (d->m_features & int(feature)) != 0;
}

/*!
    Returns the set of extensions that are present on this system's
    OpenGL implementation.

    It is assumed that the QOpenGLContext associated with this extension
    resolver is current.

    \sa hasOpenGLExtensions()
*/
QOpenGLExtensions::OpenGLExtensions QOpenGLExtensions::openGLExtensions()
{
    QOpenGLFunctionsPrivateEx *d = static_cast<QOpenGLFunctionsPrivateEx *>(d_ptr);
    if (!d)
        return 0;
    if (d->m_extensions == -1)
        d->m_extensions = qt_gl_resolve_extensions();
    return QOpenGLExtensions::OpenGLExtensions(d->m_extensions);
}

/*!
    Returns \c true if \a extension is present on this system's OpenGL
    implementation; false otherwise.

    It is assumed that the QOpenGLContext associated with this extension
    resolver is current.

    \sa openGLFeatures()
*/
bool QOpenGLExtensions::hasOpenGLExtension(QOpenGLExtensions::OpenGLExtension extension) const
{
    QOpenGLFunctionsPrivateEx *d = static_cast<QOpenGLFunctionsPrivateEx *>(d_ptr);
    if (!d)
        return false;
    if (d->m_extensions == -1)
        d->m_extensions = qt_gl_resolve_extensions();
    return (d->m_extensions & int(extension)) != 0;
}

/*!
    \fn void QOpenGLFunctions::initializeGLFunctions()
    \obsolete

    Use initializeOpenGLFunctions() instead.
*/

/*!
    Initializes OpenGL function resolution for the current context.

    After calling this function, the QOpenGLFunctions object can only be
    used with the current context and other contexts that share with it.
    Call initializeOpenGLFunctions() again to change the object's context
    association.
*/
void QOpenGLFunctions::initializeOpenGLFunctions()
{
    d_ptr = qt_gl_functions();
}

/*!
    \fn void QOpenGLFunctions::glActiveTexture(GLenum texture)

    Convenience function that calls glActiveTexture(\a texture).

    For more information, see the OpenGL/ES 2.0 documentation for
    \l{http://www.khronos.org/opengles/sdk/docs/man/glActiveTexture.xml}{glActiveTexture()}.
*/

/*!
    \fn void QOpenGLFunctions::glAttachShader(GLuint program, GLuint shader)

    Convenience function that calls glAttachShader(\a program, \a shader).

    For more information, see the OpenGL/ES 2.0 documentation for
    \l{http://www.khronos.org/opengles/sdk/docs/man/glAttachShader.xml}{glAttachShader()}.

    This convenience function will do nothing on OpenGL/ES 1.x systems.
*/

/*!
    \fn void QOpenGLFunctions::glBindAttribLocation(GLuint program, GLuint index, const char* name)

    Convenience function that calls glBindAttribLocation(\a program, \a index, \a name).

    For more information, see the OpenGL/ES 2.0 documentation for
    \l{http://www.khronos.org/opengles/sdk/docs/man/glBindAttribLocation.xml}{glBindAttribLocation()}.

    This convenience function will do nothing on OpenGL/ES 1.x systems.
*/

/*!
    \fn void QOpenGLFunctions::glBindBuffer(GLenum target, GLuint buffer)

    Convenience function that calls glBindBuffer(\a target, \a buffer).

    For more information, see the OpenGL/ES 2.0 documentation for
    \l{http://www.khronos.org/opengles/sdk/docs/man/glBindBuffer.xml}{glBindBuffer()}.
*/

/*!
    \fn void QOpenGLFunctions::glBindFramebuffer(GLenum target, GLuint framebuffer)

    Convenience function that calls glBindFramebuffer(\a target, \a framebuffer).

    Note that Qt will translate a \a framebuffer argument of 0 to the currently
    bound QOpenGLContext's defaultFramebufferObject().

    For more information, see the OpenGL/ES 2.0 documentation for
    \l{http://www.khronos.org/opengles/sdk/docs/man/glBindFramebuffer.xml}{glBindFramebuffer()}.
*/

/*!
    \fn void QOpenGLFunctions::glBindRenderbuffer(GLenum target, GLuint renderbuffer)

    Convenience function that calls glBindRenderbuffer(\a target, \a renderbuffer).

    For more information, see the OpenGL/ES 2.0 documentation for
    \l{http://www.khronos.org/opengles/sdk/docs/man/glBindRenderbuffer.xml}{glBindRenderbuffer()}.
*/

/*!
    \fn void QOpenGLFunctions::glBlendColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)

    Convenience function that calls glBlendColor(\a red, \a green, \a blue, \a alpha).

    For more information, see the OpenGL/ES 2.0 documentation for
    \l{http://www.khronos.org/opengles/sdk/docs/man/glBlendColor.xml}{glBlendColor()}.
*/

/*!
    \fn void QOpenGLFunctions::glBlendEquation(GLenum mode)

    Convenience function that calls glBlendEquation(\a mode).

    For more information, see the OpenGL/ES 2.0 documentation for
    \l{http://www.khronos.org/opengles/sdk/docs/man/glBlendEquation.xml}{glBlendEquation()}.
*/

/*!
    \fn void QOpenGLFunctions::glBlendEquationSeparate(GLenum modeRGB, GLenum modeAlpha)

    Convenience function that calls glBlendEquationSeparate(\a modeRGB, \a modeAlpha).

    For more information, see the OpenGL/ES 2.0 documentation for
    \l{http://www.khronos.org/opengles/sdk/docs/man/glBlendEquationSeparate.xml}{glBlendEquationSeparate()}.
*/

/*!
    \fn void QOpenGLFunctions::glBlendFuncSeparate(GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha)

    Convenience function that calls glBlendFuncSeparate(\a srcRGB, \a dstRGB, \a srcAlpha, \a dstAlpha).

    For more information, see the OpenGL/ES 2.0 documentation for
    \l{http://www.khronos.org/opengles/sdk/docs/man/glBlendFuncSeparate.xml}{glBlendFuncSeparate()}.
*/

/*!
    \fn void QOpenGLFunctions::glBufferData(GLenum target, qopengl_GLsizeiptr size, const void* data, GLenum usage)

    Convenience function that calls glBufferData(\a target, \a size, \a data, \a usage).

    For more information, see the OpenGL/ES 2.0 documentation for
    \l{http://www.khronos.org/opengles/sdk/docs/man/glBufferData.xml}{glBufferData()}.
*/

/*!
    \fn void QOpenGLFunctions::glBufferSubData(GLenum target, qopengl_GLintptr offset, qopengl_GLsizeiptr size, const void* data)

    Convenience function that calls glBufferSubData(\a target, \a offset, \a size, \a data).

    For more information, see the OpenGL/ES 2.0 documentation for
    \l{http://www.khronos.org/opengles/sdk/docs/man/glBufferSubData.xml}{glBufferSubData()}.
*/

/*!
    \fn GLenum QOpenGLFunctions::glCheckFramebufferStatus(GLenum target)

    Convenience function that calls glCheckFramebufferStatus(\a target).

    For more information, see the OpenGL/ES 2.0 documentation for
    \l{http://www.khronos.org/opengles/sdk/docs/man/glCheckFramebufferStatus.xml}{glCheckFramebufferStatus()}.
*/

/*!
    \fn void QOpenGLFunctions::glClearDepthf(GLclampf depth)

    Convenience function that calls glClearDepth(\a depth) on
    desktop OpenGL systems and glClearDepthf(\a depth) on
    embedded OpenGL/ES systems.

    For more information, see the OpenGL/ES 2.0 documentation for
    \l{http://www.khronos.org/opengles/sdk/docs/man/glClearDepthf.xml}{glClearDepthf()}.
*/

/*!
    \fn void QOpenGLFunctions::glCompileShader(GLuint shader)

    Convenience function that calls glCompileShader(\a shader).

    For more information, see the OpenGL/ES 2.0 documentation for
    \l{http://www.khronos.org/opengles/sdk/docs/man/glCompileShader.xml}{glCompileShader()}.

    This convenience function will do nothing on OpenGL/ES 1.x systems.
*/

/*!
    \fn void QOpenGLFunctions::glCompressedTexImage2D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void* data)

    Convenience function that calls glCompressedTexImage2D(\a target, \a level, \a internalformat, \a width, \a height, \a border, \a imageSize, \a data).

    For more information, see the OpenGL/ES 2.0 documentation for
    \l{http://www.khronos.org/opengles/sdk/docs/man/glCompressedTexImage2D.xml}{glCompressedTexImage2D()}.
*/

/*!
    \fn void QOpenGLFunctions::glCompressedTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void* data)

    Convenience function that calls glCompressedTexSubImage2D(\a target, \a level, \a xoffset, \a yoffset, \a width, \a height, \a format, \a imageSize, \a data).

    For more information, see the OpenGL/ES 2.0 documentation for
    \l{http://www.khronos.org/opengles/sdk/docs/man/glCompressedTexSubImage2D.xml}{glCompressedTexSubImage2D()}.
*/

/*!
    \fn GLuint QOpenGLFunctions::glCreateProgram()

    Convenience function that calls glCreateProgram().

    For more information, see the OpenGL/ES 2.0 documentation for
    \l{http://www.khronos.org/opengles/sdk/docs/man/glCreateProgram.xml}{glCreateProgram()}.

    This convenience function will do nothing on OpenGL/ES 1.x systems.
*/

/*!
    \fn GLuint QOpenGLFunctions::glCreateShader(GLenum type)

    Convenience function that calls glCreateShader(\a type).

    For more information, see the OpenGL/ES 2.0 documentation for
    \l{http://www.khronos.org/opengles/sdk/docs/man/glCreateShader.xml}{glCreateShader()}.

    This convenience function will do nothing on OpenGL/ES 1.x systems.
*/

/*!
    \fn void QOpenGLFunctions::glDeleteBuffers(GLsizei n, const GLuint* buffers)

    Convenience function that calls glDeleteBuffers(\a n, \a buffers).

    For more information, see the OpenGL/ES 2.0 documentation for
    \l{http://www.khronos.org/opengles/sdk/docs/man/glDeleteBuffers.xml}{glDeleteBuffers()}.
*/

/*!
    \fn void QOpenGLFunctions::glDeleteFramebuffers(GLsizei n, const GLuint* framebuffers)

    Convenience function that calls glDeleteFramebuffers(\a n, \a framebuffers).

    For more information, see the OpenGL/ES 2.0 documentation for
    \l{http://www.khronos.org/opengles/sdk/docs/man/glDeleteFramebuffers.xml}{glDeleteFramebuffers()}.
*/

/*!
    \fn void QOpenGLFunctions::glDeleteProgram(GLuint program)

    Convenience function that calls glDeleteProgram(\a program).

    For more information, see the OpenGL/ES 2.0 documentation for
    \l{http://www.khronos.org/opengles/sdk/docs/man/glDeleteProgram.xml}{glDeleteProgram()}.

    This convenience function will do nothing on OpenGL/ES 1.x systems.
*/

/*!
    \fn void QOpenGLFunctions::glDeleteRenderbuffers(GLsizei n, const GLuint* renderbuffers)

    Convenience function that calls glDeleteRenderbuffers(\a n, \a renderbuffers).

    For more information, see the OpenGL/ES 2.0 documentation for
    \l{http://www.khronos.org/opengles/sdk/docs/man/glDeleteRenderbuffers.xml}{glDeleteRenderbuffers()}.
*/

/*!
    \fn void QOpenGLFunctions::glDeleteShader(GLuint shader)

    Convenience function that calls glDeleteShader(\a shader).

    For more information, see the OpenGL/ES 2.0 documentation for
    \l{http://www.khronos.org/opengles/sdk/docs/man/glDeleteShader.xml}{glDeleteShader()}.

    This convenience function will do nothing on OpenGL/ES 1.x systems.
*/

/*!
    \fn void QOpenGLFunctions::glDepthRangef(GLclampf zNear, GLclampf zFar)

    Convenience function that calls glDepthRange(\a zNear, \a zFar) on
    desktop OpenGL systems and glDepthRangef(\a zNear, \a zFar) on
    embedded OpenGL/ES systems.

    For more information, see the OpenGL/ES 2.0 documentation for
    \l{http://www.khronos.org/opengles/sdk/docs/man/glDepthRangef.xml}{glDepthRangef()}.
*/

/*!
    \fn void QOpenGLFunctions::glDetachShader(GLuint program, GLuint shader)

    Convenience function that calls glDetachShader(\a program, \a shader).

    For more information, see the OpenGL/ES 2.0 documentation for
    \l{http://www.khronos.org/opengles/sdk/docs/man/glDetachShader.xml}{glDetachShader()}.

    This convenience function will do nothing on OpenGL/ES 1.x systems.
*/

/*!
    \fn void QOpenGLFunctions::glDisableVertexAttribArray(GLuint index)

    Convenience function that calls glDisableVertexAttribArray(\a index).

    For more information, see the OpenGL/ES 2.0 documentation for
    \l{http://www.khronos.org/opengles/sdk/docs/man/glDisableVertexAttribArray.xml}{glDisableVertexAttribArray()}.

    This convenience function will do nothing on OpenGL/ES 1.x systems.
*/

/*!
    \fn void QOpenGLFunctions::glEnableVertexAttribArray(GLuint index)

    Convenience function that calls glEnableVertexAttribArray(\a index).

    For more information, see the OpenGL/ES 2.0 documentation for
    \l{http://www.khronos.org/opengles/sdk/docs/man/glEnableVertexAttribArray.xml}{glEnableVertexAttribArray()}.

    This convenience function will do nothing on OpenGL/ES 1.x systems.
*/

/*!
    \fn void QOpenGLFunctions::glFramebufferRenderbuffer(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)

    Convenience function that calls glFramebufferRenderbuffer(\a target, \a attachment, \a renderbuffertarget, \a renderbuffer).

    For more information, see the OpenGL/ES 2.0 documentation for
    \l{http://www.khronos.org/opengles/sdk/docs/man/glFramebufferRenderbuffer.xml}{glFramebufferRenderbuffer()}.
*/

/*!
    \fn void QOpenGLFunctions::glFramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)

    Convenience function that calls glFramebufferTexture2D(\a target, \a attachment, \a textarget, \a texture, \a level).

    For more information, see the OpenGL/ES 2.0 documentation for
    \l{http://www.khronos.org/opengles/sdk/docs/man/glFramebufferTexture2D.xml}{glFramebufferTexture2D()}.
*/

/*!
    \fn void QOpenGLFunctions::glGenBuffers(GLsizei n, GLuint* buffers)

    Convenience function that calls glGenBuffers(\a n, \a buffers).

    For more information, see the OpenGL/ES 2.0 documentation for
    \l{http://www.khronos.org/opengles/sdk/docs/man/glGenBuffers.xml}{glGenBuffers()}.
*/

/*!
    \fn void QOpenGLFunctions::glGenerateMipmap(GLenum target)

    Convenience function that calls glGenerateMipmap(\a target).

    For more information, see the OpenGL/ES 2.0 documentation for
    \l{http://www.khronos.org/opengles/sdk/docs/man/glGenerateMipmap.xml}{glGenerateMipmap()}.
*/

/*!
    \fn void QOpenGLFunctions::glGenFramebuffers(GLsizei n, GLuint* framebuffers)

    Convenience function that calls glGenFramebuffers(\a n, \a framebuffers).

    For more information, see the OpenGL/ES 2.0 documentation for
    \l{http://www.khronos.org/opengles/sdk/docs/man/glGenFramebuffers.xml}{glGenFramebuffers()}.
*/

/*!
    \fn void QOpenGLFunctions::glGenRenderbuffers(GLsizei n, GLuint* renderbuffers)

    Convenience function that calls glGenRenderbuffers(\a n, \a renderbuffers).

    For more information, see the OpenGL/ES 2.0 documentation for
    \l{http://www.khronos.org/opengles/sdk/docs/man/glGenRenderbuffers.xml}{glGenRenderbuffers()}.
*/

/*!
    \fn void QOpenGLFunctions::glGetActiveAttrib(GLuint program, GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, char* name)

    Convenience function that calls glGetActiveAttrib(\a program, \a index, \a bufsize, \a length, \a size, \a type, \a name).

    For more information, see the OpenGL/ES 2.0 documentation for
    \l{http://www.khronos.org/opengles/sdk/docs/man/glGetActiveAttrib.xml}{glGetActiveAttrib()}.

    This convenience function will do nothing on OpenGL/ES 1.x systems.
*/

/*!
    \fn void QOpenGLFunctions::glGetActiveUniform(GLuint program, GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, char* name)

    Convenience function that calls glGetActiveUniform(\a program, \a index, \a bufsize, \a length, \a size, \a type, \a name).

    For more information, see the OpenGL/ES 2.0 documentation for
    \l{http://www.khronos.org/opengles/sdk/docs/man/glGetActiveUniform.xml}{glGetActiveUniform()}.

    This convenience function will do nothing on OpenGL/ES 1.x systems.
*/

/*!
    \fn void QOpenGLFunctions::glGetAttachedShaders(GLuint program, GLsizei maxcount, GLsizei* count, GLuint* shaders)

    Convenience function that calls glGetAttachedShaders(\a program, \a maxcount, \a count, \a shaders).

    For more information, see the OpenGL/ES 2.0 documentation for
    \l{http://www.khronos.org/opengles/sdk/docs/man/glGetAttachedShaders.xml}{glGetAttachedShaders()}.

    This convenience function will do nothing on OpenGL/ES 1.x systems.
*/

/*!
    \fn GLint QOpenGLFunctions::glGetAttribLocation(GLuint program, const char* name)

    Convenience function that calls glGetAttribLocation(\a program, \a name).

    For more information, see the OpenGL/ES 2.0 documentation for
    \l{http://www.khronos.org/opengles/sdk/docs/man/glGetAttribLocation.xml}{glGetAttribLocation()}.

    This convenience function will do nothing on OpenGL/ES 1.x systems.
*/

/*!
    \fn void QOpenGLFunctions::glGetBufferParameteriv(GLenum target, GLenum pname, GLint* params)

    Convenience function that calls glGetBufferParameteriv(\a target, \a pname, \a params).

    For more information, see the OpenGL/ES 2.0 documentation for
    \l{http://www.khronos.org/opengles/sdk/docs/man/glGetBufferParameteriv.xml}{glGetBufferParameteriv()}.
*/

/*!
    \fn void QOpenGLFunctions::glGetFramebufferAttachmentParameteriv(GLenum target, GLenum attachment, GLenum pname, GLint* params)

    Convenience function that calls glGetFramebufferAttachmentParameteriv(\a target, \a attachment, \a pname, \a params).

    For more information, see the OpenGL/ES 2.0 documentation for
    \l{http://www.khronos.org/opengles/sdk/docs/man/glGetFramebufferAttachmentParameteriv.xml}{glGetFramebufferAttachmentParameteriv()}.
*/

/*!
    \fn void QOpenGLFunctions::glGetProgramiv(GLuint program, GLenum pname, GLint* params)

    Convenience function that calls glGetProgramiv(\a program, \a pname, \a params).

    For more information, see the OpenGL/ES 2.0 documentation for
    \l{http://www.khronos.org/opengles/sdk/docs/man/glGetProgramiv.xml}{glGetProgramiv()}.

    This convenience function will do nothing on OpenGL/ES 1.x systems.
*/

/*!
    \fn void QOpenGLFunctions::glGetProgramInfoLog(GLuint program, GLsizei bufsize, GLsizei* length, char* infolog)

    Convenience function that calls glGetProgramInfoLog(\a program, \a bufsize, \a length, \a infolog).

    For more information, see the OpenGL/ES 2.0 documentation for
    \l{http://www.khronos.org/opengles/sdk/docs/man/glGetProgramInfoLog.xml}{glGetProgramInfoLog()}.

    This convenience function will do nothing on OpenGL/ES 1.x systems.
*/

/*!
    \fn void QOpenGLFunctions::glGetRenderbufferParameteriv(GLenum target, GLenum pname, GLint* params)

    Convenience function that calls glGetRenderbufferParameteriv(\a target, \a pname, \a params).

    For more information, see the OpenGL/ES 2.0 documentation for
    \l{http://www.khronos.org/opengles/sdk/docs/man/glGetRenderbufferParameteriv.xml}{glGetRenderbufferParameteriv()}.
*/

/*!
    \fn void QOpenGLFunctions::glGetShaderiv(GLuint shader, GLenum pname, GLint* params)

    Convenience function that calls glGetShaderiv(\a shader, \a pname, \a params).

    For more information, see the OpenGL/ES 2.0 documentation for
    \l{http://www.khronos.org/opengles/sdk/docs/man/glGetShaderiv.xml}{glGetShaderiv()}.

    This convenience function will do nothing on OpenGL/ES 1.x systems.
*/

/*!
    \fn void QOpenGLFunctions::glGetShaderInfoLog(GLuint shader, GLsizei bufsize, GLsizei* length, char* infolog)

    Convenience function that calls glGetShaderInfoLog(\a shader, \a bufsize, \a length, \a infolog).

    For more information, see the OpenGL/ES 2.0 documentation for
    \l{http://www.khronos.org/opengles/sdk/docs/man/glGetShaderInfoLog.xml}{glGetShaderInfoLog()}.

    This convenience function will do nothing on OpenGL/ES 1.x systems.
*/

/*!
    \fn void QOpenGLFunctions::glGetShaderPrecisionFormat(GLenum shadertype, GLenum precisiontype, GLint* range, GLint* precision)

    Convenience function that calls glGetShaderPrecisionFormat(\a shadertype, \a precisiontype, \a range, \a precision).

    For more information, see the OpenGL/ES 2.0 documentation for
    \l{http://www.khronos.org/opengles/sdk/docs/man/glGetShaderPrecisionFormat.xml}{glGetShaderPrecisionFormat()}.

    This convenience function will do nothing on OpenGL/ES 1.x systems.
*/

/*!
    \fn void QOpenGLFunctions::glGetShaderSource(GLuint shader, GLsizei bufsize, GLsizei* length, char* source)

    Convenience function that calls glGetShaderSource(\a shader, \a bufsize, \a length, \a source).

    For more information, see the OpenGL/ES 2.0 documentation for
    \l{http://www.khronos.org/opengles/sdk/docs/man/glGetShaderSource.xml}{glGetShaderSource()}.

    This convenience function will do nothing on OpenGL/ES 1.x systems.
*/

/*!
    \fn void QOpenGLFunctions::glGetUniformfv(GLuint program, GLint location, GLfloat* params)

    Convenience function that calls glGetUniformfv(\a program, \a location, \a params).

    For more information, see the OpenGL/ES 2.0 documentation for
    \l{http://www.khronos.org/opengles/sdk/docs/man/glGetUniformfv.xml}{glGetUniformfv()}.

    This convenience function will do nothing on OpenGL/ES 1.x systems.
*/

/*!
    \fn void QOpenGLFunctions::glGetUniformiv(GLuint program, GLint location, GLint* params)

    Convenience function that calls glGetUniformiv(\a program, \a location, \a params).

    For more information, see the OpenGL/ES 2.0 documentation for
    \l{http://www.khronos.org/opengles/sdk/docs/man/glGetUniformiv.xml}{glGetUniformiv()}.

    This convenience function will do nothing on OpenGL/ES 1.x systems.
*/

/*!
    \fn GLint QOpenGLFunctions::glGetUniformLocation(GLuint program, const char* name)

    Convenience function that calls glGetUniformLocation(\a program, \a name).

    For more information, see the OpenGL/ES 2.0 documentation for
    \l{http://www.khronos.org/opengles/sdk/docs/man/glGetUniformLocation.xml}{glGetUniformLocation()}.

    This convenience function will do nothing on OpenGL/ES 1.x systems.
*/

/*!
    \fn void QOpenGLFunctions::glGetVertexAttribfv(GLuint index, GLenum pname, GLfloat* params)

    Convenience function that calls glGetVertexAttribfv(\a index, \a pname, \a params).

    For more information, see the OpenGL/ES 2.0 documentation for
    \l{http://www.khronos.org/opengles/sdk/docs/man/glGetVertexAttribfv.xml}{glGetVertexAttribfv()}.

    This convenience function will do nothing on OpenGL/ES 1.x systems.
*/

/*!
    \fn void QOpenGLFunctions::glGetVertexAttribiv(GLuint index, GLenum pname, GLint* params)

    Convenience function that calls glGetVertexAttribiv(\a index, \a pname, \a params).

    For more information, see the OpenGL/ES 2.0 documentation for
    \l{http://www.khronos.org/opengles/sdk/docs/man/glGetVertexAttribiv.xml}{glGetVertexAttribiv()}.

    This convenience function will do nothing on OpenGL/ES 1.x systems.
*/

/*!
    \fn void QOpenGLFunctions::glGetVertexAttribPointerv(GLuint index, GLenum pname, void** pointer)

    Convenience function that calls glGetVertexAttribPointerv(\a index, \a pname, \a pointer).

    For more information, see the OpenGL/ES 2.0 documentation for
    \l{http://www.khronos.org/opengles/sdk/docs/man/glGetVertexAttribPointerv.xml}{glGetVertexAttribPointerv()}.

    This convenience function will do nothing on OpenGL/ES 1.x systems.
*/

/*!
    \fn GLboolean QOpenGLFunctions::glIsBuffer(GLuint buffer)

    Convenience function that calls glIsBuffer(\a buffer).

    For more information, see the OpenGL/ES 2.0 documentation for
    \l{http://www.khronos.org/opengles/sdk/docs/man/glIsBuffer.xml}{glIsBuffer()}.
*/

/*!
    \fn GLboolean QOpenGLFunctions::glIsFramebuffer(GLuint framebuffer)

    Convenience function that calls glIsFramebuffer(\a framebuffer).

    For more information, see the OpenGL/ES 2.0 documentation for
    \l{http://www.khronos.org/opengles/sdk/docs/man/glIsFramebuffer.xml}{glIsFramebuffer()}.
*/

/*!
    \fn GLboolean QOpenGLFunctions::glIsProgram(GLuint program)

    Convenience function that calls glIsProgram(\a program).

    For more information, see the OpenGL/ES 2.0 documentation for
    \l{http://www.khronos.org/opengles/sdk/docs/man/glIsProgram.xml}{glIsProgram()}.

    This convenience function will do nothing on OpenGL/ES 1.x systems.
*/

/*!
    \fn GLboolean QOpenGLFunctions::glIsRenderbuffer(GLuint renderbuffer)

    Convenience function that calls glIsRenderbuffer(\a renderbuffer).

    For more information, see the OpenGL/ES 2.0 documentation for
    \l{http://www.khronos.org/opengles/sdk/docs/man/glIsRenderbuffer.xml}{glIsRenderbuffer()}.
*/

/*!
    \fn GLboolean QOpenGLFunctions::glIsShader(GLuint shader)

    Convenience function that calls glIsShader(\a shader).

    For more information, see the OpenGL/ES 2.0 documentation for
    \l{http://www.khronos.org/opengles/sdk/docs/man/glIsShader.xml}{glIsShader()}.

    This convenience function will do nothing on OpenGL/ES 1.x systems.
*/

/*!
    \fn void QOpenGLFunctions::glLinkProgram(GLuint program)

    Convenience function that calls glLinkProgram(\a program).

    For more information, see the OpenGL/ES 2.0 documentation for
    \l{http://www.khronos.org/opengles/sdk/docs/man/glLinkProgram.xml}{glLinkProgram()}.

    This convenience function will do nothing on OpenGL/ES 1.x systems.
*/

/*!
    \fn void QOpenGLFunctions::glReleaseShaderCompiler()

    Convenience function that calls glReleaseShaderCompiler().

    For more information, see the OpenGL/ES 2.0 documentation for
    \l{http://www.khronos.org/opengles/sdk/docs/man/glReleaseShaderCompiler.xml}{glReleaseShaderCompiler()}.

    This convenience function will do nothing on OpenGL/ES 1.x systems.
*/

/*!
    \fn void QOpenGLFunctions::glRenderbufferStorage(GLenum target, GLenum internalformat, GLsizei width, GLsizei height)

    Convenience function that calls glRenderbufferStorage(\a target, \a internalformat, \a width, \a height).

    For more information, see the OpenGL/ES 2.0 documentation for
    \l{http://www.khronos.org/opengles/sdk/docs/man/glRenderbufferStorage.xml}{glRenderbufferStorage()}.
*/

/*!
    \fn void QOpenGLFunctions::glSampleCoverage(GLclampf value, GLboolean invert)

    Convenience function that calls glSampleCoverage(\a value, \a invert).

    For more information, see the OpenGL/ES 2.0 documentation for
    \l{http://www.khronos.org/opengles/sdk/docs/man/glSampleCoverage.xml}{glSampleCoverage()}.
*/

/*!
    \fn void QOpenGLFunctions::glShaderBinary(GLint n, const GLuint* shaders, GLenum binaryformat, const void* binary, GLint length)

    Convenience function that calls glShaderBinary(\a n, \a shaders, \a binaryformat, \a binary, \a length).

    For more information, see the OpenGL/ES 2.0 documentation for
    \l{http://www.khronos.org/opengles/sdk/docs/man/glShaderBinary.xml}{glShaderBinary()}.

    This convenience function will do nothing on OpenGL/ES 1.x systems.
*/

/*!
    \fn void QOpenGLFunctions::glShaderSource(GLuint shader, GLsizei count, const char** string, const GLint* length)

    Convenience function that calls glShaderSource(\a shader, \a count, \a string, \a length).

    For more information, see the OpenGL/ES 2.0 documentation for
    \l{http://www.khronos.org/opengles/sdk/docs/man/glShaderSource.xml}{glShaderSource()}.

    This convenience function will do nothing on OpenGL/ES 1.x systems.
*/

/*!
    \fn void QOpenGLFunctions::glStencilFuncSeparate(GLenum face, GLenum func, GLint ref, GLuint mask)

    Convenience function that calls glStencilFuncSeparate(\a face, \a func, \a ref, \a mask).

    For more information, see the OpenGL/ES 2.0 documentation for
    \l{http://www.khronos.org/opengles/sdk/docs/man/glStencilFuncSeparate.xml}{glStencilFuncSeparate()}.
*/

/*!
    \fn void QOpenGLFunctions::glStencilMaskSeparate(GLenum face, GLuint mask)

    Convenience function that calls glStencilMaskSeparate(\a face, \a mask).

    For more information, see the OpenGL/ES 2.0 documentation for
    \l{http://www.khronos.org/opengles/sdk/docs/man/glStencilMaskSeparate.xml}{glStencilMaskSeparate()}.
*/

/*!
    \fn void QOpenGLFunctions::glStencilOpSeparate(GLenum face, GLenum fail, GLenum zfail, GLenum zpass)

    Convenience function that calls glStencilOpSeparate(\a face, \a fail, \a zfail, \a zpass).

    For more information, see the OpenGL/ES 2.0 documentation for
    \l{http://www.khronos.org/opengles/sdk/docs/man/glStencilOpSeparate.xml}{glStencilOpSeparate()}.
*/

/*!
    \fn void QOpenGLFunctions::glUniform1f(GLint location, GLfloat x)

    Convenience function that calls glUniform1f(\a location, \a x).

    For more information, see the OpenGL/ES 2.0 documentation for
    \l{http://www.khronos.org/opengles/sdk/docs/man/glUniform1f.xml}{glUniform1f()}.

    This convenience function will do nothing on OpenGL/ES 1.x systems.
*/

/*!
    \fn void QOpenGLFunctions::glUniform1fv(GLint location, GLsizei count, const GLfloat* v)

    Convenience function that calls glUniform1fv(\a location, \a count, \a v).

    For more information, see the OpenGL/ES 2.0 documentation for
    \l{http://www.khronos.org/opengles/sdk/docs/man/glUniform1fv.xml}{glUniform1fv()}.

    This convenience function will do nothing on OpenGL/ES 1.x systems.
*/

/*!
    \fn void QOpenGLFunctions::glUniform1i(GLint location, GLint x)

    Convenience function that calls glUniform1i(\a location, \a x).

    For more information, see the OpenGL/ES 2.0 documentation for
    \l{http://www.khronos.org/opengles/sdk/docs/man/glUniform1i.xml}{glUniform1i()}.

    This convenience function will do nothing on OpenGL/ES 1.x systems.
*/

/*!
    \fn void QOpenGLFunctions::glUniform1iv(GLint location, GLsizei count, const GLint* v)

    Convenience function that calls glUniform1iv(\a location, \a count, \a v).

    For more information, see the OpenGL/ES 2.0 documentation for
    \l{http://www.khronos.org/opengles/sdk/docs/man/glUniform1iv.xml}{glUniform1iv()}.

    This convenience function will do nothing on OpenGL/ES 1.x systems.
*/

/*!
    \fn void QOpenGLFunctions::glUniform2f(GLint location, GLfloat x, GLfloat y)

    Convenience function that calls glUniform2f(\a location, \a x, \a y).

    For more information, see the OpenGL/ES 2.0 documentation for
    \l{http://www.khronos.org/opengles/sdk/docs/man/glUniform2f.xml}{glUniform2f()}.

    This convenience function will do nothing on OpenGL/ES 1.x systems.
*/

/*!
    \fn void QOpenGLFunctions::glUniform2fv(GLint location, GLsizei count, const GLfloat* v)

    Convenience function that calls glUniform2fv(\a location, \a count, \a v).

    For more information, see the OpenGL/ES 2.0 documentation for
    \l{http://www.khronos.org/opengles/sdk/docs/man/glUniform2fv.xml}{glUniform2fv()}.

    This convenience function will do nothing on OpenGL/ES 1.x systems.
*/

/*!
    \fn void QOpenGLFunctions::glUniform2i(GLint location, GLint x, GLint y)

    Convenience function that calls glUniform2i(\a location, \a x, \a y).

    For more information, see the OpenGL/ES 2.0 documentation for
    \l{http://www.khronos.org/opengles/sdk/docs/man/glUniform2i.xml}{glUniform2i()}.

    This convenience function will do nothing on OpenGL/ES 1.x systems.
*/

/*!
    \fn void QOpenGLFunctions::glUniform2iv(GLint location, GLsizei count, const GLint* v)

    Convenience function that calls glUniform2iv(\a location, \a count, \a v).

    For more information, see the OpenGL/ES 2.0 documentation for
    \l{http://www.khronos.org/opengles/sdk/docs/man/glUniform2iv.xml}{glUniform2iv()}.

    This convenience function will do nothing on OpenGL/ES 1.x systems.
*/

/*!
    \fn void QOpenGLFunctions::glUniform3f(GLint location, GLfloat x, GLfloat y, GLfloat z)

    Convenience function that calls glUniform3f(\a location, \a x, \a y, \a z).

    For more information, see the OpenGL/ES 2.0 documentation for
    \l{http://www.khronos.org/opengles/sdk/docs/man/glUniform3f.xml}{glUniform3f()}.

    This convenience function will do nothing on OpenGL/ES 1.x systems.
*/

/*!
    \fn void QOpenGLFunctions::glUniform3fv(GLint location, GLsizei count, const GLfloat* v)

    Convenience function that calls glUniform3fv(\a location, \a count, \a v).

    For more information, see the OpenGL/ES 2.0 documentation for
    \l{http://www.khronos.org/opengles/sdk/docs/man/glUniform3fv.xml}{glUniform3fv()}.

    This convenience function will do nothing on OpenGL/ES 1.x systems.
*/

/*!
    \fn void QOpenGLFunctions::glUniform3i(GLint location, GLint x, GLint y, GLint z)

    Convenience function that calls glUniform3i(\a location, \a x, \a y, \a z).

    For more information, see the OpenGL/ES 2.0 documentation for
    \l{http://www.khronos.org/opengles/sdk/docs/man/glUniform3i.xml}{glUniform3i()}.

    This convenience function will do nothing on OpenGL/ES 1.x systems.
*/

/*!
    \fn void QOpenGLFunctions::glUniform3iv(GLint location, GLsizei count, const GLint* v)

    Convenience function that calls glUniform3iv(\a location, \a count, \a v).

    For more information, see the OpenGL/ES 2.0 documentation for
    \l{http://www.khronos.org/opengles/sdk/docs/man/glUniform3iv.xml}{glUniform3iv()}.

    This convenience function will do nothing on OpenGL/ES 1.x systems.
*/

/*!
    \fn void QOpenGLFunctions::glUniform4f(GLint location, GLfloat x, GLfloat y, GLfloat z, GLfloat w)

    Convenience function that calls glUniform4f(\a location, \a x, \a y, \a z, \a w).

    For more information, see the OpenGL/ES 2.0 documentation for
    \l{http://www.khronos.org/opengles/sdk/docs/man/glUniform4f.xml}{glUniform4f()}.

    This convenience function will do nothing on OpenGL/ES 1.x systems.
*/

/*!
    \fn void QOpenGLFunctions::glUniform4fv(GLint location, GLsizei count, const GLfloat* v)

    Convenience function that calls glUniform4fv(\a location, \a count, \a v).

    For more information, see the OpenGL/ES 2.0 documentation for
    \l{http://www.khronos.org/opengles/sdk/docs/man/glUniform4fv.xml}{glUniform4fv()}.

    This convenience function will do nothing on OpenGL/ES 1.x systems.
*/

/*!
    \fn void QOpenGLFunctions::glUniform4i(GLint location, GLint x, GLint y, GLint z, GLint w)

    Convenience function that calls glUniform4i(\a location, \a x, \a y, \a z, \a w).

    For more information, see the OpenGL/ES 2.0 documentation for
    \l{http://www.khronos.org/opengles/sdk/docs/man/glUniform4i.xml}{glUniform4i()}.

    This convenience function will do nothing on OpenGL/ES 1.x systems.
*/

/*!
    \fn void QOpenGLFunctions::glUniform4iv(GLint location, GLsizei count, const GLint* v)

    Convenience function that calls glUniform4iv(\a location, \a count, \a v).

    For more information, see the OpenGL/ES 2.0 documentation for
    \l{http://www.khronos.org/opengles/sdk/docs/man/glUniform4iv.xml}{glUniform4iv()}.

    This convenience function will do nothing on OpenGL/ES 1.x systems.
*/

/*!
    \fn void QOpenGLFunctions::glUniformMatrix2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)

    Convenience function that calls glUniformMatrix2fv(\a location, \a count, \a transpose, \a value).

    For more information, see the OpenGL/ES 2.0 documentation for
    \l{http://www.khronos.org/opengles/sdk/docs/man/glUniformMatrix2fv.xml}{glUniformMatrix2fv()}.

    This convenience function will do nothing on OpenGL/ES 1.x systems.
*/

/*!
    \fn void QOpenGLFunctions::glUniformMatrix3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)

    Convenience function that calls glUniformMatrix3fv(\a location, \a count, \a transpose, \a value).

    For more information, see the OpenGL/ES 2.0 documentation for
    \l{http://www.khronos.org/opengles/sdk/docs/man/glUniformMatrix3fv.xml}{glUniformMatrix3fv()}.

    This convenience function will do nothing on OpenGL/ES 1.x systems.
*/

/*!
    \fn void QOpenGLFunctions::glUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)

    Convenience function that calls glUniformMatrix4fv(\a location, \a count, \a transpose, \a value).

    For more information, see the OpenGL/ES 2.0 documentation for
    \l{http://www.khronos.org/opengles/sdk/docs/man/glUniformMatrix4fv.xml}{glUniformMatrix4fv()}.

    This convenience function will do nothing on OpenGL/ES 1.x systems.
*/

/*!
    \fn void QOpenGLFunctions::glUseProgram(GLuint program)

    Convenience function that calls glUseProgram(\a program).

    For more information, see the OpenGL/ES 2.0 documentation for
    \l{http://www.khronos.org/opengles/sdk/docs/man/glUseProgram.xml}{glUseProgram()}.

    This convenience function will do nothing on OpenGL/ES 1.x systems.
*/

/*!
    \fn void QOpenGLFunctions::glValidateProgram(GLuint program)

    Convenience function that calls glValidateProgram(\a program).

    For more information, see the OpenGL/ES 2.0 documentation for
    \l{http://www.khronos.org/opengles/sdk/docs/man/glValidateProgram.xml}{glValidateProgram()}.

    This convenience function will do nothing on OpenGL/ES 1.x systems.
*/

/*!
    \fn void QOpenGLFunctions::glVertexAttrib1f(GLuint indx, GLfloat x)

    Convenience function that calls glVertexAttrib1f(\a indx, \a x).

    For more information, see the OpenGL/ES 2.0 documentation for
    \l{http://www.khronos.org/opengles/sdk/docs/man/glVertexAttrib1f.xml}{glVertexAttrib1f()}.

    This convenience function will do nothing on OpenGL/ES 1.x systems.
*/

/*!
    \fn void QOpenGLFunctions::glVertexAttrib1fv(GLuint indx, const GLfloat* values)

    Convenience function that calls glVertexAttrib1fv(\a indx, \a values).

    For more information, see the OpenGL/ES 2.0 documentation for
    \l{http://www.khronos.org/opengles/sdk/docs/man/glVertexAttrib1fv.xml}{glVertexAttrib1fv()}.

    This convenience function will do nothing on OpenGL/ES 1.x systems.
*/

/*!
    \fn void QOpenGLFunctions::glVertexAttrib2f(GLuint indx, GLfloat x, GLfloat y)

    Convenience function that calls glVertexAttrib2f(\a indx, \a x, \a y).

    For more information, see the OpenGL/ES 2.0 documentation for
    \l{http://www.khronos.org/opengles/sdk/docs/man/glVertexAttrib2f.xml}{glVertexAttrib2f()}.

    This convenience function will do nothing on OpenGL/ES 1.x systems.
*/

/*!
    \fn void QOpenGLFunctions::glVertexAttrib2fv(GLuint indx, const GLfloat* values)

    Convenience function that calls glVertexAttrib2fv(\a indx, \a values).

    For more information, see the OpenGL/ES 2.0 documentation for
    \l{http://www.khronos.org/opengles/sdk/docs/man/glVertexAttrib2fv.xml}{glVertexAttrib2fv()}.

    This convenience function will do nothing on OpenGL/ES 1.x systems.
*/

/*!
    \fn void QOpenGLFunctions::glVertexAttrib3f(GLuint indx, GLfloat x, GLfloat y, GLfloat z)

    Convenience function that calls glVertexAttrib3f(\a indx, \a x, \a y, \a z).

    For more information, see the OpenGL/ES 2.0 documentation for
    \l{http://www.khronos.org/opengles/sdk/docs/man/glVertexAttrib3f.xml}{glVertexAttrib3f()}.

    This convenience function will do nothing on OpenGL/ES 1.x systems.
*/

/*!
    \fn void QOpenGLFunctions::glVertexAttrib3fv(GLuint indx, const GLfloat* values)

    Convenience function that calls glVertexAttrib3fv(\a indx, \a values).

    For more information, see the OpenGL/ES 2.0 documentation for
    \l{http://www.khronos.org/opengles/sdk/docs/man/glVertexAttrib3fv.xml}{glVertexAttrib3fv()}.

    This convenience function will do nothing on OpenGL/ES 1.x systems.
*/

/*!
    \fn void QOpenGLFunctions::glVertexAttrib4f(GLuint indx, GLfloat x, GLfloat y, GLfloat z, GLfloat w)

    Convenience function that calls glVertexAttrib4f(\a indx, \a x, \a y, \a z, \a w).

    For more information, see the OpenGL/ES 2.0 documentation for
    \l{http://www.khronos.org/opengles/sdk/docs/man/glVertexAttrib4f.xml}{glVertexAttrib4f()}.

    This convenience function will do nothing on OpenGL/ES 1.x systems.
*/

/*!
    \fn void QOpenGLFunctions::glVertexAttrib4fv(GLuint indx, const GLfloat* values)

    Convenience function that calls glVertexAttrib4fv(\a indx, \a values).

    For more information, see the OpenGL/ES 2.0 documentation for
    \l{http://www.khronos.org/opengles/sdk/docs/man/glVertexAttrib4fv.xml}{glVertexAttrib4fv()}.

    This convenience function will do nothing on OpenGL/ES 1.x systems.
*/

/*!
    \fn void QOpenGLFunctions::glVertexAttribPointer(GLuint indx, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* ptr)

    Convenience function that calls glVertexAttribPointer(\a indx, \a size, \a type, \a normalized, \a stride, \a ptr).

    For more information, see the OpenGL/ES 2.0 documentation for
    \l{http://www.khronos.org/opengles/sdk/docs/man/glVertexAttribPointer.xml}{glVertexAttribPointer()}.

    This convenience function will do nothing on OpenGL/ES 1.x systems.
*/

/*!
    \fn bool QOpenGLFunctions::isInitialized(const QOpenGLFunctionsPrivate *d)
    \internal
*/

namespace {

enum ResolvePolicy
{
    ResolveOES = 0x1,
    ResolveEXT = 0x2
};

template <typename Base, typename FuncType, int Policy, typename ReturnType>
class Resolver
{
public:
    Resolver(FuncType Base::*func, FuncType fallback, const char *name, const char *alternateName = 0)
        : funcPointerName(func)
        , fallbackFuncPointer(fallback)
        , funcName(name)
        , alternateFuncName(alternateName)
    {
    }

    ReturnType operator()();

    template <typename P1>
    ReturnType operator()(P1 p1);

    template <typename P1, typename P2>
    ReturnType operator()(P1 p1, P2 p2);

    template <typename P1, typename P2, typename P3>
    ReturnType operator()(P1 p1, P2 p2, P3 p3);

    template <typename P1, typename P2, typename P3, typename P4>
    ReturnType operator()(P1 p1, P2 p2, P3 p3, P4 p4);

    template <typename P1, typename P2, typename P3, typename P4, typename P5>
    ReturnType operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5);

    template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
    ReturnType operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6);

    template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7>
    ReturnType operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7);

    template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8>
    ReturnType operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8);

    template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9>
    ReturnType operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8, P9 p9);

    template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10>
    ReturnType operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8, P9 p9, P10 p10);

private:
    FuncType Base::*funcPointerName;
    FuncType fallbackFuncPointer;
    QByteArray funcName;
    QByteArray alternateFuncName;
};

template <typename Base, typename FuncType, int Policy>
class Resolver<Base, FuncType, Policy, void>
{
public:
    Resolver(FuncType Base::*func, FuncType fallback, const char *name, const char *alternateName = 0)
        : funcPointerName(func)
        , fallbackFuncPointer(fallback)
        , funcName(name)
        , alternateFuncName(alternateName)
    {
    }

    void operator()();

    template <typename P1>
    void operator()(P1 p1);

    template <typename P1, typename P2>
    void operator()(P1 p1, P2 p2);

    template <typename P1, typename P2, typename P3>
    void operator()(P1 p1, P2 p2, P3 p3);

    template <typename P1, typename P2, typename P3, typename P4>
    void operator()(P1 p1, P2 p2, P3 p3, P4 p4);

    template <typename P1, typename P2, typename P3, typename P4, typename P5>
    void operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5);

    template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
    void operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6);

    template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7>
    void operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7);

    template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8>
    void operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8);

    template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9>
    void operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8, P9 p9);

    template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10>
    void operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8, P9 p9, P10 p10);

private:
    FuncType Base::*funcPointerName;
    FuncType fallbackFuncPointer;
    QByteArray funcName;
    QByteArray alternateFuncName;
};

#define RESOLVER_COMMON \
    QOpenGLContext *context = QOpenGLContext::currentContext(); \
    Base *funcs = qt_gl_functions(context); \
 \
    FuncType old = funcs->*funcPointerName; \
 \
    funcs->*funcPointerName = (FuncType)context->getProcAddress(funcName); \
 \
    if ((Policy & ResolveOES) && !(funcs->*funcPointerName)) \
        funcs->*funcPointerName = (FuncType)context->getProcAddress(funcName + "OES"); \
 \
    if (!(funcs->*funcPointerName)) \
        funcs->*funcPointerName = (FuncType)context->getProcAddress(funcName + "ARB"); \
 \
    if ((Policy & ResolveEXT) && !(funcs->*funcPointerName)) \
        funcs->*funcPointerName = (FuncType)context->getProcAddress(funcName + "EXT"); \
 \
    if (!alternateFuncName.isEmpty() && !(funcs->*funcPointerName)) { \
        funcs->*funcPointerName = (FuncType)context->getProcAddress(alternateFuncName); \
 \
        if ((Policy & ResolveOES) && !(funcs->*funcPointerName)) \
            funcs->*funcPointerName = (FuncType)context->getProcAddress(alternateFuncName + "OES"); \
 \
        if (!(funcs->*funcPointerName)) \
            funcs->*funcPointerName = (FuncType)context->getProcAddress(alternateFuncName + "ARB"); \
 \
        if ((Policy & ResolveEXT) && !(funcs->*funcPointerName)) \
            funcs->*funcPointerName = (FuncType)context->getProcAddress(alternateFuncName + "EXT"); \
    }

#define RESOLVER_COMMON_NON_VOID \
    RESOLVER_COMMON \
 \
    if (!(funcs->*funcPointerName)) { \
        if (fallbackFuncPointer) { \
            funcs->*funcPointerName = fallbackFuncPointer; \
        } else { \
            funcs->*funcPointerName = old; \
            return ReturnType(); \
        } \
    }

#define RESOLVER_COMMON_VOID \
    RESOLVER_COMMON \
 \
    if (!(funcs->*funcPointerName)) { \
        if (fallbackFuncPointer) { \
            funcs->*funcPointerName = fallbackFuncPointer; \
        } else { \
            funcs->*funcPointerName = old; \
            return; \
        } \
    }

template <typename Base, typename FuncType, int Policy, typename ReturnType>
ReturnType Resolver<Base, FuncType, Policy, ReturnType>::operator()()
{
    RESOLVER_COMMON_NON_VOID

    return (funcs->*funcPointerName)();
}

template <typename Base, typename FuncType, int Policy, typename ReturnType> template <typename P1>
ReturnType Resolver<Base, FuncType, Policy, ReturnType>::operator()(P1 p1)
{
    RESOLVER_COMMON_NON_VOID

    return (funcs->*funcPointerName)(p1);
}

template <typename Base, typename FuncType, int Policy, typename ReturnType> template <typename P1, typename P2>
ReturnType Resolver<Base, FuncType, Policy, ReturnType>::operator()(P1 p1, P2 p2)
{
    RESOLVER_COMMON_NON_VOID

    return (funcs->*funcPointerName)(p1, p2);
}

template <typename Base, typename FuncType, int Policy, typename ReturnType> template <typename P1, typename P2, typename P3>
ReturnType Resolver<Base, FuncType, Policy, ReturnType>::operator()(P1 p1, P2 p2, P3 p3)
{
    RESOLVER_COMMON_NON_VOID

    return (funcs->*funcPointerName)(p1, p2, p3);
}

template <typename Base, typename FuncType, int Policy, typename ReturnType> template <typename P1, typename P2, typename P3, typename P4>
ReturnType Resolver<Base, FuncType, Policy, ReturnType>::operator()(P1 p1, P2 p2, P3 p3, P4 p4)
{
    RESOLVER_COMMON_NON_VOID

    return (funcs->*funcPointerName)(p1, p2, p3, p4);
}

template <typename Base, typename FuncType, int Policy, typename ReturnType> template <typename P1, typename P2, typename P3, typename P4, typename P5>
ReturnType Resolver<Base, FuncType, Policy, ReturnType>::operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
{
    RESOLVER_COMMON_NON_VOID

    return (funcs->*funcPointerName)(p1, p2, p3, p4, p5);
}

template <typename Base, typename FuncType, int Policy, typename ReturnType> template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
ReturnType Resolver<Base, FuncType, Policy, ReturnType>::operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6)
{
    RESOLVER_COMMON_NON_VOID

    return (funcs->*funcPointerName)(p1, p2, p3, p4, p5, p6);
}

template <typename Base, typename FuncType, int Policy, typename ReturnType> template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7>
ReturnType Resolver<Base, FuncType, Policy, ReturnType>::operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7)
{
    RESOLVER_COMMON_NON_VOID

    return (funcs->*funcPointerName)(p1, p2, p3, p4, p5, p6, p7);
}

template <typename Base, typename FuncType, int Policy, typename ReturnType> template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8>
ReturnType Resolver<Base, FuncType, Policy, ReturnType>::operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8)
{
    RESOLVER_COMMON_NON_VOID

    return (funcs->*funcPointerName)(p1, p2, p3, p4, p5, p6, p7, p8);
}

template <typename Base, typename FuncType, int Policy, typename ReturnType> template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9>
ReturnType Resolver<Base, FuncType, Policy, ReturnType>::operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8, P9 p9)
{
    RESOLVER_COMMON_NON_VOID

    return (funcs->*funcPointerName)(p1, p2, p3, p4, p5, p6, p7, p8, p9);
}

template <typename Base, typename FuncType, int Policy, typename ReturnType> template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10>
ReturnType Resolver<Base, FuncType, Policy, ReturnType>::operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8, P9 p9, P10 p10)
{
    RESOLVER_COMMON_NON_VOID

    return (funcs->*funcPointerName)(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10);
}

template <typename Base, typename FuncType, int Policy>
void Resolver<Base, FuncType, Policy, void>::operator()()
{
    RESOLVER_COMMON_VOID

    (funcs->*funcPointerName)();
}

template <typename Base, typename FuncType, int Policy> template <typename P1>
void Resolver<Base, FuncType, Policy, void>::operator()(P1 p1)
{
    RESOLVER_COMMON_VOID

    (funcs->*funcPointerName)(p1);
}

template <typename Base, typename FuncType, int Policy> template <typename P1, typename P2>
void Resolver<Base, FuncType, Policy, void>::operator()(P1 p1, P2 p2)
{
    RESOLVER_COMMON_VOID

    (funcs->*funcPointerName)(p1, p2);
}

template <typename Base, typename FuncType, int Policy> template <typename P1, typename P2, typename P3>
void Resolver<Base, FuncType, Policy, void>::operator()(P1 p1, P2 p2, P3 p3)
{
    RESOLVER_COMMON_VOID

    (funcs->*funcPointerName)(p1, p2, p3);
}

template <typename Base, typename FuncType, int Policy> template <typename P1, typename P2, typename P3, typename P4>
void Resolver<Base, FuncType, Policy, void>::operator()(P1 p1, P2 p2, P3 p3, P4 p4)
{
    RESOLVER_COMMON_VOID

    (funcs->*funcPointerName)(p1, p2, p3, p4);
}

template <typename Base, typename FuncType, int Policy> template <typename P1, typename P2, typename P3, typename P4, typename P5>
void Resolver<Base, FuncType, Policy, void>::operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
{
    RESOLVER_COMMON_VOID

    (funcs->*funcPointerName)(p1, p2, p3, p4, p5);
}

template <typename Base, typename FuncType, int Policy> template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
void Resolver<Base, FuncType, Policy, void>::operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6)
{
    RESOLVER_COMMON_VOID

    (funcs->*funcPointerName)(p1, p2, p3, p4, p5, p6);
}

template <typename Base, typename FuncType, int Policy> template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7>
void Resolver<Base, FuncType, Policy, void>::operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7)
{
    RESOLVER_COMMON_VOID

    (funcs->*funcPointerName)(p1, p2, p3, p4, p5, p6, p7);
}

template <typename Base, typename FuncType, int Policy> template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8>
void Resolver<Base, FuncType, Policy, void>::operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8)
{
    RESOLVER_COMMON_VOID

    (funcs->*funcPointerName)(p1, p2, p3, p4, p5, p6, p7, p8);
}

template <typename Base, typename FuncType, int Policy> template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9>
void Resolver<Base, FuncType, Policy, void>::operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8, P9 p9)
{
    RESOLVER_COMMON_VOID

    (funcs->*funcPointerName)(p1, p2, p3, p4, p5, p6, p7, p8, p9);
}

template <typename Base, typename FuncType, int Policy> template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10>
void Resolver<Base, FuncType, Policy, void>::operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8, P9 p9, P10 p10)
{
    RESOLVER_COMMON_VOID

    (funcs->*funcPointerName)(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10);
}

template <typename ReturnType, int Policy, typename Base, typename FuncType>
Resolver<Base, FuncType, Policy, ReturnType> functionResolverWithFallback(FuncType Base::*func, FuncType fallback, const char *name, const char *alternate = 0)
{
    return Resolver<Base, FuncType, Policy, ReturnType>(func, fallback, name, alternate);
}

template <typename ReturnType, int Policy, typename Base, typename FuncType>
Resolver<Base, FuncType, Policy, ReturnType> functionResolver(FuncType Base::*func, const char *name, const char *alternate = 0)
{
    return Resolver<Base, FuncType, Policy, ReturnType>(func, 0, name, alternate);
}

}

#define RESOLVE_FUNC(RETURN_TYPE, POLICY, NAME) \
    return functionResolver<RETURN_TYPE, POLICY>(&QOpenGLExtensionsPrivate::NAME, "gl" #NAME)

#define RESOLVE_FUNC_VOID(POLICY, NAME) \
    functionResolver<void, POLICY>(&QOpenGLExtensionsPrivate::NAME, "gl" #NAME)

#define RESOLVE_FUNC_SPECIAL(RETURN_TYPE, POLICY, NAME) \
    return functionResolverWithFallback<RETURN_TYPE, POLICY>(&QOpenGLExtensionsPrivate::NAME, qopenglfSpecial##NAME, "gl" #NAME)

#define RESOLVE_FUNC_SPECIAL_VOID(POLICY, NAME) \
    functionResolverWithFallback<void, POLICY>(&QOpenGLExtensionsPrivate::NAME, qopenglfSpecial##NAME, "gl" #NAME)

#define RESOLVE_FUNC_WITH_ALTERNATE(RETURN_TYPE, POLICY, NAME, ALTERNATE) \
    return functionResolver<RETURN_TYPE, POLICY>(&QOpenGLExtensionsPrivate::NAME, "gl" #NAME, "gl" #ALTERNATE)

#define RESOLVE_FUNC_VOID_WITH_ALTERNATE(POLICY, NAME, ALTERNATE) \
    functionResolver<void, POLICY>(&QOpenGLExtensionsPrivate::NAME, "gl" #NAME, "gl" #ALTERNATE)

#ifndef QT_OPENGL_ES_2

static void QOPENGLF_APIENTRY qopenglfResolveActiveTexture(GLenum texture)
{
    RESOLVE_FUNC_VOID(0, ActiveTexture)(texture);
}

static void QOPENGLF_APIENTRY qopenglfResolveAttachShader(GLuint program, GLuint shader)
{
    RESOLVE_FUNC_VOID_WITH_ALTERNATE(0, AttachShader, AttachObject)(program, shader);
}

static void QOPENGLF_APIENTRY qopenglfResolveBindAttribLocation(GLuint program, GLuint index, const char* name)
{
    RESOLVE_FUNC_VOID(0, BindAttribLocation)(program, index, name);
}

static void QOPENGLF_APIENTRY qopenglfResolveBindBuffer(GLenum target, GLuint buffer)
{
    RESOLVE_FUNC_VOID(ResolveOES | ResolveEXT, BindBuffer)(target, buffer);
}

static void QOPENGLF_APIENTRY qopenglfResolveBindFramebuffer(GLenum target, GLuint framebuffer)
{
    RESOLVE_FUNC_VOID(ResolveOES | ResolveEXT, BindFramebuffer)(target, framebuffer);
}

static void QOPENGLF_APIENTRY qopenglfResolveBindRenderbuffer(GLenum target, GLuint renderbuffer)
{
    RESOLVE_FUNC_VOID(ResolveOES | ResolveEXT, BindRenderbuffer)(target, renderbuffer);
}

static void QOPENGLF_APIENTRY qopenglfResolveBlendColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
{
    RESOLVE_FUNC_VOID(ResolveOES | ResolveEXT, BlendColor)(red, green, blue, alpha);
}

static void QOPENGLF_APIENTRY qopenglfResolveBlendEquation(GLenum mode)
{
    RESOLVE_FUNC_VOID(ResolveOES | ResolveEXT, BlendEquation)(mode);
}

static void QOPENGLF_APIENTRY qopenglfResolveBlendEquationSeparate(GLenum modeRGB, GLenum modeAlpha)
{
    RESOLVE_FUNC_VOID(ResolveOES | ResolveEXT, BlendEquationSeparate)(modeRGB, modeAlpha);
}

static void QOPENGLF_APIENTRY qopenglfResolveBlendFuncSeparate(GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha)
{
    RESOLVE_FUNC_VOID(ResolveOES | ResolveEXT, BlendFuncSeparate)(srcRGB, dstRGB, srcAlpha, dstAlpha);
}

static void QOPENGLF_APIENTRY qopenglfResolveBufferData(GLenum target, qopengl_GLsizeiptr size, const void* data, GLenum usage)
{
    RESOLVE_FUNC_VOID(ResolveOES | ResolveEXT, BufferData)(target, size, data, usage);
}

static void QOPENGLF_APIENTRY qopenglfResolveBufferSubData(GLenum target, qopengl_GLintptr offset, qopengl_GLsizeiptr size, const void* data)
{
    RESOLVE_FUNC_VOID(ResolveOES | ResolveEXT, BufferSubData)(target, offset, size, data);
}

static GLenum QOPENGLF_APIENTRY qopenglfResolveCheckFramebufferStatus(GLenum target)
{
    RESOLVE_FUNC(GLenum, ResolveOES | ResolveEXT, CheckFramebufferStatus)(target);
}

static void QOPENGLF_APIENTRY qopenglfResolveCompileShader(GLuint shader)
{
    RESOLVE_FUNC_VOID(0, CompileShader)(shader);
}

static void QOPENGLF_APIENTRY qopenglfResolveCompressedTexImage2D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void* data)
{
    RESOLVE_FUNC_VOID(ResolveOES | ResolveEXT, CompressedTexImage2D)(target, level, internalformat, width, height, border, imageSize, data);
}

static void QOPENGLF_APIENTRY qopenglfResolveCompressedTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void* data)
{
    RESOLVE_FUNC_VOID(ResolveOES | ResolveEXT, CompressedTexSubImage2D)(target, level, xoffset, yoffset, width, height, format, imageSize, data);
}

static GLuint QOPENGLF_APIENTRY qopenglfResolveCreateProgram()
{
    RESOLVE_FUNC_WITH_ALTERNATE(GLuint, 0, CreateProgram, CreateProgramObject)();
}

static GLuint QOPENGLF_APIENTRY qopenglfResolveCreateShader(GLenum type)
{
    RESOLVE_FUNC_WITH_ALTERNATE(GLuint, 0, CreateShader, CreateShaderObject)(type);
}

static void QOPENGLF_APIENTRY qopenglfResolveDeleteBuffers(GLsizei n, const GLuint* buffers)
{
    RESOLVE_FUNC_VOID(ResolveOES | ResolveEXT, DeleteBuffers)(n, buffers);
}

static void QOPENGLF_APIENTRY qopenglfResolveDeleteFramebuffers(GLsizei n, const GLuint* framebuffers)
{
    RESOLVE_FUNC_VOID(ResolveOES | ResolveEXT, DeleteFramebuffers)(n, framebuffers);
}

static void QOPENGLF_APIENTRY qopenglfResolveDeleteProgram(GLuint program)
{
    RESOLVE_FUNC_VOID(0, DeleteProgram)(program);
}

static void QOPENGLF_APIENTRY qopenglfResolveDeleteRenderbuffers(GLsizei n, const GLuint* renderbuffers)
{
    RESOLVE_FUNC_VOID(ResolveOES | ResolveEXT, DeleteRenderbuffers)(n, renderbuffers);
}

static void QOPENGLF_APIENTRY qopenglfResolveDeleteShader(GLuint shader)
{
    RESOLVE_FUNC_VOID_WITH_ALTERNATE(0, DeleteShader, DeleteObject)(shader);
}

static void QOPENGLF_APIENTRY qopenglfResolveDetachShader(GLuint program, GLuint shader)
{
    RESOLVE_FUNC_VOID_WITH_ALTERNATE(0, DetachShader, DetachObject)(program, shader);
}

static void QOPENGLF_APIENTRY qopenglfResolveDisableVertexAttribArray(GLuint index)
{
    RESOLVE_FUNC_VOID(0, DisableVertexAttribArray)(index);
}

static void QOPENGLF_APIENTRY qopenglfResolveEnableVertexAttribArray(GLuint index)
{
    RESOLVE_FUNC_VOID(0, EnableVertexAttribArray)(index);
}

static void QOPENGLF_APIENTRY qopenglfResolveFramebufferRenderbuffer(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)
{
    RESOLVE_FUNC_VOID(ResolveOES | ResolveEXT, FramebufferRenderbuffer)(target, attachment, renderbuffertarget, renderbuffer);
}

static void QOPENGLF_APIENTRY qopenglfResolveFramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
    RESOLVE_FUNC_VOID(ResolveOES | ResolveEXT, FramebufferTexture2D)(target, attachment, textarget, texture, level);
}

static void QOPENGLF_APIENTRY qopenglfResolveGenBuffers(GLsizei n, GLuint* buffers)
{
    RESOLVE_FUNC_VOID(ResolveOES | ResolveEXT, GenBuffers)(n, buffers);
}

static void QOPENGLF_APIENTRY qopenglfResolveGenerateMipmap(GLenum target)
{
    RESOLVE_FUNC_VOID(ResolveOES | ResolveEXT, GenerateMipmap)(target);
}

static void QOPENGLF_APIENTRY qopenglfResolveGenFramebuffers(GLsizei n, GLuint* framebuffers)
{
    RESOLVE_FUNC_VOID(ResolveOES | ResolveEXT, GenFramebuffers)(n, framebuffers);
}

static void QOPENGLF_APIENTRY qopenglfResolveGenRenderbuffers(GLsizei n, GLuint* renderbuffers)
{
    RESOLVE_FUNC_VOID(ResolveOES | ResolveEXT, GenRenderbuffers)(n, renderbuffers);
}

static void QOPENGLF_APIENTRY qopenglfResolveGetActiveAttrib(GLuint program, GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, char* name)
{
    RESOLVE_FUNC_VOID(0, GetActiveAttrib)(program, index, bufsize, length, size, type, name);
}

static void QOPENGLF_APIENTRY qopenglfResolveGetActiveUniform(GLuint program, GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, char* name)
{
    RESOLVE_FUNC_VOID(0, GetActiveUniform)(program, index, bufsize, length, size, type, name);
}

static void QOPENGLF_APIENTRY qopenglfResolveGetAttachedShaders(GLuint program, GLsizei maxcount, GLsizei* count, GLuint* shaders)
{
    RESOLVE_FUNC_VOID_WITH_ALTERNATE(0, GetAttachedShaders, GetAttachedObjects)(program, maxcount, count, shaders);
}

static GLint QOPENGLF_APIENTRY qopenglfResolveGetAttribLocation(GLuint program, const char* name)
{
    RESOLVE_FUNC(GLint, 0, GetAttribLocation)(program, name);
}

static void QOPENGLF_APIENTRY qopenglfResolveGetBufferParameteriv(GLenum target, GLenum pname, GLint* params)
{
    RESOLVE_FUNC_VOID(ResolveOES | ResolveEXT, GetBufferParameteriv)(target, pname, params);
}

static void QOPENGLF_APIENTRY qopenglfResolveGetFramebufferAttachmentParameteriv(GLenum target, GLenum attachment, GLenum pname, GLint* params)
{
    RESOLVE_FUNC_VOID(ResolveOES | ResolveEXT, GetFramebufferAttachmentParameteriv)(target, attachment, pname, params);
}

static void QOPENGLF_APIENTRY qopenglfResolveGetProgramiv(GLuint program, GLenum pname, GLint* params)
{
    RESOLVE_FUNC_VOID_WITH_ALTERNATE(0, GetProgramiv, GetObjectParameteriv)(program, pname, params);
}

static void QOPENGLF_APIENTRY qopenglfResolveGetProgramInfoLog(GLuint program, GLsizei bufsize, GLsizei* length, char* infolog)
{
    RESOLVE_FUNC_VOID_WITH_ALTERNATE(0, GetProgramInfoLog, GetInfoLog)(program, bufsize, length, infolog);
}

static void QOPENGLF_APIENTRY qopenglfResolveGetRenderbufferParameteriv(GLenum target, GLenum pname, GLint* params)
{
    RESOLVE_FUNC_VOID(ResolveOES | ResolveEXT, GetRenderbufferParameteriv)(target, pname, params);
}

static void QOPENGLF_APIENTRY qopenglfResolveGetShaderiv(GLuint shader, GLenum pname, GLint* params)
{
    RESOLVE_FUNC_VOID_WITH_ALTERNATE(0, GetShaderiv, GetObjectParameteriv)(shader, pname, params);
}

static void QOPENGLF_APIENTRY qopenglfResolveGetShaderInfoLog(GLuint shader, GLsizei bufsize, GLsizei* length, char* infolog)
{
    RESOLVE_FUNC_VOID_WITH_ALTERNATE(0, GetShaderInfoLog, GetInfoLog)(shader, bufsize, length, infolog);
}

static void QOPENGLF_APIENTRY qopenglfSpecialGetShaderPrecisionFormat(GLenum shadertype, GLenum precisiontype, GLint* range, GLint* precision)
{
    Q_UNUSED(shadertype);
    Q_UNUSED(precisiontype);
    range[0] = range[1] = precision[0] = 0;
}

static void QOPENGLF_APIENTRY qopenglfResolveGetShaderPrecisionFormat(GLenum shadertype, GLenum precisiontype, GLint* range, GLint* precision)
{
    RESOLVE_FUNC_SPECIAL_VOID(ResolveOES | ResolveEXT, GetShaderPrecisionFormat)(shadertype, precisiontype, range, precision);
}

static void QOPENGLF_APIENTRY qopenglfResolveGetShaderSource(GLuint shader, GLsizei bufsize, GLsizei* length, char* source)
{
    RESOLVE_FUNC_VOID(0, GetShaderSource)(shader, bufsize, length, source);
}

static void QOPENGLF_APIENTRY qopenglfResolveGetUniformfv(GLuint program, GLint location, GLfloat* params)
{
    RESOLVE_FUNC_VOID(0, GetUniformfv)(program, location, params);
}

static void QOPENGLF_APIENTRY qopenglfResolveGetUniformiv(GLuint program, GLint location, GLint* params)
{
    RESOLVE_FUNC_VOID(0, GetUniformiv)(program, location, params);
}

static GLint QOPENGLF_APIENTRY qopenglfResolveGetUniformLocation(GLuint program, const char* name)
{
    RESOLVE_FUNC(GLint, 0, GetUniformLocation)(program, name);
}

static void QOPENGLF_APIENTRY qopenglfResolveGetVertexAttribfv(GLuint index, GLenum pname, GLfloat* params)
{
    RESOLVE_FUNC_VOID(0, GetVertexAttribfv)(index, pname, params);
}

static void QOPENGLF_APIENTRY qopenglfResolveGetVertexAttribiv(GLuint index, GLenum pname, GLint* params)
{
    RESOLVE_FUNC_VOID(0, GetVertexAttribiv)(index, pname, params);
}

static void QOPENGLF_APIENTRY qopenglfResolveGetVertexAttribPointerv(GLuint index, GLenum pname, void** pointer)
{
    RESOLVE_FUNC_VOID(0, GetVertexAttribPointerv)(index, pname, pointer);
}

static GLboolean QOPENGLF_APIENTRY qopenglfResolveIsBuffer(GLuint buffer)
{
    RESOLVE_FUNC(GLboolean, ResolveOES | ResolveEXT, IsBuffer)(buffer);
}

static GLboolean QOPENGLF_APIENTRY qopenglfResolveIsFramebuffer(GLuint framebuffer)
{
    RESOLVE_FUNC(GLboolean, ResolveOES | ResolveEXT, IsFramebuffer)(framebuffer);
}

static GLboolean QOPENGLF_APIENTRY qopenglfSpecialIsProgram(GLuint program)
{
    return program != 0;
}

static GLboolean QOPENGLF_APIENTRY qopenglfResolveIsProgram(GLuint program)
{
    RESOLVE_FUNC_SPECIAL(GLboolean, 0, IsProgram)(program);
}

static GLboolean QOPENGLF_APIENTRY qopenglfResolveIsRenderbuffer(GLuint renderbuffer)
{
    RESOLVE_FUNC(GLboolean, ResolveOES | ResolveEXT, IsRenderbuffer)(renderbuffer);
}

static GLboolean QOPENGLF_APIENTRY qopenglfSpecialIsShader(GLuint shader)
{
    return shader != 0;
}

static GLboolean QOPENGLF_APIENTRY qopenglfResolveIsShader(GLuint shader)
{
    RESOLVE_FUNC_SPECIAL(GLboolean, 0, IsShader)(shader);
}

static void QOPENGLF_APIENTRY qopenglfResolveLinkProgram(GLuint program)
{
    RESOLVE_FUNC_VOID(0, LinkProgram)(program);
}

static void QOPENGLF_APIENTRY qopenglfSpecialReleaseShaderCompiler()
{
}

static void QOPENGLF_APIENTRY qopenglfResolveReleaseShaderCompiler()
{
    RESOLVE_FUNC_SPECIAL_VOID(0, ReleaseShaderCompiler)();
}

static void QOPENGLF_APIENTRY qopenglfResolveRenderbufferStorage(GLenum target, GLenum internalformat, GLsizei width, GLsizei height)
{
    RESOLVE_FUNC_VOID(ResolveOES | ResolveEXT, RenderbufferStorage)(target, internalformat, width, height);
}

static void QOPENGLF_APIENTRY qopenglfResolveSampleCoverage(GLclampf value, GLboolean invert)
{
    RESOLVE_FUNC_VOID(ResolveOES | ResolveEXT, SampleCoverage)(value, invert);
}

static void QOPENGLF_APIENTRY qopenglfResolveShaderBinary(GLint n, const GLuint* shaders, GLenum binaryformat, const void* binary, GLint length)
{
    RESOLVE_FUNC_VOID(0, ShaderBinary)(n, shaders, binaryformat, binary, length);
}

static void QOPENGLF_APIENTRY qopenglfResolveShaderSource(GLuint shader, GLsizei count, const char** string, const GLint* length)
{
    RESOLVE_FUNC_VOID(0, ShaderSource)(shader, count, string, length);
}

static void QOPENGLF_APIENTRY qopenglfResolveStencilFuncSeparate(GLenum face, GLenum func, GLint ref, GLuint mask)
{
    RESOLVE_FUNC_VOID(ResolveEXT, StencilFuncSeparate)(face, func, ref, mask);
}

static void QOPENGLF_APIENTRY qopenglfResolveStencilMaskSeparate(GLenum face, GLuint mask)
{
    RESOLVE_FUNC_VOID(ResolveEXT, StencilMaskSeparate)(face, mask);
}

static void QOPENGLF_APIENTRY qopenglfResolveStencilOpSeparate(GLenum face, GLenum fail, GLenum zfail, GLenum zpass)
{
    RESOLVE_FUNC_VOID(ResolveEXT, StencilOpSeparate)(face, fail, zfail, zpass);
}

static void QOPENGLF_APIENTRY qopenglfResolveUniform1f(GLint location, GLfloat x)
{
    RESOLVE_FUNC_VOID(0, Uniform1f)(location, x);
}

static void QOPENGLF_APIENTRY qopenglfResolveUniform1fv(GLint location, GLsizei count, const GLfloat* v)
{
    RESOLVE_FUNC_VOID(0, Uniform1fv)(location, count, v);
}

static void QOPENGLF_APIENTRY qopenglfResolveUniform1i(GLint location, GLint x)
{
    RESOLVE_FUNC_VOID(0, Uniform1i)(location, x);
}

static void QOPENGLF_APIENTRY qopenglfResolveUniform1iv(GLint location, GLsizei count, const GLint* v)
{
    RESOLVE_FUNC_VOID(0, Uniform1iv)(location, count, v);
}

static void QOPENGLF_APIENTRY qopenglfResolveUniform2f(GLint location, GLfloat x, GLfloat y)
{
    RESOLVE_FUNC_VOID(0, Uniform2f)(location, x, y);
}

static void QOPENGLF_APIENTRY qopenglfResolveUniform2fv(GLint location, GLsizei count, const GLfloat* v)
{
    RESOLVE_FUNC_VOID(0, Uniform2fv)(location, count, v);
}

static void QOPENGLF_APIENTRY qopenglfResolveUniform2i(GLint location, GLint x, GLint y)
{
    RESOLVE_FUNC_VOID(0, Uniform2i)(location, x, y);
}

static void QOPENGLF_APIENTRY qopenglfResolveUniform2iv(GLint location, GLsizei count, const GLint* v)
{
    RESOLVE_FUNC_VOID(0, Uniform2iv)(location, count, v);
}

static void QOPENGLF_APIENTRY qopenglfResolveUniform3f(GLint location, GLfloat x, GLfloat y, GLfloat z)
{
    RESOLVE_FUNC_VOID(0, Uniform3f)(location, x, y, z);
}

static void QOPENGLF_APIENTRY qopenglfResolveUniform3fv(GLint location, GLsizei count, const GLfloat* v)
{
    RESOLVE_FUNC_VOID(0, Uniform3fv)(location, count, v);
}

static void QOPENGLF_APIENTRY qopenglfResolveUniform3i(GLint location, GLint x, GLint y, GLint z)
{
    RESOLVE_FUNC_VOID(0, Uniform3i)(location, x, y, z);
}

static void QOPENGLF_APIENTRY qopenglfResolveUniform3iv(GLint location, GLsizei count, const GLint* v)
{
    RESOLVE_FUNC_VOID(0, Uniform3iv)(location, count, v);
}

static void QOPENGLF_APIENTRY qopenglfResolveUniform4f(GLint location, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    RESOLVE_FUNC_VOID(0, Uniform4f)(location, x, y, z, w);
}

static void QOPENGLF_APIENTRY qopenglfResolveUniform4fv(GLint location, GLsizei count, const GLfloat* v)
{
    RESOLVE_FUNC_VOID(0, Uniform4fv)(location, count, v);
}

static void QOPENGLF_APIENTRY qopenglfResolveUniform4i(GLint location, GLint x, GLint y, GLint z, GLint w)
{
    RESOLVE_FUNC_VOID(0, Uniform4i)(location, x, y, z, w);
}

static void QOPENGLF_APIENTRY qopenglfResolveUniform4iv(GLint location, GLsizei count, const GLint* v)
{
    RESOLVE_FUNC_VOID(0, Uniform4iv)(location, count, v);
}

static void QOPENGLF_APIENTRY qopenglfResolveUniformMatrix2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    RESOLVE_FUNC_VOID(0, UniformMatrix2fv)(location, count, transpose, value);
}

static void QOPENGLF_APIENTRY qopenglfResolveUniformMatrix3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    RESOLVE_FUNC_VOID(0, UniformMatrix3fv)(location, count, transpose, value);
}

static void QOPENGLF_APIENTRY qopenglfResolveUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    RESOLVE_FUNC_VOID(0, UniformMatrix4fv)(location, count, transpose, value);
}

static void QOPENGLF_APIENTRY qopenglfResolveUseProgram(GLuint program)
{
    RESOLVE_FUNC_VOID(0, UseProgram)(program);
}

static void QOPENGLF_APIENTRY qopenglfResolveValidateProgram(GLuint program)
{
    RESOLVE_FUNC_VOID(0, ValidateProgram)(program);
}

static void QOPENGLF_APIENTRY qopenglfResolveVertexAttrib1f(GLuint indx, GLfloat x)
{
    RESOLVE_FUNC_VOID(0, VertexAttrib1f)(indx, x);
}

static void QOPENGLF_APIENTRY qopenglfResolveVertexAttrib1fv(GLuint indx, const GLfloat* values)
{
    RESOLVE_FUNC_VOID(0, VertexAttrib1fv)(indx, values);
}

static void QOPENGLF_APIENTRY qopenglfResolveVertexAttrib2f(GLuint indx, GLfloat x, GLfloat y)
{
    RESOLVE_FUNC_VOID(0, VertexAttrib2f)(indx, x, y);
}

static void QOPENGLF_APIENTRY qopenglfResolveVertexAttrib2fv(GLuint indx, const GLfloat* values)
{
    RESOLVE_FUNC_VOID(0, VertexAttrib2fv)(indx, values);
}

static void QOPENGLF_APIENTRY qopenglfResolveVertexAttrib3f(GLuint indx, GLfloat x, GLfloat y, GLfloat z)
{
    RESOLVE_FUNC_VOID(0, VertexAttrib3f)(indx, x, y, z);
}

static void QOPENGLF_APIENTRY qopenglfResolveVertexAttrib3fv(GLuint indx, const GLfloat* values)
{
    RESOLVE_FUNC_VOID(0, VertexAttrib3fv)(indx, values);
}

static void QOPENGLF_APIENTRY qopenglfResolveVertexAttrib4f(GLuint indx, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    RESOLVE_FUNC_VOID(0, VertexAttrib4f)(indx, x, y, z, w);
}

static void QOPENGLF_APIENTRY qopenglfResolveVertexAttrib4fv(GLuint indx, const GLfloat* values)
{
    RESOLVE_FUNC_VOID(0, VertexAttrib4fv)(indx, values);
}

static void QOPENGLF_APIENTRY qopenglfResolveVertexAttribPointer(GLuint indx, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* ptr)
{
    RESOLVE_FUNC_VOID(0, VertexAttribPointer)(indx, size, type, normalized, stride, ptr);
}

#endif // !QT_OPENGL_ES_2

static GLvoid *QOPENGLF_APIENTRY qopenglfResolveMapBuffer(GLenum target, GLenum access)
{
    RESOLVE_FUNC(GLvoid *, ResolveOES, MapBuffer)(target, access);
}

static GLboolean QOPENGLF_APIENTRY qopenglfResolveUnmapBuffer(GLenum target)
{
    RESOLVE_FUNC(GLboolean, ResolveOES, UnmapBuffer)(target);
}

static void QOPENGLF_APIENTRY qopenglfResolveBlitFramebuffer(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1,
                       GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1,
                       GLbitfield mask, GLenum filter)
{
    RESOLVE_FUNC_VOID(ResolveEXT, BlitFramebuffer)
        (srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter);
}

static void QOPENGLF_APIENTRY qopenglfResolveRenderbufferStorageMultisample(GLenum target, GLsizei samples,
                                      GLenum internalFormat,
                                      GLsizei width, GLsizei height)
{
    RESOLVE_FUNC_VOID(ResolveEXT, RenderbufferStorageMultisample)
        (target, samples, internalFormat, width, height);
}

static void QOPENGLF_APIENTRY qopenglfResolveGetBufferSubData(GLenum target, qopengl_GLintptr offset, qopengl_GLsizeiptr size, GLvoid *data)
{
    RESOLVE_FUNC_VOID(ResolveEXT, GetBufferSubData)
        (target, offset, size, data);
}

QOpenGLFunctionsPrivate::QOpenGLFunctionsPrivate(QOpenGLContext *)
{
    /* Assign a pointer to an above defined static function
     * which on first call resolves the function from the current
     * context, assigns it to the member variable and executes it
     * (see Resolver template) */
#ifndef QT_OPENGL_ES_2
    ActiveTexture = qopenglfResolveActiveTexture;
    AttachShader = qopenglfResolveAttachShader;
    BindAttribLocation = qopenglfResolveBindAttribLocation;
    BindBuffer = qopenglfResolveBindBuffer;
    BindFramebuffer = qopenglfResolveBindFramebuffer;
    BindRenderbuffer = qopenglfResolveBindRenderbuffer;
    BlendColor = qopenglfResolveBlendColor;
    BlendEquation = qopenglfResolveBlendEquation;
    BlendEquationSeparate = qopenglfResolveBlendEquationSeparate;
    BlendFuncSeparate = qopenglfResolveBlendFuncSeparate;
    BufferData = qopenglfResolveBufferData;
    BufferSubData = qopenglfResolveBufferSubData;
    CheckFramebufferStatus = qopenglfResolveCheckFramebufferStatus;
    CompileShader = qopenglfResolveCompileShader;
    CompressedTexImage2D = qopenglfResolveCompressedTexImage2D;
    CompressedTexSubImage2D = qopenglfResolveCompressedTexSubImage2D;
    CreateProgram = qopenglfResolveCreateProgram;
    CreateShader = qopenglfResolveCreateShader;
    DeleteBuffers = qopenglfResolveDeleteBuffers;
    DeleteFramebuffers = qopenglfResolveDeleteFramebuffers;
    DeleteProgram = qopenglfResolveDeleteProgram;
    DeleteRenderbuffers = qopenglfResolveDeleteRenderbuffers;
    DeleteShader = qopenglfResolveDeleteShader;
    DetachShader = qopenglfResolveDetachShader;
    DisableVertexAttribArray = qopenglfResolveDisableVertexAttribArray;
    EnableVertexAttribArray = qopenglfResolveEnableVertexAttribArray;
    FramebufferRenderbuffer = qopenglfResolveFramebufferRenderbuffer;
    FramebufferTexture2D = qopenglfResolveFramebufferTexture2D;
    GenBuffers = qopenglfResolveGenBuffers;
    GenerateMipmap = qopenglfResolveGenerateMipmap;
    GenFramebuffers = qopenglfResolveGenFramebuffers;
    GenRenderbuffers = qopenglfResolveGenRenderbuffers;
    GetActiveAttrib = qopenglfResolveGetActiveAttrib;
    GetActiveUniform = qopenglfResolveGetActiveUniform;
    GetAttachedShaders = qopenglfResolveGetAttachedShaders;
    GetAttribLocation = qopenglfResolveGetAttribLocation;
    GetBufferParameteriv = qopenglfResolveGetBufferParameteriv;
    GetFramebufferAttachmentParameteriv = qopenglfResolveGetFramebufferAttachmentParameteriv;
    GetProgramiv = qopenglfResolveGetProgramiv;
    GetProgramInfoLog = qopenglfResolveGetProgramInfoLog;
    GetRenderbufferParameteriv = qopenglfResolveGetRenderbufferParameteriv;
    GetShaderiv = qopenglfResolveGetShaderiv;
    GetShaderInfoLog = qopenglfResolveGetShaderInfoLog;
    GetShaderPrecisionFormat = qopenglfResolveGetShaderPrecisionFormat;
    GetShaderSource = qopenglfResolveGetShaderSource;
    GetUniformfv = qopenglfResolveGetUniformfv;
    GetUniformiv = qopenglfResolveGetUniformiv;
    GetUniformLocation = qopenglfResolveGetUniformLocation;
    GetVertexAttribfv = qopenglfResolveGetVertexAttribfv;
    GetVertexAttribiv = qopenglfResolveGetVertexAttribiv;
    GetVertexAttribPointerv = qopenglfResolveGetVertexAttribPointerv;
    IsBuffer = qopenglfResolveIsBuffer;
    IsFramebuffer = qopenglfResolveIsFramebuffer;
    IsProgram = qopenglfResolveIsProgram;
    IsRenderbuffer = qopenglfResolveIsRenderbuffer;
    IsShader = qopenglfResolveIsShader;
    LinkProgram = qopenglfResolveLinkProgram;
    ReleaseShaderCompiler = qopenglfResolveReleaseShaderCompiler;
    RenderbufferStorage = qopenglfResolveRenderbufferStorage;
    SampleCoverage = qopenglfResolveSampleCoverage;
    ShaderBinary = qopenglfResolveShaderBinary;
    ShaderSource = qopenglfResolveShaderSource;
    StencilFuncSeparate = qopenglfResolveStencilFuncSeparate;
    StencilMaskSeparate = qopenglfResolveStencilMaskSeparate;
    StencilOpSeparate = qopenglfResolveStencilOpSeparate;
    Uniform1f = qopenglfResolveUniform1f;
    Uniform1fv = qopenglfResolveUniform1fv;
    Uniform1i = qopenglfResolveUniform1i;
    Uniform1iv = qopenglfResolveUniform1iv;
    Uniform2f = qopenglfResolveUniform2f;
    Uniform2fv = qopenglfResolveUniform2fv;
    Uniform2i = qopenglfResolveUniform2i;
    Uniform2iv = qopenglfResolveUniform2iv;
    Uniform3f = qopenglfResolveUniform3f;
    Uniform3fv = qopenglfResolveUniform3fv;
    Uniform3i = qopenglfResolveUniform3i;
    Uniform3iv = qopenglfResolveUniform3iv;
    Uniform4f = qopenglfResolveUniform4f;
    Uniform4fv = qopenglfResolveUniform4fv;
    Uniform4i = qopenglfResolveUniform4i;
    Uniform4iv = qopenglfResolveUniform4iv;
    UniformMatrix2fv = qopenglfResolveUniformMatrix2fv;
    UniformMatrix3fv = qopenglfResolveUniformMatrix3fv;
    UniformMatrix4fv = qopenglfResolveUniformMatrix4fv;
    UseProgram = qopenglfResolveUseProgram;
    ValidateProgram = qopenglfResolveValidateProgram;
    VertexAttrib1f = qopenglfResolveVertexAttrib1f;
    VertexAttrib1fv = qopenglfResolveVertexAttrib1fv;
    VertexAttrib2f = qopenglfResolveVertexAttrib2f;
    VertexAttrib2fv = qopenglfResolveVertexAttrib2fv;
    VertexAttrib3f = qopenglfResolveVertexAttrib3f;
    VertexAttrib3fv = qopenglfResolveVertexAttrib3fv;
    VertexAttrib4f = qopenglfResolveVertexAttrib4f;
    VertexAttrib4fv = qopenglfResolveVertexAttrib4fv;
    VertexAttribPointer = qopenglfResolveVertexAttribPointer;
#endif // !QT_OPENGL_ES_2
}

QOpenGLExtensionsPrivate::QOpenGLExtensionsPrivate(QOpenGLContext *ctx)
    : QOpenGLFunctionsPrivate(ctx)
{
    MapBuffer = qopenglfResolveMapBuffer;
    UnmapBuffer = qopenglfResolveUnmapBuffer;
    BlitFramebuffer = qopenglfResolveBlitFramebuffer;
    RenderbufferStorageMultisample = qopenglfResolveRenderbufferStorageMultisample;
    GetBufferSubData = qopenglfResolveGetBufferSubData;
}

#if defined(QT_OPENGL_DYNAMIC)
extern int qgl_proxyLibraryType(void);
extern HMODULE qgl_glHandle(void);
#endif

/*!
    \enum QOpenGLFunctions::PlatformGLType
    This enum defines the type of the underlying GL implementation.

    \value DesktopGL Desktop OpenGL
    \value GLES2 OpenGL ES 2.0 or higher
    \value GLES1 OpenGL ES 1.x

    \since 5.3
 */

/*!
  \fn QOpenGLFunctions::isES()

  On platforms where the OpenGL implementation is dynamically loaded
  this function returns true if the underlying GL implementation is
  Open GL ES.

  On platforms that do not use runtime loading of the GL the return
  value is based on Qt's compile-time configuration and will never
  change during runtime.

  \sa platformGLType()

  \since 5.3
  */

/*!
  Returns the underlying GL implementation type.

  On platforms where the OpenGL implementation is not dynamically
  loaded, the return value is determined during compile time and never
  changes.

  Platforms that use dynamic GL loading (e.g. Windows) cannot rely on
  compile-time defines for differentiating between desktop and ES
  OpenGL code. Instead, they rely on this function to query, during
  runtime, the type of the loaded graphics library.

  \since 5.3
 */
QOpenGLFunctions::PlatformGLType QOpenGLFunctions::platformGLType()
{
#if defined(QT_OPENGL_DYNAMIC)
    return PlatformGLType(qgl_proxyLibraryType());
#elif defined(QT_OPENGL_ES_2)
    return GLES2;
#elif defined(QT_OPENGL_ES)
    return GLES1;
#else
    return DesktopGL;
#endif
}

/*!
  Returns the platform-specific handle for the OpenGL implementation that
  is currently in use. (for example, a HMODULE on Windows)

  On platforms that do not use dynamic GL switch the return value is null.

  The library might be GL-only, meaning that windowing system interface
  functions (for example EGL) may live in another, separate library.

  Always use platformGLType() before resolving any functions to check if the
  library implements desktop OpenGL or OpenGL ES.

  \sa platformGLType()

  \since 5.3
 */
void *QOpenGLFunctions::platformGLHandle()
{
#if defined(QT_OPENGL_DYNAMIC)
    return qgl_glHandle();
#else
    return 0;
#endif
}

QT_END_NAMESPACE
