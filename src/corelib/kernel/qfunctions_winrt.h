/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtCore module of the Qt Toolkit.
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

#ifndef QFUNCTIONS_WINRT_H
#define QFUNCTIONS_WINRT_H

#include <QtCore/qglobal.h>

#ifdef Q_OS_WIN

#include <QtCore/QThread>
#include <QtCore/QAbstractEventDispatcher>
#include <QtCore/qt_windows.h>

QT_BEGIN_NAMESPACE

#ifdef QT_BUILD_CORE_LIB
#endif

QT_END_NAMESPACE

#ifdef Q_OS_WINRT

// Environment ------------------------------------------------------
errno_t qt_winrt_getenv_s(size_t*, char*, size_t, const char*);
errno_t qt_winrt__putenv_s(const char*, const char*);
void qt_winrt_tzset();
void qt_winrt__tzset();

// As Windows Runtime lacks some standard functions used in Qt, these got
// reimplemented. Other projects do this as well. Inline functions are used
// that there is a central place to disable functions for newer versions if
// they get available. There are no defines used anymore, because this
// will break member functions of classes which are called like these
// functions.
// The other declarations available in this file are being used per
// define inside qplatformdefs.h of the corresponding WinRT mkspec.

#define generate_inline_return_func0(funcname, returntype) \
        inline returntype funcname() \
        { \
            return qt_winrt_##funcname(); \
        }
#define generate_inline_return_func1(funcname, returntype, param1) \
        inline returntype funcname(param1 p1) \
        { \
            return qt_winrt_##funcname(p1); \
        }
#define generate_inline_return_func2(funcname, returntype, param1, param2) \
        inline returntype funcname(param1 p1, param2 p2) \
        { \
            return qt_winrt_##funcname(p1,  p2); \
        }
#define generate_inline_return_func3(funcname, returntype, param1, param2, param3) \
        inline returntype funcname(param1 p1, param2 p2, param3 p3) \
        { \
            return qt_winrt_##funcname(p1,  p2, p3); \
        }
#define generate_inline_return_func4(funcname, returntype, param1, param2, param3, param4) \
        inline returntype funcname(param1 p1, param2 p2, param3 p3, param4 p4) \
        { \
            return qt_winrt_##funcname(p1,  p2, p3, p4); \
        }
#define generate_inline_return_func5(funcname, returntype, param1, param2, param3, param4, param5) \
        inline returntype funcname(param1 p1, param2 p2, param3 p3, param4 p4, param5 p5) \
        { \
            return qt_winrt_##funcname(p1,  p2, p3, p4, p5); \
        }
#define generate_inline_return_func6(funcname, returntype, param1, param2, param3, param4, param5, param6) \
        inline returntype funcname(param1 p1, param2 p2, param3 p3, param4 p4, param5 p5, param6 p6) \
        { \
            return qt_winrt_##funcname(p1,  p2, p3, p4, p5, p6); \
        }
#define generate_inline_return_func7(funcname, returntype, param1, param2, param3, param4, param5, param6, param7) \
        inline returntype funcname(param1 p1, param2 p2, param3 p3, param4 p4, param5 p5, param6 p6, param7 p7) \
        { \
            return qt_winrt_##funcname(p1,  p2, p3, p4, p5, p6, p7); \
        }

typedef unsigned (__stdcall *StartAdressExFunc)(void *);
typedef void(*StartAdressFunc)(void *);
typedef int ( __cdecl *CompareFunc ) (const void *, const void *) ;

generate_inline_return_func4(getenv_s, errno_t, size_t *, char *, size_t, const char *)
generate_inline_return_func2(_putenv_s, errno_t, const char *, const char *)
generate_inline_return_func0(tzset, void)
generate_inline_return_func0(_tzset, void)

#endif // Q_OS_WINRT

// Convenience macros for handling HRESULT values
#define RETURN_IF_FAILED(msg, ret) \
    if (FAILED(hr)) { \
        qErrnoWarning(hr, msg); \
        ret; \
    }

#define RETURN_HR_IF_FAILED(msg) RETURN_IF_FAILED(msg, return hr)
#define RETURN_OK_IF_FAILED(msg) RETURN_IF_FAILED(msg, return S_OK)
#define RETURN_FALSE_IF_FAILED(msg) RETURN_IF_FAILED(msg, return false)
#define RETURN_VOID_IF_FAILED(msg) RETURN_IF_FAILED(msg, return)

#define Q_ASSERT_SUCCEEDED(hr) \
    Q_ASSERT_X(SUCCEEDED(hr), Q_FUNC_INFO, qPrintable(qt_error_string(hr)));


namespace Microsoft { namespace WRL { template <typename T> class ComPtr; } }

namespace QWinRTFunctions {

// Synchronization methods
enum AwaitStyle
{
    YieldThread = 0,
    ProcessThreadEvents = 1,
    ProcessMainThreadEvents = 2
};

template <typename T>
static inline HRESULT _await_impl(const Microsoft::WRL::ComPtr<T> &asyncOp, AwaitStyle awaitStyle)
{
    Microsoft::WRL::ComPtr<IAsyncInfo> asyncInfo;
    HRESULT hr = asyncOp.As(&asyncInfo);
    if (FAILED(hr))
        return hr;

    AsyncStatus status;
    switch (awaitStyle) {
    case ProcessMainThreadEvents:
        while (SUCCEEDED(hr = asyncInfo->get_Status(&status)) && status == Started)
            QCoreApplication::processEvents();
        break;
    case ProcessThreadEvents:
        if (QAbstractEventDispatcher *dispatcher = QThread::currentThread()->eventDispatcher()) {
            while (SUCCEEDED(hr = asyncInfo->get_Status(&status)) && status == Started)
                dispatcher->processEvents(QEventLoop::AllEvents);
            break;
        }
        // fall through
    default:
    case YieldThread:
        while (SUCCEEDED(hr = asyncInfo->get_Status(&status)) && status == Started)
            QThread::yieldCurrentThread();
        break;
    }

    if (FAILED(hr) || status != Completed) {
        HRESULT ec;
        hr = asyncInfo->get_ErrorCode(&ec);
        if (FAILED(hr))
            return hr;
        return ec;
    }

    return hr;
}

template <typename T>
static inline HRESULT await(const Microsoft::WRL::ComPtr<T> &asyncOp, AwaitStyle awaitStyle = YieldThread)
{
    HRESULT hr = _await_impl(asyncOp, awaitStyle);
    if (FAILED(hr))
        return hr;

    return asyncOp->GetResults();
}

template <typename T, typename U>
static inline HRESULT await(const Microsoft::WRL::ComPtr<T> &asyncOp, U *results, AwaitStyle awaitStyle = YieldThread)
{
    HRESULT hr = _await_impl(asyncOp, awaitStyle);
    if (FAILED(hr))
        return hr;

    return asyncOp->GetResults(results);
}

} // QWinRTFunctions

#endif // Q_OS_WIN

#endif // QFUNCTIONS_WINRT_H
