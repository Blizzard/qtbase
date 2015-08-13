/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the test suite of the Qt Toolkit.
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

#include "qwidgetdump.h"

#include <QWidget>
#if QT_VERSION > 0x050000
#  include <QtGui/QScreen>
#  include <QtGui/QWindow>
#endif
#include <QApplication>
#include <QtCore/QMetaObject>
#include <QtCore/QDebug>
#include <QtCore/QTextStream>

namespace QtDiag {

static void dumpWidgetRecursion(QTextStream &str, const QWidget *w,
                                FormatWindowOptions options, int depth = 0)
{
    indentStream(str, 2 * depth);
    formatObject(str, w);
    str << ' ' << (w->isVisible() ? "[visible] " : "[hidden] ");
    if (const WId nativeWinId = w->internalWinId())
        str << "[native: " << hex << showbase << nativeWinId << dec << noshowbase << "] ";
    if (w->isWindow())
        str << "[top] ";
    str << (w->testAttribute(Qt::WA_Mapped) ? "[mapped] " : "[not mapped] ");
    if (w->testAttribute(Qt::WA_DontCreateNativeAncestors))
        str << "[NoNativeAncestors] ";
    formatRect(str, w->geometry());
    if (!(options & DontPrintWindowFlags)) {
        str << ' ';
        formatWindowFlags(str, w->windowFlags());
    }
    str << '\n';
#if QT_VERSION > 0x050000
    if (const QWindow *win = w->windowHandle()) {
        indentStream(str, 2 * (1 + depth));
        formatWindow(str, win, options);
        str << '\n';
    }
#endif // Qt 5
    foreach (const QObject *co, w->children()) {
        if (co->isWidgetType())
            dumpWidgetRecursion(str, static_cast<const QWidget *>(co), options, depth + 1);
    }
}

void dumpAllWidgets(FormatWindowOptions options)
{
    QString d;
    QTextStream str(&d);
    str << "### QWidgets:\n";
    foreach (QWidget *tw, QApplication::topLevelWidgets())
        dumpWidgetRecursion(str, tw, options);
#if QT_VERSION >= 0x050400
    qDebug().noquote() << d;
#else
    qDebug("%s", qPrintable(d));
#endif
}

} // namespace QtDiag
