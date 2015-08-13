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

#ifdef XCB_USE_XLIB

#include "qxlibconvenience.h"

// Some Xlib headers are heavy macro namespace polluters and conflict with Qt types.
// This unit makes it easier to deal with them by encapsulating these includes in this .cpp.
#include <X11/Xutil.h>

QT_BEGIN_NAMESPACE

xcb_keysym_t q_XLookupString(void *display, xcb_window_t window, xcb_window_t root, uint state, xcb_keycode_t code, int type, QByteArray *chars)
{
    KeySym sym = 0;
    chars->resize(512);
    XKeyEvent event;
    memset(&event, 0, sizeof(event));
    event.type = type;
    event.display = static_cast<Display*>(display);
    event.window = window;
    event.root = root;
    event.state = state;
    event.keycode = code;
    int count = XLookupString(&event, chars->data(), chars->size(), &sym, 0);
    chars->resize(count);
    return sym;
}

QT_END_NAMESPACE

#endif
