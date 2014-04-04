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
#include "qcocoaaccessibilityelement.h"
#include "qcocoaaccessibility.h"
#include "qcocoahelpers.h"

#include <QtGui/qaccessible.h>

#import <AppKit/NSAccessibility.h>

@implementation QCocoaAccessibleElement

- (id)initWithId:(QAccessible::Id)anId parent:(id)aParent
{
    Q_ASSERT((int)anId < 0);
    self = [super init];
    if (self) {
        axid = anId;
        QAccessibleInterface *iface = QAccessible::accessibleInterface(axid);
        Q_ASSERT(iface);
        role = QCocoaAccessible::macRole(iface);
        parent = aParent;
    }

    return self;
}

+ (QCocoaAccessibleElement *)createElementWithId:(QAccessible::Id)anId parent:(id)aParent
{
    return [[self alloc] initWithId:anId parent:aParent];
}

- (void)dealloc {
    [super dealloc];
}

- (BOOL)isEqual:(id)object {
    if ([object isKindOfClass:[QCocoaAccessibleElement class]]) {
        QCocoaAccessibleElement *other = object;
        return other->axid == axid;
    } else {
        return NO;
    }
}

- (NSUInteger)hash {
    return axid;
}

//
// accessibility protocol
//

// attributes

+ (id) lineNumberForIndex: (int)index forText:(const QString &)text
{
    QStringRef textBefore = QStringRef(&text, 0, index);
    int newlines = textBefore.count(QLatin1Char('\n'));
    return [NSNumber numberWithInt: newlines];
}

- (NSArray *)accessibilityAttributeNames {
    static NSArray *defaultAttributes = nil;

    QAccessibleInterface *iface = QAccessible::accessibleInterface(axid);
    if (!iface)
        return defaultAttributes;

    if (defaultAttributes == nil) {
        defaultAttributes = [[NSArray alloc] initWithObjects:
        NSAccessibilityRoleAttribute,
        NSAccessibilityRoleDescriptionAttribute,
        NSAccessibilityChildrenAttribute,
        NSAccessibilityFocusedAttribute,
        NSAccessibilityParentAttribute,
        NSAccessibilityWindowAttribute,
        NSAccessibilityTopLevelUIElementAttribute,
        NSAccessibilityPositionAttribute,
        NSAccessibilitySizeAttribute,
        NSAccessibilityDescriptionAttribute,
        NSAccessibilityEnabledAttribute,
        nil];
    }

    NSMutableArray *attributes = [[NSMutableArray alloc] initWithCapacity : [defaultAttributes count]];
    [attributes addObjectsFromArray : defaultAttributes];

    if (QCocoaAccessible::hasValueAttribute(iface)) {
        [attributes addObject : NSAccessibilityValueAttribute];
    }

    if (iface->textInterface()) {
        [attributes addObjectsFromArray: [[NSArray alloc] initWithObjects:
            NSAccessibilityNumberOfCharactersAttribute,
            NSAccessibilitySelectedTextAttribute,
            NSAccessibilitySelectedTextRangeAttribute,
            NSAccessibilityVisibleCharacterRangeAttribute,
            NSAccessibilityInsertionPointLineNumberAttribute,
            nil
        ]];

// TODO: multi-selection: NSAccessibilitySelectedTextRangesAttribute,
    }

    return [attributes autorelease];
}

