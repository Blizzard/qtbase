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

#include "qstandardpaths.h"

#include <qdir.h>
#include <private/qsystemlibrary_p.h>
#include <qstringlist.h>

#ifndef QT_BOOTSTRAPPED
#include <qcoreapplication.h>
#endif

#if !defined(Q_OS_WINCE)
const GUID qCLSID_FOLDERID_Downloads = { 0x374de290, 0x123f, 0x4565, { 0x91, 0x64,  0x39,  0xc4,  0x92,  0x5e,  0x46,  0x7b } };
#endif

#include <qt_windows.h>
#include <shlobj.h>
#if !defined(Q_OS_WINCE)
#  include <intshcut.h>
#else
#  if !defined(STANDARDSHELL_UI_MODEL)
#    include <winx.h>
#  endif
#endif

#ifndef CSIDL_MYMUSIC
#define CSIDL_MYMUSIC 13
#define CSIDL_MYVIDEO 14
#endif

#ifndef QT_NO_STANDARDPATHS

QT_BEGIN_NAMESPACE

#if !defined(Q_OS_WINCE)
typedef HRESULT (WINAPI *GetKnownFolderPath)(const GUID&, DWORD, HANDLE, LPWSTR*);
#endif

static QString convertCharArray(const wchar_t *path)
{
    return QDir::fromNativeSeparators(QString::fromWCharArray(path));
}

static inline int clsidForAppDataLocation(QStandardPaths::StandardLocation type)
{
#ifndef Q_OS_WINCE
    return type == QStandardPaths::AppDataLocation ?
        CSIDL_APPDATA :      // "Roaming" path
        CSIDL_LOCAL_APPDATA; // Local path
#else
    Q_UNUSED(type)
    return CSIDL_APPDATA;
#endif
}

QString QStandardPaths::writableLocation(StandardLocation type)
{
    QString result;

#if !defined(Q_OS_WINCE)
    static GetKnownFolderPath SHGetKnownFolderPath = (GetKnownFolderPath)QSystemLibrary::resolve(QLatin1String("shell32"), "SHGetKnownFolderPath");
#endif

    wchar_t path[MAX_PATH];

    switch (type) {
    case ConfigLocation: // same as AppLocalDataLocation, on Windows
    case GenericConfigLocation: // same as GenericDataLocation on Windows
    case AppConfigLocation:
    case AppDataLocation:
    case AppLocalDataLocation:
    case GenericDataLocation:
        if (SHGetSpecialFolderPath(0, path, clsidForAppDataLocation(type), FALSE))
            result = convertCharArray(path);
        if (isTestModeEnabled())
            result += QLatin1String("/qttest");
#ifndef QT_BOOTSTRAPPED
        if (type != GenericDataLocation && type != GenericConfigLocation) {
            if (!QCoreApplication::organizationName().isEmpty())
                result += QLatin1Char('/') + QCoreApplication::organizationName();
            if (!QCoreApplication::applicationName().isEmpty())
                result += QLatin1Char('/') + QCoreApplication::applicationName();
        }
#endif
        break;

    case DesktopLocation:
        if (SHGetSpecialFolderPath(0, path, CSIDL_DESKTOPDIRECTORY, FALSE))
            result = convertCharArray(path);
        break;

    case DownloadLocation:
#if !defined(Q_OS_WINCE)
        if (SHGetKnownFolderPath) {
            LPWSTR path;
            if (SHGetKnownFolderPath(qCLSID_FOLDERID_Downloads, 0, 0, &path) == S_OK) {
                result = convertCharArray(path);
                CoTaskMemFree(path);
            }
            break;
        }
#endif
        // fall through
    case DocumentsLocation:
        if (SHGetSpecialFolderPath(0, path, CSIDL_PERSONAL, FALSE))
            result = convertCharArray(path);
        break;

    case FontsLocation:
        if (SHGetSpecialFolderPath(0, path, CSIDL_FONTS, FALSE))
            result = convertCharArray(path);
        break;

    case ApplicationsLocation:
        if (SHGetSpecialFolderPath(0, path, CSIDL_PROGRAMS, FALSE))
            result = convertCharArray(path);
        break;

    case MusicLocation:
        if (SHGetSpecialFolderPath(0, path, CSIDL_MYMUSIC, FALSE))
            result = convertCharArray(path);
        break;

    case MoviesLocation:
        if (SHGetSpecialFolderPath(0, path, CSIDL_MYVIDEO, FALSE))
            result = convertCharArray(path);
        break;

    case PicturesLocation:
        if (SHGetSpecialFolderPath(0, path, CSIDL_MYPICTURES, FALSE))
            result = convertCharArray(path);
        break;

    case CacheLocation:
        // Although Microsoft has a Cache key it is a pointer to IE's cache, not a cache
        // location for everyone.  Most applications seem to be using a
        // cache directory located in their AppData directory
        return writableLocation(AppLocalDataLocation) + QLatin1String("/cache");

    case GenericCacheLocation:
        return writableLocation(GenericDataLocation) + QLatin1String("/cache");

    case RuntimeLocation:
    case HomeLocation:
        result = QDir::homePath();
        break;

    case TempLocation:
        result = QDir::tempPath();
        break;
    }
    return result;
}

QStringList QStandardPaths::standardLocations(StandardLocation type)
{
    QStringList dirs;

    // type-specific handling goes here

#ifndef Q_OS_WINCE
    {
        wchar_t path[MAX_PATH];
        switch (type) {
        case ConfigLocation: // same as AppLocalDataLocation, on Windows (oversight, but too late to fix it)
        case GenericConfigLocation: // same as GenericDataLocation, on Windows
        case AppConfigLocation: // same as AppLocalDataLocation, that one on purpose
        case AppDataLocation:
        case AppLocalDataLocation:
        case GenericDataLocation:
            if (SHGetSpecialFolderPath(0, path, clsidForAppDataLocation(type), FALSE)) {
                QString result = convertCharArray(path);
                if (type != GenericDataLocation && type != GenericConfigLocation) {
#ifndef QT_BOOTSTRAPPED
                    if (!QCoreApplication::organizationName().isEmpty())
                        result += QLatin1Char('/') + QCoreApplication::organizationName();
                    if (!QCoreApplication::applicationName().isEmpty())
                        result += QLatin1Char('/') + QCoreApplication::applicationName();
#endif
                }
                dirs.append(result);
#ifndef QT_BOOTSTRAPPED
                if (type != GenericDataLocation) {
                    dirs.append(QCoreApplication::applicationDirPath());
                    dirs.append(QCoreApplication::applicationDirPath() + QLatin1String("/data"));
                }
#endif
            }
            break;
        default:
            break;
        }
    }
#endif

    const QString localDir = writableLocation(type);
    dirs.prepend(localDir);
    return dirs;
}

QT_END_NAMESPACE

#endif // QT_NO_STANDARDPATHS
