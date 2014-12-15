/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtGui module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL21$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia. For licensing terms and
** conditions see http://qt.digia.com/licensing. For further information
** use the contact form at http://qt.digia.com/contact-us.
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
** In addition, as a special exception, Digia gives you certain additional
** rights. These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** $QT_END_LICENSE$
**
****************************************************************************/


#include "qtextimagehandler_p.h"

#include <qguiapplication.h>
#include <qtextformat.h>
#include <qpainter.h>
#include <qdebug.h>
#include <private/qtextengine_p.h>
#include <qpalette.h>
#include <qthread.h>

QT_BEGIN_NAMESPACE

static QString resolveFileName(QString fileName, QUrl *url, qreal targetDevicePixelRatio)
{
    // We might use the fileName for loading if url loading fails
    // try to make sure it is a valid file path.
    // Also, QFile{Info}::exists works only on filepaths (not urls)

    if (url->isValid()) {
      if (url->scheme() == QLatin1Literal("qrc")) {
        fileName = fileName.right(fileName.length() - 3);
      }
      else if (url->scheme() == QLatin1Literal("file")) {
        fileName = url->toLocalFile();
      }
    }

    if (targetDevicePixelRatio <= 1.0)
        return fileName;

    // try to find a 2x version

    const int dotIndex = fileName.lastIndexOf(QLatin1Char('.'));
    if (dotIndex != -1) {
        QString at2xfileName = fileName;
        at2xfileName.insert(dotIndex, QStringLiteral("@2x"));
        if (QFile::exists(at2xfileName))  {
            fileName = at2xfileName;
            *url = QUrl(fileName);
        }
    }

    return fileName;
}


static QPixmap getPixmap(QTextDocument *doc, const QTextImageFormat &format, const qreal devicePixelRatio = 1.0)
{
    QPixmap pm;

    QString name = format.name();
    if (name.startsWith(QLatin1String(":/"))) // auto-detect resources and convert them to url
        name.prepend(QLatin1String("qrc"));
    QUrl url = QUrl(name);
    name = resolveFileName(name, &url, devicePixelRatio);
    const QVariant data = doc->resource(QTextDocument::ImageResource, url);
    if (data.type() == QVariant::Pixmap || data.type() == QVariant::Image) {
        pm = qvariant_cast<QPixmap>(data);
    } else if (data.type() == QVariant::ByteArray) {
        pm.loadFromData(data.toByteArray());
    }

    if (pm.isNull()) {
#if 0
        QString context;
        // ### Qt5
        QTextBrowser *browser = qobject_cast<QTextBrowser *>(doc->parent());
        if (browser)
            context = browser->source().toString();
#endif
        // try direct loading
        QImage img;
        if (name.isEmpty() || !img.load(name))
            return QPixmap(QLatin1String(":/qt-project.org/styles/commonstyle/images/file-16.png"));

        pm = QPixmap::fromImage(img);
        doc->addResource(QTextDocument::ImageResource, url, pm);
    }

    if (name.contains(QStringLiteral("@2x")))
        pm.setDevicePixelRatio(2.0);

    return pm;
}

static QSize getPixmapSize(QTextDocument *doc, const QTextImageFormat &format)
{
    QPixmap pm;

    const bool hasWidth = format.hasProperty(QTextFormat::ImageWidth);
    const int width = qRound(format.width());
    const bool hasHeight = format.hasProperty(QTextFormat::ImageHeight);
    const int height = qRound(format.height());

    QSize size(width, height);
    if (!hasWidth || !hasHeight) {
        pm = getPixmap(doc, format);
        const int pmWidth = pm.width() / pm.devicePixelRatio();
        const int pmHeight = pm.height() / pm.devicePixelRatio();

        if (!hasWidth) {
            if (!hasHeight)
                size.setWidth(pmWidth);
            else
                size.setWidth(qRound(height * (pmWidth / (qreal) pmHeight)));
        }
        if (!hasHeight) {
            if (!hasWidth)
                size.setHeight(pmHeight);
            else
                size.setHeight(qRound(width * (pmHeight / (qreal) pmWidth)));
        }
    }

    qreal scale = 1.0;
    QPaintDevice *pdev = doc->documentLayout()->paintDevice();
    if (pdev) {
        if (pm.isNull())
            pm = getPixmap(doc, format);
        if (!pm.isNull())
            scale = qreal(pdev->logicalDpiY()) / qreal(qt_defaultDpi());
    }
    size *= scale;

    return size;
}