- (id)accessibilityAttributeValue:(NSString *)attribute {
    QAccessibleInterface *iface = QAccessible::accessibleInterface(axid);
    if (!iface) {
        qWarning() << "Called attribute on invalid object: " << axid;
        return nil;
    }

    if ([attribute isEqualToString:NSAccessibilityRoleAttribute]) {
        return role;
    } else if ([attribute isEqualToString:NSAccessibilityRoleDescriptionAttribute]) {
        return NSAccessibilityRoleDescription(role, nil);
    } else if ([attribute isEqualToString:NSAccessibilityChildrenAttribute]) {
        return QCocoaAccessible::unignoredChildren(self, iface);
    } else if ([attribute isEqualToString:NSAccessibilityFocusedAttribute]) {
        // Just check if the app thinks we're focused.
        id focusedElement = [NSApp accessibilityAttributeValue:NSAccessibilityFocusedUIElementAttribute];
        return [NSNumber numberWithBool:[focusedElement isEqual:self]];
    } else if ([attribute isEqualToString:NSAccessibilityParentAttribute]) {
        return NSAccessibilityUnignoredAncestor(parent);
    } else if ([attribute isEqualToString:NSAccessibilityWindowAttribute]) {
        // We're in the same window as our parent.
        return [parent accessibilityAttributeValue:NSAccessibilityWindowAttribute];
    } else if ([attribute isEqualToString:NSAccessibilityTopLevelUIElementAttribute]) {
        // We're in the same top level element as our parent.
        return [parent accessibilityAttributeValue:NSAccessibilityTopLevelUIElementAttribute];
    } else if ([attribute isEqualToString:NSAccessibilityPositionAttribute]) {
        QPoint qtPosition = iface->rect().topLeft();
        QSize qtSize = iface->rect().size();
        return [NSValue valueWithPoint: NSMakePoint(qtPosition.x(), qt_mac_flipYCoordinate(qtPosition.y() + qtSize.height()))];
    } else if ([attribute isEqualToString:NSAccessibilitySizeAttribute]) {
        QSize qtSize = iface->rect().size();
        return [NSValue valueWithSize: NSMakeSize(qtSize.width(), qtSize.height())];
    } else if ([attribute isEqualToString:NSAccessibilityDescriptionAttribute]) {
        return QCFString::toNSString(iface->text(QAccessible::Name));
    } else if ([attribute isEqualToString:NSAccessibilityEnabledAttribute]) {
        return [NSNumber numberWithBool:!iface->state().disabled];
    } else if ([attribute isEqualToString:NSAccessibilityValueAttribute]) {
        // VoiceOver asks for the value attribute for all elements. Return nil
        // if we don't want the element to have a value attribute.
        if (!QCocoaAccessible::hasValueAttribute(iface))
            return nil;

        return QCocoaAccessible::getValueAttribute(iface);

    } else if ([attribute isEqualToString:NSAccessibilityNumberOfCharactersAttribute]) {
        if (QAccessibleTextInterface *text = iface->textInterface())
            return [NSNumber numberWithInt: text->characterCount()];
        return nil;
    } else if ([attribute isEqualToString:NSAccessibilitySelectedTextAttribute]) {
        if (QAccessibleTextInterface *text = iface->textInterface()) {
            int start = 0;
            int end = 0;
            text->selection(0, &start, &end);
            return text->text(start, end).toNSString();
        }
        return nil;
    } else if ([attribute isEqualToString:NSAccessibilitySelectedTextRangeAttribute]) {
        if (QAccessibleTextInterface *text = iface->textInterface()) {
            int start = 0;
            int end = 0;
            if (text->selectionCount() > 0) {
                text->selection(0, &start, &end);
            } else {
                start = text->cursorPosition();
                end = start;
            }
            return [NSValue valueWithRange:NSMakeRange(quint32(start), quint32(end - start))];
        }
        return [NSValue valueWithRange: NSMakeRange(0, 0)];
    } else if ([attribute isEqualToString:NSAccessibilityVisibleCharacterRangeAttribute]) {
        // FIXME This is not correct and may impact performance for big texts
        return [NSValue valueWithRange: NSMakeRange(0, iface->textInterface()->characterCount())];

    } else if ([attribute isEqualToString:NSAccessibilityInsertionPointLineNumberAttribute]) {
        if (QAccessibleTextInterface *text = iface->textInterface()) {
            QString textBeforeCursor = text->text(0, text->cursorPosition());
            return [NSNumber numberWithInt: textBeforeCursor.count(QLatin1Char('\n'))];
        }
        return nil;
    }

    return nil;
}

