/****************************************************************************
 **
 ** Copyright (C) 2016 The Qt Company Ltd.
 ** Copyright (C) 2014 Petroules Corporation.
 ** Contact: https://www.qt.io/licensing/
 **
 ** This file is part of the QtCore module of the Qt Toolkit.
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

#include <private/qcore_mac_p.h>

#ifdef Q_OS_OSX
#include <AppKit/NSText.h>
#include <Carbon/Carbon.h>
#endif

#include <qdebug.h>

QT_BEGIN_NAMESPACE

// -------------------------------------------------------------------------

QDebug operator<<(QDebug dbg, const NSObject *nsObject)
{
    return dbg << (nsObject ? nsObject.description.UTF8String : "NSObject(0x0)");
}

QDebug operator<<(QDebug dbg, CFStringRef stringRef)
{
    if (!stringRef)
        return dbg << "CFStringRef(0x0)";

    if (const UniChar *chars = CFStringGetCharactersPtr(stringRef))
        dbg << QString::fromRawData(reinterpret_cast<const QChar *>(chars), CFStringGetLength(stringRef));
    else
        dbg << QString::fromCFString(stringRef);

    return dbg;
}

// Prevents breaking the ODR in case we introduce support for more types
// later on, and lets the user override our default QDebug operators.
#define QT_DECLARE_WEAK_QDEBUG_OPERATOR_FOR_CF_TYPE(CFType) \
    __attribute__((weak)) Q_DECLARE_QDEBUG_OPERATOR_FOR_CF_TYPE(CFType)

QT_FOR_EACH_CORE_FOUNDATION_TYPE(QT_DECLARE_WEAK_QDEBUG_OPERATOR_FOR_CF_TYPE);
QT_FOR_EACH_MUTABLE_CORE_FOUNDATION_TYPE(QT_DECLARE_WEAK_QDEBUG_OPERATOR_FOR_CF_TYPE);
QT_FOR_EACH_CORE_GRAPHICS_TYPE(QT_DECLARE_WEAK_QDEBUG_OPERATOR_FOR_CF_TYPE);
QT_FOR_EACH_MUTABLE_CORE_GRAPHICS_TYPE(QT_DECLARE_WEAK_QDEBUG_OPERATOR_FOR_CF_TYPE);

// -------------------------------------------------------------------------

QMacAutoReleasePool::QMacAutoReleasePool()
    : pool([[NSAutoreleasePool alloc] init])
{
}

QMacAutoReleasePool::~QMacAutoReleasePool()
{
    // Drain behaves the same as release, with the advantage that
    // if we're ever used in a garbage-collected environment, the
    // drain acts as a hint to the garbage collector to collect.
    [static_cast<NSAutoreleasePool*>(pool) drain];
}

// -------------------------------------------------------------------------

#ifdef Q_OS_OSX

// Use this method to keep all the information in the TextSegment. As long as it is ordered
// we are in OK shape, and we can influence that ourselves.
struct KeyPair
{
    QChar cocoaKey;
    Qt::Key qtKey;
};

bool operator==(const KeyPair &entry, QChar qchar)
{
    return entry.cocoaKey == qchar;
}

bool operator<(const KeyPair &entry, QChar qchar)
{
    return entry.cocoaKey < qchar;
}

bool operator<(QChar qchar, const KeyPair &entry)
{
    return qchar < entry.cocoaKey;
}

bool operator<(const Qt::Key &key, const KeyPair &entry)
{
    return key < entry.qtKey;
}

bool operator<(const KeyPair &entry, const Qt::Key &key)
{
    return entry.qtKey < key;
}

struct qtKey2CocoaKeySortLessThan
{
    typedef bool result_type;
    Q_DECL_CONSTEXPR result_type operator()(const KeyPair &entry1, const KeyPair &entry2) const Q_DECL_NOTHROW
    {
        return entry1.qtKey < entry2.qtKey;
    }
};

static const int NumEntries = 59;
static const KeyPair entries[NumEntries] = {
    { NSEnterCharacter, Qt::Key_Enter },
    { NSBackspaceCharacter, Qt::Key_Backspace },
    { NSTabCharacter, Qt::Key_Tab },
    { NSNewlineCharacter, Qt::Key_Return },
    { NSCarriageReturnCharacter, Qt::Key_Return },
    { NSBackTabCharacter, Qt::Key_Backtab },
    { kEscapeCharCode, Qt::Key_Escape },
    // Cocoa sends us delete when pressing backspace!
    // (NB when we reverse this list in qtKey2CocoaKey, there
    // will be two indices of Qt::Key_Backspace. But is seems to work
    // ok for menu shortcuts (which uses that function):
    { NSDeleteCharacter, Qt::Key_Backspace },
    { NSUpArrowFunctionKey, Qt::Key_Up },
    { NSDownArrowFunctionKey, Qt::Key_Down },
    { NSLeftArrowFunctionKey, Qt::Key_Left },
    { NSRightArrowFunctionKey, Qt::Key_Right },
    { NSF1FunctionKey, Qt::Key_F1 },
    { NSF2FunctionKey, Qt::Key_F2 },
    { NSF3FunctionKey, Qt::Key_F3 },
    { NSF4FunctionKey, Qt::Key_F4 },
    { NSF5FunctionKey, Qt::Key_F5 },
    { NSF6FunctionKey, Qt::Key_F6 },
    { NSF7FunctionKey, Qt::Key_F7 },
    { NSF8FunctionKey, Qt::Key_F8 },
    { NSF9FunctionKey, Qt::Key_F9 },
    { NSF10FunctionKey, Qt::Key_F10 },
    { NSF11FunctionKey, Qt::Key_F11 },
    { NSF12FunctionKey, Qt::Key_F12 },
    { NSF13FunctionKey, Qt::Key_F13 },
    { NSF14FunctionKey, Qt::Key_F14 },
    { NSF15FunctionKey, Qt::Key_F15 },
    { NSF16FunctionKey, Qt::Key_F16 },
    { NSF17FunctionKey, Qt::Key_F17 },
    { NSF18FunctionKey, Qt::Key_F18 },
    { NSF19FunctionKey, Qt::Key_F19 },
    { NSF20FunctionKey, Qt::Key_F20 },
    { NSF21FunctionKey, Qt::Key_F21 },
    { NSF22FunctionKey, Qt::Key_F22 },
    { NSF23FunctionKey, Qt::Key_F23 },
    { NSF24FunctionKey, Qt::Key_F24 },
    { NSF25FunctionKey, Qt::Key_F25 },
    { NSF26FunctionKey, Qt::Key_F26 },
    { NSF27FunctionKey, Qt::Key_F27 },
    { NSF28FunctionKey, Qt::Key_F28 },
    { NSF29FunctionKey, Qt::Key_F29 },
    { NSF30FunctionKey, Qt::Key_F30 },
    { NSF31FunctionKey, Qt::Key_F31 },
    { NSF32FunctionKey, Qt::Key_F32 },
    { NSF33FunctionKey, Qt::Key_F33 },
    { NSF34FunctionKey, Qt::Key_F34 },
    { NSF35FunctionKey, Qt::Key_F35 },
    { NSInsertFunctionKey, Qt::Key_Insert },
    { NSDeleteFunctionKey, Qt::Key_Delete },
    { NSHomeFunctionKey, Qt::Key_Home },
    { NSEndFunctionKey, Qt::Key_End },
    { NSPageUpFunctionKey, Qt::Key_PageUp },
    { NSPageDownFunctionKey, Qt::Key_PageDown },
    { NSPrintScreenFunctionKey, Qt::Key_Print },
    { NSScrollLockFunctionKey, Qt::Key_ScrollLock },
    { NSPauseFunctionKey, Qt::Key_Pause },
    { NSSysReqFunctionKey, Qt::Key_SysReq },
    { NSMenuFunctionKey, Qt::Key_Menu },
    { NSHelpFunctionKey, Qt::Key_Help },
};
static const KeyPair * const end = entries + NumEntries;

QChar qt_mac_qtKey2CocoaKey(Qt::Key key)
{
    // The first time this function is called, create a reverse
    // lookup table sorted on Qt Key rather than Cocoa key:
    static QVector<KeyPair> rev_entries(NumEntries);
    static bool mustInit = true;
    if (mustInit){
        mustInit = false;
        for (int i=0; i<NumEntries; ++i)
            rev_entries[i] = entries[i];
        std::sort(rev_entries.begin(), rev_entries.end(), qtKey2CocoaKeySortLessThan());
    }
    const QVector<KeyPair>::iterator i
            = std::lower_bound(rev_entries.begin(), rev_entries.end(), key);
    if ((i == rev_entries.end()) || (key < *i))
        return QChar();
    return i->cocoaKey;
}

Qt::Key qt_mac_cocoaKey2QtKey(QChar keyCode)
{
    const KeyPair *i = std::lower_bound(entries, end, keyCode);
    if ((i == end) || (keyCode < *i))
        return Qt::Key(keyCode.toUpper().unicode());
    return i->qtKey;
}

#endif // Q_OS_OSX

// -------------------------------------------------------------------------

QT_END_NAMESPACE