static QImage getImage(QTextDocument *doc, const QTextImageFormat &format, const qreal devicePixelRatio = 1.0)
{
    QImage image;

    QString name = format.name();
    if (name.startsWith(QLatin1String(":/"))) // auto-detect resources
        name.prepend(QLatin1String("qrc"));
    QUrl url = QUrl(name);
    name = resolveFileName(name, &url, devicePixelRatio);
    const QVariant data = doc->resource(QTextDocument::ImageResource, url);
    if (data.type() == QVariant::Image) {
        image = qvariant_cast<QImage>(data);
    } else if (data.type() == QVariant::ByteArray) {
        image.loadFromData(data.toByteArray());
    }

    if (image.isNull()) {
#if 0
        QString context;
        // ### Qt5
        QTextBrowser *browser = qobject_cast<QTextBrowser *>(doc->parent());
        if (browser)
            context = browser->source().toString();
#endif
        // try direct loading

        if (name.isEmpty() || !image.load(name))
            return QImage(QLatin1String(":/qt-project.org/styles/commonstyle/images/file-16.png"));

        doc->addResource(QTextDocument::ImageResource, url, image);
    }

    if (name.contains(QStringLiteral("@2x")))
        image.setDevicePixelRatio(2.0);

    return image;
}

static QSize getImageSize(QTextDocument *doc, const QTextImageFormat &format)
{
    QImage image;

    const bool hasWidth = format.hasProperty(QTextFormat::ImageWidth);
    const int width = qRound(format.width());
    const bool hasHeight = format.hasProperty(QTextFormat::ImageHeight);
    const int height = qRound(format.height());

    QSize size(width, height);
    if (!hasWidth || !hasHeight) {
        image = getImage(doc, format);
        if (!hasWidth)
            size.setWidth(image.width() / image.devicePixelRatio());
        if (!hasHeight)
            size.setHeight(image.height() / image.devicePixelRatio());
    }

    qreal scale = 1.0;
    QPaintDevice *pdev = doc->documentLayout()->paintDevice();
    if (pdev) {
        if (image.isNull())
            image = getImage(doc, format);
        if (!image.isNull())
            scale = qreal(pdev->logicalDpiY()) / qreal(qt_defaultDpi());
    }
    size *= scale;

    return size;
}

QTextImageHandler::QTextImageHandler(QObject *parent)
    : QObject(parent)
{
}

QSizeF QTextImageHandler::intrinsicSize(QTextDocument *doc, int posInDocument, const QTextFormat &format)
{
    Q_UNUSED(posInDocument)
    const QTextImageFormat imageFormat = format.toImageFormat();

    if (QCoreApplication::instance()->thread() != QThread::currentThread())
        return getImageSize(doc, imageFormat);
    return getPixmapSize(doc, imageFormat);
}

QImage QTextImageHandler::image(QTextDocument *doc, const QTextImageFormat &imageFormat)
{
    Q_ASSERT(doc != 0);

    return getImage(doc, imageFormat);
}

void QTextImageHandler::drawObject(QPainter *p, const QRectF &rect, QTextDocument *doc, int posInDocument, const QTextFormat &format)
{
    Q_UNUSED(posInDocument)
        const QTextImageFormat imageFormat = format.toImageFormat();

    if (QCoreApplication::instance()->thread() != QThread::currentThread()) {
        const QImage image = getImage(doc, imageFormat, p->device()->devicePixelRatio());
        p->drawImage(rect, image, image.rect());
    } else {
        const QPixmap pixmap = getPixmap(doc, imageFormat, p->device()->devicePixelRatio());
        p->drawPixmap(rect, pixmap, pixmap.rect());
    }
}

QT_END_NAMESPACE