- (NSArray *)accessibilityParameterizedAttributeNames {

    QAccessibleInterface *iface = QAccessible::accessibleInterface(axid);
    if (!iface) {
        qWarning() << "Called attribute on invalid object: " << axid;
        return nil;
    }

    if (iface->textInterface()) {
            return [[NSArray alloc] initWithObjects:
                    NSAccessibilityStringForRangeParameterizedAttribute,
                    NSAccessibilityLineForIndexParameterizedAttribute,
                    NSAccessibilityRangeForLineParameterizedAttribute,
                    NSAccessibilityRangeForPositionParameterizedAttribute,
//                    NSAccessibilityRangeForIndexParameterizedAttribute,
                    NSAccessibilityBoundsForRangeParameterizedAttribute,
//                    NSAccessibilityRTFForRangeParameterizedAttribute,
//                    NSAccessibilityStyleRangeForIndexParameterizedAttribute,
                    NSAccessibilityAttributedStringForRangeParameterizedAttribute,
                    nil
                ];
    }

    return nil;
}

- (id)accessibilityAttributeValue:(NSString *)attribute forParameter:(id)parameter {
    QAccessibleInterface *iface = QAccessible::accessibleInterface(axid);
    if (!iface) {
        qWarning() << "Called attribute on invalid object: " << axid;
        return nil;
    }

    if (!iface->textInterface())
        return nil;

    if ([attribute isEqualToString: NSAccessibilityStringForRangeParameterizedAttribute]) {
        NSRange range = [parameter rangeValue];
        QString text = iface->textInterface()->text(range.location, range.location + range.length);
        return text.toNSString();
    }
    if ([attribute isEqualToString: NSAccessibilityLineForIndexParameterizedAttribute]) {
        int index = [parameter intValue];
        NSNumber *ln = [QCocoaAccessibleElement lineNumberForIndex: index forText: iface->text(QAccessible::Value)];
        return ln;
    }
    if ([attribute isEqualToString: NSAccessibilityRangeForLineParameterizedAttribute]) {
        int lineNumber = [parameter intValue];
        QString text = iface->text(QAccessible::Value);
        int startOffset = 0;
        // skip newlines until we have the one we look for
        for (int i = 0; i < lineNumber; ++i)
            startOffset = text.indexOf(QLatin1Char('\n'), startOffset) + 1;
        if (startOffset < 0) // invalid line number, return the first line
            startOffset = 0;
        int endOffset = text.indexOf(QLatin1Char('\n'), startOffset + 1);
        if (endOffset == -1)
            endOffset = text.length();
        return [NSValue valueWithRange:NSMakeRange(quint32(startOffset), quint32(endOffset - startOffset))];
    }
    if ([attribute isEqualToString: NSAccessibilityBoundsForRangeParameterizedAttribute]) {
        NSRange range = [parameter rangeValue];
        QRect firstRect = iface->textInterface()->characterRect(range.location);
        QRect lastRect = iface->textInterface()->characterRect(range.location + range.length);
        QRect rect = firstRect.united(lastRect); // This is off quite often, but at least a rough approximation
        return [NSValue valueWithRect: NSMakeRect((CGFloat) rect.x(),(CGFloat) qt_mac_flipYCoordinate(rect.y() + rect.height()), rect.width(), rect.height())];
    }
    if ([attribute isEqualToString: NSAccessibilityAttributedStringForRangeParameterizedAttribute]) {
        NSRange range = [parameter rangeValue];
        QString text = iface->textInterface()->text(range.location, range.location + range.length);
        return [[NSAttributedString alloc] initWithString: text.toNSString()];
    }
    return nil;
}

- (BOOL)accessibilityIsAttributeSettable:(NSString *)attribute {
    QAccessibleInterface *iface = QAccessible::accessibleInterface(axid);
    if (!iface)
        return nil;

    if ([attribute isEqualToString:NSAccessibilityFocusedAttribute]) {
        return iface->state().focusable ? YES : NO;
    } else if ([attribute isEqualToString:NSAccessibilitySelectedTextRangeAttribute]) {
        return iface->textInterface() ? YES : NO;
    }
    return NO;
}

