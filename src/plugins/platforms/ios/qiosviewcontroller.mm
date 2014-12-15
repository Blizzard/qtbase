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

#import "qiosviewcontroller.h"

#include <QtGui/QGuiApplication>
#include <QtGui/QWindow>
#include <QtGui/QScreen>

#include <QtGui/private/qwindow_p.h>

#include "qiosscreen.h"
#include "qiosglobal.h"
#include "qioswindow.h"
#include "quiview.h"

// -------------------------------------------------------------------------

@interface QIOSDesktopManagerView : UIView
@end

@implementation QIOSDesktopManagerView

- (void)layoutSubviews
{
    for (int i = int(self.subviews.count) - 1; i >= 0; --i) {
        UIView *view = static_cast<UIView *>([self.subviews objectAtIndex:i]);
        if (![view isKindOfClass:[QUIView class]])
            continue;

        [self layoutView: static_cast<QUIView *>(view)];
    }
}

- (void)layoutView:(QUIView *)view
{
    QWindow *window = view.qwindow;
    Q_ASSERT(window->handle());

    // Re-apply window states to update geometry
    if (window->windowState() & (Qt::WindowFullScreen | Qt::WindowMaximized))
        window->handle()->setWindowState(window->windowState());
}

// Even if the root view controller has both wantsFullScreenLayout and
// extendedLayoutIncludesOpaqueBars enabled, iOS will still push the root
// view down 20 pixels (and shrink the view accordingly) when the in-call
// statusbar is active (instead of updating the topLayoutGuide). Since
// we treat the root view controller as our screen, we want to reflect
// the in-call statusbar as a change in available geometry, not in screen
// geometry. To simplify the screen geometry mapping code we reset the
// view modifications that iOS does and take the statusbar height
// explicitly into account in QIOSScreen::updateProperties().

- (void)setFrame:(CGRect)newFrame
{
    [super setFrame:CGRectMake(0, 0, CGRectGetWidth(newFrame), CGRectGetHeight(self.window.bounds))];
}

- (void)setBounds:(CGRect)newBounds
{
    CGRect transformedWindowBounds = [self convertRect:self.window.bounds fromView:self.window];
    [super setBounds:CGRectMake(0, 0, CGRectGetWidth(newBounds), CGRectGetHeight(transformedWindowBounds))];
}

- (void)setCenter:(CGPoint)newCenter
{
    Q_UNUSED(newCenter);
    [super setCenter:self.window.center];
}

- (void)didMoveToWindow
{
    // The initial frame computed during startup may happen before the view has
    // a window, meaning our calculations above will be wrong. We ensure that the
    // frame is set correctly once we have a window to base our calulations on.
    [self setFrame:self.window.bounds];
}

@end

// -------------------------------------------------------------------------

@implementation QIOSViewController

- (id)initWithQIOSScreen:(QIOSScreen *)screen
{
    if (self = [self init]) {
        m_screen = screen;

#if QT_IOS_DEPLOYMENT_TARGET_BELOW(__IPHONE_7_0)
        QSysInfo::MacVersion iosVersion = QSysInfo::MacintoshVersion;

        // We prefer to keep the root viewcontroller in fullscreen layout, so that
        // we don't have to compensate for the viewcontroller position. This also
        // gives us the same behavior on iOS 5/6 as on iOS 7, where full screen layout
        // is the only way.
        if (iosVersion < QSysInfo::MV_IOS_7_0)
            self.wantsFullScreenLayout = YES;

        // Use translucent statusbar by default on iOS6 iPhones (unless the user changed
        // the default in the Info.plist), so that windows placed under the stausbar are
        // still visible, just like on iOS7.
        if (screen->uiScreen() == [UIScreen mainScreen]
            && iosVersion >= QSysInfo::MV_IOS_6_0 && iosVersion < QSysInfo::MV_IOS_7_0
            && [UIDevice currentDevice].userInterfaceIdiom == UIUserInterfaceIdiomPhone
            && [UIApplication sharedApplication].statusBarStyle == UIStatusBarStyleDefault)
            [[UIApplication sharedApplication] setStatusBarStyle:UIStatusBarStyleBlackTranslucent];
#endif

        self.changingOrientation = NO;

        // Status bar may be initially hidden at startup through Info.plist
        self.prefersStatusBarHidden = infoPlistValue(@"UIStatusBarHidden", false);
        self.preferredStatusBarUpdateAnimation = UIStatusBarAnimationNone;

        QObject::connect(qApp, &QGuiApplication::focusWindowChanged, [self]() {
            [self updateProperties];
        });
    }

    return self;
}

- (void)loadView
{
    self.view = [[[QIOSDesktopManagerView alloc] init] autorelease];
}

- (void)viewDidLoad
{
    [super viewDidLoad];

    NSNotificationCenter *center = [NSNotificationCenter defaultCenter];
    [center addObserver:self selector:@selector(willChangeStatusBarFrame:)
            name:UIApplicationWillChangeStatusBarFrameNotification
            object:[UIApplication sharedApplication]];
}

