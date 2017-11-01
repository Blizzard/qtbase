/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the plugins of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QCOCOAHELPERS_H
#define QCOCOAHELPERS_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It provides helper functions
// for the Cocoa lighthouse plugin. This header file may
// change from version to version without notice, or even be removed.
//
// We mean it.
//
#include "qt_mac_p.h"
#include <private/qguiapplication_p.h>
#include <QtGui/qpalette.h>
#include <QtGui/qscreen.h>

Q_FORWARD_DECLARE_OBJC_CLASS(QT_MANGLE_NAMESPACE(QNSView));

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(lcQpaCocoaWindow)

class QPixmap;
class QString;

// Conversion functions
QStringList qt_mac_NSArrayToQStringList(void *nsarray);
void *qt_mac_QStringListToNSMutableArrayVoid(const QStringList &list);

inline NSMutableArray *qt_mac_QStringListToNSMutableArray(const QStringList &qstrlist)
{ return reinterpret_cast<NSMutableArray *>(qt_mac_QStringListToNSMutableArrayVoid(qstrlist)); }

NSDragOperation qt_mac_mapDropAction(Qt::DropAction action);
NSDragOperation qt_mac_mapDropActions(Qt::DropActions actions);
Qt::DropAction qt_mac_mapNSDragOperation(NSDragOperation nsActions);
Qt::DropActions qt_mac_mapNSDragOperations(NSDragOperation nsActions);

QT_MANGLE_NAMESPACE(QNSView) *qnsview_cast(NSView *view);

// Misc
void qt_mac_transformProccessToForegroundApplication();
QString qt_mac_applicationName();

int qt_mac_flipYCoordinate(int y);
qreal qt_mac_flipYCoordinate(qreal y);
QPointF qt_mac_flipPoint(const NSPoint &p);
NSPoint qt_mac_flipPoint(const QPoint &p);
NSPoint qt_mac_flipPoint(const QPointF &p);

NSRect qt_mac_flipRect(const QRect &rect);

Qt::MouseButton cocoaButton2QtButton(NSInteger buttonNum);

// strip out '&' characters, and convert "&&" to a single '&', in menu
// text - since menu text is sometimes decorated with these for Windows
// accelerators.
QString qt_mac_removeAmpersandEscapes(QString s);

enum {
    QtCocoaEventSubTypeWakeup       = SHRT_MAX,
    QtCocoaEventSubTypePostMessage  = SHRT_MAX-1
};

class QCocoaPostMessageArgs {
public:
    id target;
    SEL selector;
    int argCount;
    id arg1;
    id arg2;
    QCocoaPostMessageArgs(id target, SEL selector, int argCount=0, id arg1=0, id arg2=0)
        : target(target), selector(selector), argCount(argCount), arg1(arg1), arg2(arg2)
    {
        [target retain];
        [arg1 retain];
        [arg2 retain];
    }

    ~QCocoaPostMessageArgs()
    {
        [arg2 release];
        [arg1 release];
        [target release];
    }
};

template<typename T>
T qt_mac_resolveOption(const T &fallback, const QByteArray &environment)
{
    // check for environment variable
    if (!environment.isEmpty()) {
        QByteArray env = qgetenv(environment);
        if (!env.isEmpty())
            return T(env.toInt()); // works when T is bool, int.
    }

    return fallback;
}

template<typename T>
T qt_mac_resolveOption(const T &fallback, QWindow *window, const QByteArray &property, const QByteArray &environment)
{
    // check for environment variable
    if (!environment.isEmpty()) {
        QByteArray env = qgetenv(environment);
        if (!env.isEmpty())
            return T(env.toInt()); // works when T is bool, int.
    }

    // check for window property
    if (window && !property.isNull()) {
        QVariant windowProperty = window->property(property);
        if (windowProperty.isValid())
            return windowProperty.value<T>();
    }

    // return default value.
    return fallback;
}

QT_END_NAMESPACE

@protocol QT_MANGLE_NAMESPACE(QNSPanelDelegate)
@required
- (void)onOkClicked;
- (void)onCancelClicked;
@end

@interface QT_MANGLE_NAMESPACE(QNSPanelContentsWrapper) : NSView {
    NSButton *_okButton;
    NSButton *_cancelButton;
    NSView *_panelContents;
    NSEdgeInsets _panelContentsMargins;
}

@property (nonatomic, readonly) NSButton *okButton;
@property (nonatomic, readonly) NSButton *cancelButton;
@property (nonatomic, readonly) NSView *panelContents; // ARC: unretained, make it weak
@property (nonatomic, assign) NSEdgeInsets panelContentsMargins;

- (instancetype)initWithPanelDelegate:(id<QT_MANGLE_NAMESPACE(QNSPanelDelegate)>)panelDelegate;
- (void)dealloc;

- (NSButton *)createButtonWithTitle:(const char *)title;
- (void)layout;
@end

QT_NAMESPACE_ALIAS_OBJC_CLASS(QNSPanelContentsWrapper);

#endif //QCOCOAHELPERS_H