- (void)accessibilitySetValue:(id)value forAttribute:(NSString *)attribute {
    QAccessibleInterface *iface = QAccessible::accessibleInterface(axid);
    if (!iface)
        return;
    if ([attribute isEqualToString:NSAccessibilityFocusedAttribute]) {
        if (QAccessibleActionInterface *action = iface->actionInterface())
            action->doAction(QAccessibleActionInterface::setFocusAction());
    } else if ([attribute isEqualToString:NSAccessibilitySelectedTextRangeAttribute]) {
        if (QAccessibleTextInterface *text = iface->textInterface()) {
            NSRange range = [value rangeValue];
            if (range.length > 0)
                text->setSelection(0, range.location, range.location + range.length);
            else
                text->setCursorPosition(range.location);
        }
    }
}

// actions

- (NSArray *)accessibilityActionNames {
    NSMutableArray * nsActions = [NSMutableArray new];
    QAccessibleInterface *iface = QAccessible::accessibleInterface(axid);
    if (!iface)
        return nsActions;

    QAccessibleActionInterface *actionInterface = iface->actionInterface();
    if (actionInterface) {
        QStringList supportedActionNames = actionInterface->actionNames();

        foreach (const QString &qtAction, supportedActionNames) {
            NSString *nsAction = QCocoaAccessible::getTranslatedAction(qtAction);
            if (nsAction)
                [nsActions addObject : nsAction];
        }
    }

    return nsActions;
}

- (NSString *)accessibilityActionDescription:(NSString *)action {
    QAccessibleInterface *iface = QAccessible::accessibleInterface(axid);
    if (!iface)
        return nil; // FIXME is that the right return type??
    QAccessibleActionInterface *actionInterface = iface->actionInterface();
    if (actionInterface) {
        QString qtAction = QCocoaAccessible::translateAction(action);

        // Return a description from the action interface if this action is not known to the OS.
        if (qtAction.isEmpty()) {
            QString description = actionInterface->localizedActionDescription(qtAction);
            return QCFString::toNSString(description);
        }
    }

    return NSAccessibilityActionDescription(action);
}

- (void)accessibilityPerformAction:(NSString *)action {
    QAccessibleInterface *iface = QAccessible::accessibleInterface(axid);
    if (iface) {
        QAccessibleActionInterface *actionInterface = iface->actionInterface();
        if (actionInterface) {
            actionInterface->doAction(QCocoaAccessible::translateAction(action));
        }
    }
}

// misc

- (BOOL)accessibilityIsIgnored {
    QAccessibleInterface *iface = QAccessible::accessibleInterface(axid);
    if (!iface || !iface->isValid())
        return true;
    return QCocoaAccessible::shouldBeIgnored(iface);
}

- (id)accessibilityHitTest:(NSPoint)point {
    QAccessibleInterface *iface = QAccessible::accessibleInterface(axid);
    if (!iface || !iface->isValid()) {
//        qDebug() << "Hit test: INVALID";
        return NSAccessibilityUnignoredAncestor(self);
    }

    QAccessibleInterface *childInterface = iface->childAt(point.x, qt_mac_flipYCoordinate(point.y));

    // No child found, meaning we hit this element.
    if (!childInterface) {
//        qDebug() << "Hit test returns: " << id << iface;
        return NSAccessibilityUnignoredAncestor(self);
    }

    QAccessible::Id childId = QAccessible::uniqueId(childInterface);
    // hit a child, forward to child accessible interface.
    QCocoaAccessibleElement *accessibleElement = [QCocoaAccessibleElement createElementWithId:childId parent:self];
    [accessibleElement autorelease];

    return [accessibleElement accessibilityHitTest:point];
}

- (id)accessibilityFocusedUIElement {
    return NSAccessibilityUnignoredAncestor(self);
}

@end
