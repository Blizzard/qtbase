/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the plugins of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL21$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file. Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** As a special exception, The Qt Company gives you certain additional
** rights. These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtCore/qtextstream.h>
#include <QtGui/qpa/qplatformcursor.h>

#include "qeglfsscreen.h"
#include "qeglfswindow.h"
#include "qeglfshooks.h"

QT_BEGIN_NAMESPACE

QEglFSScreen::QEglFSScreen(EGLDisplay dpy)
    : QEGLPlatformScreen(dpy),
      m_surface(EGL_NO_SURFACE),
      m_cursor(0)
{
    m_cursor = qt_egl_device_integration()->createCursor(this);
}

QEglFSScreen::~QEglFSScreen()
{
    delete m_cursor;
}

QRect QEglFSScreen::geometry() const
{
    return QRect(QPoint(0, 0), qt_egl_device_integration()->screenSize());
}

int QEglFSScreen::depth() const
{
    return qt_egl_device_integration()->screenDepth();
}

QImage::Format QEglFSScreen::format() const
{
    return qt_egl_device_integration()->screenFormat();
}

QSizeF QEglFSScreen::physicalSize() const
{
    return qt_egl_device_integration()->physicalScreenSize();
}

QDpi QEglFSScreen::logicalDpi() const
{
    return qt_egl_device_integration()->logicalDpi();
}

Qt::ScreenOrientation QEglFSScreen::nativeOrientation() const
{
    return qt_egl_device_integration()->nativeOrientation();
}

Qt::ScreenOrientation QEglFSScreen::orientation() const
{
    return qt_egl_device_integration()->orientation();
}

QPlatformCursor *QEglFSScreen::cursor() const
{
    return m_cursor;
}

qreal QEglFSScreen::refreshRate() const
{
    return qt_egl_device_integration()->refreshRate();
}

void QEglFSScreen::setPrimarySurface(EGLSurface surface)
{
    m_surface = surface;
}

QT_END_NAMESPACE
