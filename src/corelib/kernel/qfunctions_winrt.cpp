/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtCore module of the Qt Toolkit.
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

#include "qfunctions_winrt.h"

#ifdef Q_OS_WINRT

#include "qstring.h"
#include "qbytearray.h"
#include "qhash.h"

QT_BEGIN_NAMESPACE

// Environment ------------------------------------------------------
inline QHash<QByteArray, QByteArray> &qt_app_environment()
{
    static QHash<QByteArray, QByteArray> internalEnvironment;
    return internalEnvironment;
}

errno_t qt_winrt_getenv_s(size_t* sizeNeeded, char* buffer, size_t bufferSize, const char* varName)
{
    if (!sizeNeeded)
        return EINVAL;

    if (!qt_app_environment().contains(varName)) {
        if (buffer)
            buffer[0] = '\0';
        return ENOENT;
    }

    QByteArray value = qt_app_environment().value(varName);
    if (!value.endsWith('\0')) // win32 guarantees terminated string
        value.append('\0');

    if (bufferSize < (size_t)value.size()) {
        *sizeNeeded = value.size();
        return ERANGE;
    }

    strcpy(buffer, value.constData());
    return 0;
}

errno_t qt_winrt__putenv_s(const char* varName, const char* value)
{
    QByteArray input = value;
    if (input.isEmpty()) {
        if (qt_app_environment().contains(varName))
            qt_app_environment().remove(varName);
    } else {
        // win32 on winrt guarantees terminated string
        if (!input.endsWith('\0'))
            input.append('\0');
        qt_app_environment()[varName] = input;
    }

    return 0;
}

void qt_winrt_tzset()
{
}

void qt_winrt__tzset()
{
}

QT_END_NAMESPACE

#endif // Q_OS_WINRT