- (void)viewDidUnload
{
    [[NSNotificationCenter defaultCenter] removeObserver:self name:nil object:nil];
    [super viewDidUnload];
}

// -------------------------------------------------------------------------

-(BOOL)shouldAutorotate
{
    // Until a proper orientation and rotation API is in place, we always auto rotate.
    // If auto rotation is not wanted, you would need to switch it off manually from Info.plist.
    return YES;
}

#if QT_IOS_PLATFORM_SDK_EQUAL_OR_ABOVE(__IPHONE_6_0)
-(NSUInteger)supportedInterfaceOrientations
{
    // We need to tell iOS that we support all orientations in order to set
    // status bar orientation when application content orientation changes.
    return UIInterfaceOrientationMaskAll;
}
#endif

#if QT_IOS_DEPLOYMENT_TARGET_BELOW(__IPHONE_6_0)
-(BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
    Q_UNUSED(interfaceOrientation);
    return YES;
}
#endif

- (void)willRotateToInterfaceOrientation:(UIInterfaceOrientation)orientation duration:(NSTimeInterval)duration
{
    Q_UNUSED(orientation);
    Q_UNUSED(duration);

    self.changingOrientation = YES;
}

- (void)didRotateFromInterfaceOrientation:(UIInterfaceOrientation)orientation
{
    Q_UNUSED(orientation);

    self.changingOrientation = NO;
}

- (void)willChangeStatusBarFrame:(NSNotification*)notification
{
    Q_UNUSED(notification);

    if (self.view.window.screen != [UIScreen mainScreen])
        return;

    // Orientation changes will already result in laying out subviews, so we don't
    // need to do anything extra for frame changes during an orientation change.
    // Technically we can receive another actual statusbar frame update during the
    // orientation change that we should react to, but to simplify the logic we
    // use a simple bool variable instead of a ignoreNextFrameChange approach.
    if (self.changingOrientation)
        return;

    // UIKit doesn't have a delegate callback for statusbar changes that's run inside the
    // animation block, like UIViewController's willAnimateRotationToInterfaceOrientation,
    // nor does it expose a constant for the duration and easing of the animation. However,
    // though poking at the various UIStatusBar methods, we can observe that the animation
    // uses the default easing curve, and runs with a duration of 0.35 seconds.
    static qreal kUIStatusBarAnimationDuration = 0.35;

    [UIView animateWithDuration:kUIStatusBarAnimationDuration animations:^{
        [self.view setNeedsLayout];
        [self.view layoutIfNeeded];
    }];
}

- (void)viewWillLayoutSubviews
{
    if (!QCoreApplication::instance())
        return;

    m_screen->updateProperties();
}

// -------------------------------------------------------------------------

- (void)updateProperties
{
    if (!isQtApplication())
        return;

    QWindow *focusWindow = QGuiApplication::focusWindow();

    // If we don't have a focus window we leave the statusbar
    // as is, so that the user can activate a new window with
    // the same window state without the status bar jumping
    // back and forth.
    if (!focusWindow)
        return;

    // We only care about changes to focusWindow that involves our screen
    if (!focusWindow->screen() || focusWindow->screen()->handle() != m_screen)
        return;

    // All decisions are based on the the top level window
    focusWindow = qt_window_private(focusWindow)->topLevelWindow();

    bool hasScrolledRootViewDueToVirtualKeyboard =
        !CATransform3DIsIdentity(self.view.layer.sublayerTransform);

    bool currentStatusBarVisibility = self.prefersStatusBarHidden;
    self.prefersStatusBarHidden = focusWindow->windowState() == Qt::WindowFullScreen
                                    || hasScrolledRootViewDueToVirtualKeyboard;
    self.preferredStatusBarUpdateAnimation = hasScrolledRootViewDueToVirtualKeyboard ?
        UIStatusBarAnimationFade : UIStatusBarAnimationNone;

    if (self.prefersStatusBarHidden != currentStatusBarVisibility) {
#if QT_IOS_PLATFORM_SDK_EQUAL_OR_ABOVE(__IPHONE_7_0)
        if (QSysInfo::MacintoshVersion >= QSysInfo::MV_IOS_7_0) {
            [self setNeedsStatusBarAppearanceUpdate];
        } else
#endif
        {
            [[UIApplication sharedApplication]
                setStatusBarHidden:self.prefersStatusBarHidden
                withAnimation:self.preferredStatusBarUpdateAnimation];
        }

        [self.view setNeedsLayout];
    }
}

#if QT_IOS_PLATFORM_SDK_EQUAL_OR_ABOVE(__IPHONE_7_0)
- (UIStatusBarStyle)preferredStatusBarStyle
{
    // Since we don't place anything behind the status bare by default, we
    // end up with a black area, so we have to enable the white text mode
    // of the iOS7 statusbar.
    return UIStatusBarStyleLightContent;

    // FIXME: Try to detect the content underneath the statusbar and choose
    // an appropriate style, and/or expose Qt APIs to control the style.
}
#endif

@end

