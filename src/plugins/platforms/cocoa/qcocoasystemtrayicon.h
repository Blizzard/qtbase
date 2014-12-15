/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
** Copyright (C) 2012 Klaralvdalens Datakonsult AB, a KDAB Group company, info@kdab.com, author Christoph Schleifenbaum <christoph.schleifenbaum@kdab.com>
** Contact: http://www.qt-project.org/legal
**
** This file is part of the plugins of the Qt Toolkit.
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

#ifndef QCOCOASYSTEMTRAYICON_P_H
#define QCOCOASYSTEMTRAYICON_P_H

#include <QtCore/qglobal.h>

#ifndef QT_NO_SYSTEMTRAYICON

#include "QtCore/qstring.h"
#include "QtGui/qpa/qplatformsystemtrayicon.h"

QT_BEGIN_NAMESPACE

class QSystemTrayIconSys;

class Q_GUI_EXPORT QCocoaSystemTrayIcon : public QPlatformSystemTrayIcon
{
public:
    QCocoaSystemTrayIcon() : m_sys(0) {}

    virtual void init();
    virtual void cleanup();
    virtual void updateIcon(const QIcon &icon);
    virtual void updateToolTip(const QString &toolTip);
    virtual void updateMenu(QPlatformMenu *menu);
    virtual QRect geometry() const;
    virtual void showMessage(const QString &msg, const QString &title,
                             const QIcon& icon, MessageIcon iconType, int secs);

    virtual bool isSystemTrayAvailable() const;
    virtual bool supportsMessages() const;

private:
    QSystemTrayIconSys *m_sys;
};

QT_END_NAMESPACE

#endif // QT_NO_SYSTEMTRAYICON

#endif // QCOCOASYSTEMTRAYICON_P_H
