/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the plugins of the Qt Toolkit.
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

#include "qcocoabackingstore.h"
#include <QtGui/QPainter>
#include "qcocoahelpers.h"

QT_BEGIN_NAMESPACE

QCocoaBackingStore::QCocoaBackingStore(QWindow *window)
    : QPlatformBackingStore(window)
    , m_cgImage(0)
{
}

QCocoaBackingStore::~QCocoaBackingStore()
{
    CGImageRelease(m_cgImage);
    m_cgImage = 0;
}

QPaintDevice *QCocoaBackingStore::paintDevice()
{
    if (m_qImage.size() / m_qImage.devicePixelRatio() != m_requestedSize) {
        CGImageRelease(m_cgImage);
        m_cgImage = 0;

        int scaleFactor = 1;
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_7
        if (QSysInfo::MacintoshVersion >= QSysInfo::MV_10_7) {
            QCocoaWindow *cocoaWindow = static_cast<QCocoaWindow *>(window()->handle());
            if (cocoaWindow && cocoaWindow->m_contentView && [cocoaWindow->m_contentView window]) {
                scaleFactor = int([[cocoaWindow->m_contentView window] backingScaleFactor]);
            }
        }
#endif

        QCocoaWindow *cocoaWindow = static_cast<QCocoaWindow *>(window()->handle());
        QImage::Format format = (window()->format().hasAlpha() || cocoaWindow->m_drawContentBorderGradient)
                ? QImage::Format_ARGB32_Premultiplied : QImage::Format_RGB32;
        m_qImage = QImage(m_requestedSize * scaleFactor, format);
        m_qImage.setDevicePixelRatio(scaleFactor);
        if (format == QImage::Format_ARGB32_Premultiplied)
            m_qImage.fill(Qt::transparent);
    }
    return &m_qImage;
}

void QCocoaBackingStore::flush(QWindow *win, const QRegion &region, const QPoint &offset)
{
    // A flush means that qImage has changed. Since CGImages are seen as
    // immutable, CoreImage fails to pick up this change for m_cgImage
    // (since it usually cached), so we must recreate it. We await doing this
    // until one of the views needs it, since, together with calling
    // "setNeedsDisplayInRect" instead of "displayRect" we will, in most
    // cases, get away with doing this once for every repaint. Also note that
    // m_cgImage is only a reference to the data inside m_qImage, it is not a copy.
    CGImageRelease(m_cgImage);
    m_cgImage = 0;
    if (!m_qImage.isNull()) {
        if (QCocoaWindow *cocoaWindow = static_cast<QCocoaWindow *>(win->handle()))
            [cocoaWindow->m_qtView flushBackingStore:this region:region offset:offset];
    }
}

void QCocoaBackingStore::resize(const QSize &size, const QRegion &)
{
    m_requestedSize = size;
}

bool QCocoaBackingStore::scroll(const QRegion &area, int dx, int dy)
{
    extern void qt_scrollRectInImage(QImage &img, const QRect &rect, const QPoint &offset);
    const qreal devicePixelRatio = m_qImage.devicePixelRatio();
    QPoint qpoint(dx * devicePixelRatio, dy * devicePixelRatio);
    const QVector<QRect> qrects = area.rects();
    for (int i = 0; i < qrects.count(); ++i) {
        const QRect &qrect = QRect(qrects.at(i).topLeft() * devicePixelRatio, qrects.at(i).size() * devicePixelRatio);
        qt_scrollRectInImage(m_qImage, qrect, qpoint);
    }
    return true;
}

CGImageRef QCocoaBackingStore::getBackingStoreCGImage()
{
    if (!m_cgImage)
        m_cgImage = qt_mac_toCGImage(m_qImage);

    // Warning: do not retain/release/cache the returned image from
    // outside the backingstore since it shares data with a QImage and
    // needs special memory considerations.
    return m_cgImage;
}

qreal QCocoaBackingStore::getBackingStoreDevicePixelRatio()
{
    return m_qImage.devicePixelRatio();
}

QT_END_NAMESPACE
