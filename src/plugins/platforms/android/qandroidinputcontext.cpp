/****************************************************************************
**
** Copyright (C) 2012 BogDan Vatra <bogdan@kde.org>
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

#include <android/log.h>

#include "qandroidinputcontext.h"
#include "androidjnimain.h"
#include "androidjniinput.h"
#include <QDebug>
#include <qevent.h>
#include <qguiapplication.h>
#include <qsharedpointer.h>
#include <qthread.h>
#include <qinputmethod.h>
#include <qwindow.h>

#include <QTextCharFormat>

#include <QDebug>

QT_BEGIN_NAMESPACE

static QAndroidInputContext *m_androidInputContext = 0;
static char const *const QtNativeInputConnectionClassName = "org.qtproject.qt5.android.QtNativeInputConnection";
static char const *const QtExtractedTextClassName = "org.qtproject.qt5.android.QtExtractedText";
static jclass m_extractedTextClass = 0;
static jmethodID m_classConstructorMethodID = 0;
static jfieldID m_partialEndOffsetFieldID = 0;
static jfieldID m_partialStartOffsetFieldID = 0;
static jfieldID m_selectionEndFieldID = 0;
static jfieldID m_selectionStartFieldID = 0;
static jfieldID m_startOffsetFieldID = 0;
static jfieldID m_textFieldID = 0;

static jboolean commitText(JNIEnv *env, jobject /*thiz*/, jstring text, jint newCursorPosition)
{
    if (!m_androidInputContext)
        return JNI_FALSE;

    jboolean isCopy;
    const jchar *jstr = env->GetStringChars(text, &isCopy);
    QString str(reinterpret_cast<const QChar *>(jstr), env->GetStringLength(text));
    env->ReleaseStringChars(text, jstr);

#ifdef QT_DEBUG_ANDROID_IM_PROTOCOL
    qDebug() << "@@@ COMMIT" << str << newCursorPosition;
#endif
    return m_androidInputContext->commitText(str, newCursorPosition);
}

static jboolean deleteSurroundingText(JNIEnv */*env*/, jobject /*thiz*/, jint leftLength, jint rightLength)
{
    if (!m_androidInputContext)
        return JNI_FALSE;

#ifdef QT_DEBUG_ANDROID_IM_PROTOCOL
    qDebug() << "@@@ DELETE" << leftLength << rightLength;
#endif
    return m_androidInputContext->deleteSurroundingText(leftLength, rightLength);
}

static jboolean finishComposingText(JNIEnv */*env*/, jobject /*thiz*/)
{
    if (!m_androidInputContext)
        return JNI_FALSE;

#ifdef QT_DEBUG_ANDROID_IM_PROTOCOL
    qDebug() << "@@@ FINISH";
#endif
    return m_androidInputContext->finishComposingText();
}

static jint getCursorCapsMode(JNIEnv */*env*/, jobject /*thiz*/, jint reqModes)
{
    if (!m_androidInputContext)
        return 0;

    return m_androidInputContext->getCursorCapsMode(reqModes);
}

static jobject getExtractedText(JNIEnv *env, jobject /*thiz*/, int hintMaxChars, int hintMaxLines, jint flags)
{
    if (!m_androidInputContext)
        return 0;

#ifdef QT_DEBUG_ANDROID_IM_PROTOCOL
    qDebug() << "@@@ GETEX";
#endif
    const QAndroidInputContext::ExtractedText &extractedText =
            m_androidInputContext->getExtractedText(hintMaxChars, hintMaxLines, flags);

    jobject object = env->NewObject(m_extractedTextClass, m_classConstructorMethodID);
    env->SetIntField(object, m_partialStartOffsetFieldID, extractedText.partialStartOffset);
    env->SetIntField(object, m_partialEndOffsetFieldID, extractedText.partialEndOffset);
    env->SetIntField(object, m_selectionStartFieldID, extractedText.selectionStart);
    env->SetIntField(object, m_selectionEndFieldID, extractedText.selectionEnd);
    env->SetIntField(object, m_startOffsetFieldID, extractedText.startOffset);
    env->SetObjectField(object,
                        m_textFieldID,
                        env->NewString(reinterpret_cast<const jchar *>(extractedText.text.constData()),
                                       jsize(extractedText.text.length())));

    return object;
}

static jstring getSelectedText(JNIEnv *env, jobject /*thiz*/, jint flags)
{
    if (!m_androidInputContext)
        return 0;

    const QString &text = m_androidInputContext->getSelectedText(flags);
#ifdef QT_DEBUG_ANDROID_IM_PROTOCOL
    qDebug() << "@@@ GETSEL" << text;
#endif
    return env->NewString(reinterpret_cast<const jchar *>(text.constData()), jsize(text.length()));
}

static jstring getTextAfterCursor(JNIEnv *env, jobject /*thiz*/, jint length, jint flags)
{
    if (!m_androidInputContext)
        return 0;

    const QString &text = m_androidInputContext->getTextAfterCursor(length, flags);
#ifdef QT_DEBUG_ANDROID_IM_PROTOCOL
    qDebug() << "@@@ GETA" << length << text;
#endif
    return env->NewString(reinterpret_cast<const jchar *>(text.constData()), jsize(text.length()));
}

static jstring getTextBeforeCursor(JNIEnv *env, jobject /*thiz*/, jint length, jint flags)
{
    if (!m_androidInputContext)
        return 0;

    const QString &text = m_androidInputContext->getTextBeforeCursor(length, flags);
#ifdef QT_DEBUG_ANDROID_IM_PROTOCOL
    qDebug() << "@@@ GETB" << length << text;
#endif
    return env->NewString(reinterpret_cast<const jchar *>(text.constData()), jsize(text.length()));
}

static jboolean setComposingText(JNIEnv *env, jobject /*thiz*/, jstring text, jint newCursorPosition)
{
    if (!m_androidInputContext)
        return JNI_FALSE;

    jboolean isCopy;
    const jchar *jstr = env->GetStringChars(text, &isCopy);
    QString str(reinterpret_cast<const QChar *>(jstr), env->GetStringLength(text));
    env->ReleaseStringChars(text, jstr);

#ifdef QT_DEBUG_ANDROID_IM_PROTOCOL
    qDebug() << "@@@ SET" << str << newCursorPosition;
#endif
    return m_androidInputContext->setComposingText(str, newCursorPosition);
}

static jboolean setComposingRegion(JNIEnv */*env*/, jobject /*thiz*/, jint start, jint end)
{
    if (!m_androidInputContext)
        return JNI_FALSE;

#ifdef QT_DEBUG_ANDROID_IM_PROTOCOL
    qDebug() << "@@@ SETR" << start << end;
#endif
    return m_androidInputContext->setComposingRegion(start, end);
}


static jboolean setSelection(JNIEnv */*env*/, jobject /*thiz*/, jint start, jint end)
{
    if (!m_androidInputContext)
        return JNI_FALSE;

#ifdef QT_DEBUG_ANDROID_IM_PROTOCOL
    qDebug() << "@@@ SETSEL" << start << end;
#endif
    return m_androidInputContext->setSelection(start, end);
}

static jboolean selectAll(JNIEnv */*env*/, jobject /*thiz*/)
{
    if (!m_androidInputContext)
        return JNI_FALSE;

#ifdef QT_DEBUG_ANDROID_IM_PROTOCOL
    qDebug() << "@@@ SELALL";
#endif
    return m_androidInputContext->selectAll();
}

static jboolean cut(JNIEnv */*env*/, jobject /*thiz*/)
{
    if (!m_androidInputContext)
        return JNI_FALSE;

#ifdef QT_DEBUG_ANDROID_IM_PROTOCOL
    qDebug() << "@@@";
#endif
    return m_androidInputContext->cut();
}

static jboolean copy(JNIEnv */*env*/, jobject /*thiz*/)
{
    if (!m_androidInputContext)
        return JNI_FALSE;

#ifdef QT_DEBUG_ANDROID_IM_PROTOCOL
    qDebug() << "@@@";
#endif
    return m_androidInputContext->copy();
}

static jboolean copyURL(JNIEnv */*env*/, jobject /*thiz*/)
{
    if (!m_androidInputContext)
        return JNI_FALSE;

#ifdef QT_DEBUG_ANDROID_IM_PROTOCOL
    qDebug() << "@@@";
#endif
    return m_androidInputContext->copyURL();
}

static jboolean paste(JNIEnv */*env*/, jobject /*thiz*/)
{
    if (!m_androidInputContext)
        return JNI_FALSE;

#ifdef QT_DEBUG_ANDROID_IM_PROTOCOL
    qDebug() << "@@@";
#endif
    return m_androidInputContext->paste();
}

static jboolean updateCursorPosition(JNIEnv */*env*/, jobject /*thiz*/)
{
    if (!m_androidInputContext)
        return JNI_FALSE;

#ifdef QT_DEBUG_ANDROID_IM_PROTOCOL
    qDebug() << "@@@ UPDATECURSORPOS";
#endif
    m_androidInputContext->updateCursorPosition();
    return true;
}


static JNINativeMethod methods[] = {
    {"commitText", "(Ljava/lang/String;I)Z", (void *)commitText},
    {"deleteSurroundingText", "(II)Z", (void *)deleteSurroundingText},
    {"finishComposingText", "()Z", (void *)finishComposingText},
    {"getCursorCapsMode", "(I)I", (void *)getCursorCapsMode},
    {"getExtractedText", "(III)Lorg/qtproject/qt5/android/QtExtractedText;", (void *)getExtractedText},
    {"getSelectedText", "(I)Ljava/lang/String;", (void *)getSelectedText},
    {"getTextAfterCursor", "(II)Ljava/lang/String;", (void *)getTextAfterCursor},
    {"getTextBeforeCursor", "(II)Ljava/lang/String;", (void *)getTextBeforeCursor},
    {"setComposingText", "(Ljava/lang/String;I)Z", (void *)setComposingText},
    {"setComposingRegion", "(II)Z", (void *)setComposingRegion},
    {"setSelection", "(II)Z", (void *)setSelection},
    {"selectAll", "()Z", (void *)selectAll},
    {"cut", "()Z", (void *)cut},
    {"copy", "()Z", (void *)copy},
    {"copyURL", "()Z", (void *)copyURL},
    {"paste", "()Z", (void *)paste},
    {"updateCursorPosition", "()Z", (void *)updateCursorPosition}
};


QAndroidInputContext::QAndroidInputContext()
    : QPlatformInputContext(), m_blockUpdateSelection(false)
{
    QtAndroid::AttachedJNIEnv env;
    if (!env.jniEnv)
        return;

    jclass clazz = QtAndroid::findClass(QtNativeInputConnectionClassName, env.jniEnv);
    if (clazz == NULL) {
        qCritical() << "Native registration unable to find class '"
                    << QtNativeInputConnectionClassName
                    << "'";
        return;
    }

    if (env.jniEnv->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(methods[0])) < 0) {
        qCritical() << "RegisterNatives failed for '"
                    << QtNativeInputConnectionClassName
                    << "'";
        return;
    }

    clazz = QtAndroid::findClass(QtExtractedTextClassName, env.jniEnv);
    if (clazz == NULL) {
        qCritical() << "Native registration unable to find class '"
                    << QtExtractedTextClassName
                    << "'";
        return;
    }

    m_extractedTextClass = static_cast<jclass>(env.jniEnv->NewGlobalRef(clazz));
    m_classConstructorMethodID = env.jniEnv->GetMethodID(m_extractedTextClass, "<init>", "()V");
    if (m_classConstructorMethodID == NULL) {
        qCritical() << "GetMethodID failed";
        return;
    }

    m_partialEndOffsetFieldID = env.jniEnv->GetFieldID(m_extractedTextClass, "partialEndOffset", "I");
    if (m_partialEndOffsetFieldID == NULL) {
        qCritical() << "Can't find field partialEndOffset";
        return;
    }

    m_partialStartOffsetFieldID = env.jniEnv->GetFieldID(m_extractedTextClass, "partialStartOffset", "I");
    if (m_partialStartOffsetFieldID == NULL) {
        qCritical() << "Can't find field partialStartOffset";
        return;
    }

    m_selectionEndFieldID = env.jniEnv->GetFieldID(m_extractedTextClass, "selectionEnd", "I");
    if (m_selectionEndFieldID == NULL) {
        qCritical() << "Can't find field selectionEnd";
        return;
    }

    m_selectionStartFieldID = env.jniEnv->GetFieldID(m_extractedTextClass, "selectionStart", "I");
    if (m_selectionStartFieldID == NULL) {
        qCritical() << "Can't find field selectionStart";
        return;
    }

    m_startOffsetFieldID = env.jniEnv->GetFieldID(m_extractedTextClass, "startOffset", "I");
    if (m_startOffsetFieldID == NULL) {
        qCritical() << "Can't find field startOffset";
        return;
    }

    m_textFieldID = env.jniEnv->GetFieldID(m_extractedTextClass, "text", "Ljava/lang/String;");
    if (m_textFieldID == NULL) {
        qCritical() << "Can't find field text";
        return;
    }
    qRegisterMetaType<QInputMethodEvent *>("QInputMethodEvent*");
    qRegisterMetaType<QInputMethodQueryEvent *>("QInputMethodQueryEvent*");
    m_androidInputContext = this;
}

QAndroidInputContext::~QAndroidInputContext()
{
    m_androidInputContext = 0;
    m_extractedTextClass = 0;
    m_partialEndOffsetFieldID = 0;
    m_partialStartOffsetFieldID = 0;
    m_selectionEndFieldID = 0;
    m_selectionStartFieldID = 0;
    m_startOffsetFieldID = 0;
    m_textFieldID = 0;
}

QAndroidInputContext *QAndroidInputContext::androidInputContext()
{
    return m_androidInputContext;
}

void QAndroidInputContext::reset()
{
    clear();
    if (qGuiApp->focusObject())
        QtAndroidInput::resetSoftwareKeyboard();
    else
        QtAndroidInput::hideSoftwareKeyboard();
}

void QAndroidInputContext::commit()
{
    finishComposingText();
}

void QAndroidInputContext::updateCursorPosition()
{
    QSharedPointer<QInputMethodQueryEvent> query = focusObjectInputMethodQuery();
    if (!query.isNull() && !m_blockUpdateSelection) {
        // make sure it also works with editors that have not been updated to the new API
        QVariant absolutePos = query->value(Qt::ImAbsolutePosition);
        const int cursorPos = absolutePos.isValid() ? absolutePos.toInt() : query->value(Qt::ImCursorPosition).toInt();
        QtAndroidInput::updateSelection(cursorPos, cursorPos, -1, -1); //selection empty and no pre-edit text
    }
}

void QAndroidInputContext::update(Qt::InputMethodQueries queries)
{
    QSharedPointer<QInputMethodQueryEvent> query = focusObjectInputMethodQuery(queries);
    if (query.isNull())
        return;
#warning TODO extract the needed data from query
}

void QAndroidInputContext::invokeAction(QInputMethod::Action action, int cursorPosition)
{
#warning TODO Handle at least QInputMethod::ContextMenu action
    Q_UNUSED(action)
    Q_UNUSED(cursorPosition)
    //### click should be passed to the IM, but in the meantime it's better to ignore it than to do something wrong
    // if (action == QInputMethod::Click)
    //     commit();
}

QRectF QAndroidInputContext::keyboardRect() const
{
    return QPlatformInputContext::keyboardRect();
}

bool QAndroidInputContext::isAnimating() const
{
    return false;
}

void QAndroidInputContext::showInputPanel()
{
    QSharedPointer<QInputMethodQueryEvent> query = focusObjectInputMethodQuery();
    if (query.isNull())
        return;

    disconnect(m_updateCursorPosConnection);
    if (qGuiApp->focusObject()->metaObject()->indexOfSignal("cursorPositionChanged(int,int)") >= 0) // QLineEdit breaks the pattern
        m_updateCursorPosConnection = connect(qGuiApp->focusObject(), SIGNAL(cursorPositionChanged(int,int)), this, SLOT(updateCursorPosition()));
    else
        m_updateCursorPosConnection = connect(qGuiApp->focusObject(), SIGNAL(cursorPositionChanged()), this, SLOT(updateCursorPosition()));
    QRectF itemRect = qGuiApp->inputMethod()->inputItemRectangle();
    QRect rect = qGuiApp->inputMethod()->inputItemTransform().mapRect(itemRect).toRect();
    QWindow *window = qGuiApp->focusWindow();
    if (window)
        rect = QRect(window->mapToGlobal(rect.topLeft()), rect.size());

    QtAndroidInput::showSoftwareKeyboard(rect.left(),
                                         rect.top(),
                                         rect.width(),
                                         rect.height(),
                                         query->value(Qt::ImHints).toUInt());
}

void QAndroidInputContext::hideInputPanel()
{
    QtAndroidInput::hideSoftwareKeyboard();
}

bool QAndroidInputContext::isInputPanelVisible() const
{
    return QtAndroidInput::isSoftwareKeyboardVisible();
}

bool QAndroidInputContext::isComposing() const
{
    return m_composingText.length();
}

void QAndroidInputContext::clear()
{
    m_composingText.clear();
    m_extractedText.clear();
}

void QAndroidInputContext::sendEvent(QObject *receiver, QInputMethodEvent *event)
{
    QCoreApplication::sendEvent(receiver, event);
}

void QAndroidInputContext::sendEvent(QObject *receiver, QInputMethodQueryEvent *event)
{
    QCoreApplication::sendEvent(receiver, event);
}

jboolean QAndroidInputContext::commitText(const QString &text, jint /*newCursorPosition*/)
{
    m_composingText = text;
    return finishComposingText();
}

jboolean QAndroidInputContext::deleteSurroundingText(jint leftLength, jint rightLength)
{
    QSharedPointer<QInputMethodQueryEvent> query = focusObjectInputMethodQuery();
    if (query.isNull())
        return JNI_TRUE;

    m_composingText.clear();

    QInputMethodEvent event;
    event.setCommitString(QString(), -leftLength, leftLength+rightLength);
    sendInputMethodEvent(&event);
    clear();

    return JNI_TRUE;
}

jboolean QAndroidInputContext::finishComposingText()
{
    QInputMethodEvent event;
    event.setCommitString(m_composingText);
    sendInputMethodEvent(&event);
    clear();

    return JNI_TRUE;
}

jint QAndroidInputContext::getCursorCapsMode(jint /*reqModes*/)
{
    jint res = 0;
    QSharedPointer<QInputMethodQueryEvent> query = focusObjectInputMethodQuery();
    if (query.isNull())
        return res;

    const uint qtInputMethodHints = query->value(Qt::ImHints).toUInt();

    if (qtInputMethodHints & Qt::ImhPreferUppercase)
        res = CAP_MODE_SENTENCES;

    if (qtInputMethodHints & Qt::ImhUppercaseOnly)
        res = CAP_MODE_CHARACTERS;

    return res;
}

const QAndroidInputContext::ExtractedText &QAndroidInputContext::getExtractedText(jint hintMaxChars, jint /*hintMaxLines*/, jint /*flags*/)
{
    QSharedPointer<QInputMethodQueryEvent> query = focusObjectInputMethodQuery();
    if (query.isNull())
        return m_extractedText;

    if (hintMaxChars)
        m_extractedText.text = query->value(Qt::ImSurroundingText).toString().right(hintMaxChars);

    m_extractedText.startOffset = query->value(Qt::ImCursorPosition).toInt();
    const QString &selection = query->value(Qt::ImCurrentSelection).toString();
    const int selLen = selection.length();
    if (selLen) {
        m_extractedText.selectionStart = query->value(Qt::ImAnchorPosition).toInt();
        m_extractedText.selectionEnd = m_extractedText.startOffset;
    }

    return m_extractedText;
}

QString QAndroidInputContext::getSelectedText(jint /*flags*/)
{
    QSharedPointer<QInputMethodQueryEvent> query = focusObjectInputMethodQuery();
    if (query.isNull())
        return QString();

    return query->value(Qt::ImCurrentSelection).toString();
}

QString QAndroidInputContext::getTextAfterCursor(jint length, jint /*flags*/)
{
    QVariant textAfter = queryFocusObjectThreadSafe(Qt::ImTextAfterCursor, QVariant(length));
    if (textAfter.isValid()) {
        return textAfter.toString().left(length);
    }

    //compatibility code for old controls that do not implement the new API
    QSharedPointer<QInputMethodQueryEvent> query = focusObjectInputMethodQuery();
    if (query.isNull())
        return QString();

    QString text = query->value(Qt::ImSurroundingText).toString();
    if (!text.length())
        return text;

    int cursorPos = query->value(Qt::ImCursorPosition).toInt();
    return text.mid(cursorPos, length);
}

QString QAndroidInputContext::getTextBeforeCursor(jint length, jint /*flags*/)
{
    QVariant textBefore = queryFocusObjectThreadSafe(Qt::ImTextBeforeCursor, QVariant(length));
    if (textBefore.isValid()) {
        return textBefore.toString().left(length);
    }

    //compatibility code for old controls that do not implement the new API
    QSharedPointer<QInputMethodQueryEvent> query = focusObjectInputMethodQuery();
    if (query.isNull())
        return QString();

    int cursorPos = query->value(Qt::ImCursorPosition).toInt();
    QString text = query->value(Qt::ImSurroundingText).toString();
    if (!text.length())
        return text;

    const int wordLeftPos = cursorPos - length;
    return text.mid(wordLeftPos > 0 ? wordLeftPos : 0, cursorPos);
}

jboolean QAndroidInputContext::setComposingText(const QString &text, jint newCursorPosition)
{
    if (newCursorPosition > 0)
        newCursorPosition += text.length() - 1;
    m_composingText = text;
    QList<QInputMethodEvent::Attribute> attributes;
    attributes.append(QInputMethodEvent::Attribute(QInputMethodEvent::Cursor,
                                                   newCursorPosition,
                                                   1,
                                                   QVariant()));
    // Show compose text underlined
    QTextCharFormat underlined;
    underlined.setFontUnderline(true);
    attributes.append(QInputMethodEvent::Attribute(QInputMethodEvent::TextFormat,0, text.length(),
                                                   QVariant(underlined)));

    QInputMethodEvent event(m_composingText, attributes);
    sendInputMethodEvent(&event);

    QSharedPointer<QInputMethodQueryEvent> query = focusObjectInputMethodQuery();
    if (!query.isNull() && !m_blockUpdateSelection) {
        QVariant absolutePos = query->value(Qt::ImAbsolutePosition);
        const int cursorPos = absolutePos.isValid() ? absolutePos.toInt() : query->value(Qt::ImCursorPosition).toInt();
        const int preeditLength = text.length();
        QtAndroidInput::updateSelection(cursorPos+preeditLength, cursorPos+preeditLength, cursorPos, cursorPos+preeditLength);
    }

    return JNI_TRUE;
}

// Android docs say:
// * start may be after end, same meaning as if swapped
// * this function must not trigger updateSelection
// * if start == end then we should stop composing
jboolean QAndroidInputContext::setComposingRegion(jint start, jint end)
{
    QSharedPointer<QInputMethodQueryEvent> query = focusObjectInputMethodQuery();
    if (query.isNull())
        return JNI_FALSE;

    if (start > end)
        qSwap(start, end);

    /*
      start and end are  cursor positions, not character positions,
      i.e. selecting the first character is done by start == 0 and end == 1,
      and start == end means no character selected

      Therefore, the length of the region is end - start
     */
    int length = end - start;
    int localPos = query->value(Qt::ImCursorPosition).toInt();
    QVariant absolutePos = query->value(Qt::ImAbsolutePosition);
    int blockPosition = absolutePos.isValid() ? absolutePos.toInt() - localPos : 0;
    int localStart = start - blockPosition; // Qt uses position inside block

    bool updateSelectionWasBlocked = m_blockUpdateSelection;
    m_blockUpdateSelection = true;

    QString text = query->value(Qt::ImSurroundingText).toString();
    m_composingText = text.mid(localStart, length);

    //in the Qt text controls, the cursor position is the start of the preedit
    int relativeStart = localStart - localPos;

    QList<QInputMethodEvent::Attribute> attributes;

    // Show compose text underlined
    QTextCharFormat underlined;
    underlined.setFontUnderline(true);
    attributes.append(QInputMethodEvent::Attribute(QInputMethodEvent::TextFormat,0, length,
                                                   QVariant(underlined)));

    // Keep the cursor position unchanged (don't move to end of preedit)
    attributes.append(QInputMethodEvent::Attribute(QInputMethodEvent::Cursor, localPos - localStart, length, QVariant()));

    QInputMethodEvent event(m_composingText, attributes);
    event.setCommitString(QString(), relativeStart, length);
    sendInputMethodEvent(&event);

    m_blockUpdateSelection = updateSelectionWasBlocked;
    return JNI_TRUE;
}

jboolean QAndroidInputContext::setSelection(jint start, jint end)
{
    QList<QInputMethodEvent::Attribute> attributes;
    attributes.append(QInputMethodEvent::Attribute(QInputMethodEvent::Selection,
                                                   start,
                                                   end - start,
                                                   QVariant()));

    QInputMethodEvent event(QString(), attributes);
    sendInputMethodEvent(&event);
    return JNI_TRUE;
}

jboolean QAndroidInputContext::selectAll()
{
#warning TODO
    return JNI_FALSE;
}

jboolean QAndroidInputContext::cut()
{
#warning TODO
    return JNI_FALSE;
}

jboolean QAndroidInputContext::copy()
{
#warning TODO
    return JNI_FALSE;
}

jboolean QAndroidInputContext::copyURL()
{
#warning TODO
    return JNI_FALSE;
}

jboolean QAndroidInputContext::paste()
{
#warning TODO
    return JNI_FALSE;
}


Q_INVOKABLE QVariant QAndroidInputContext::queryFocusObjectUnsafe(Qt::InputMethodQuery query, QVariant argument)
{
    return QInputMethod::queryFocusObject(query, argument);
}

QVariant QAndroidInputContext::queryFocusObjectThreadSafe(Qt::InputMethodQuery query, QVariant argument)
{
    bool inMainThread = qGuiApp->thread() == QThread::currentThread();
    QVariant retval;

    QMetaObject::invokeMethod(this, "queryFocusObjectUnsafe",
                              inMainThread ? Qt::DirectConnection : Qt::BlockingQueuedConnection,
                              Q_RETURN_ARG(QVariant, retval),
                              Q_ARG(Qt::InputMethodQuery, query),
                              Q_ARG(QVariant, argument));

    return retval;
}

QSharedPointer<QInputMethodQueryEvent> QAndroidInputContext::focusObjectInputMethodQuery(Qt::InputMethodQueries queries)
{
#warning TODO make qGuiApp->focusObject() thread safe !!!
    QObject *focusObject = qGuiApp->focusObject();
    if (!focusObject)
        return QSharedPointer<QInputMethodQueryEvent>();

    QSharedPointer<QInputMethodQueryEvent> ret = QSharedPointer<QInputMethodQueryEvent>(new QInputMethodQueryEvent(queries));
    if (qGuiApp->thread()==QThread::currentThread()) {
        QCoreApplication::sendEvent(focusObject, ret.data());
    } else {
        QMetaObject::invokeMethod(this,
                                  "sendEvent",
                                  Qt::BlockingQueuedConnection,
                                  Q_ARG(QObject*, focusObject),
                                  Q_ARG(QInputMethodQueryEvent*, ret.data()));
    }

    return ret;
}

void QAndroidInputContext::sendInputMethodEvent(QInputMethodEvent *event)
{
#warning TODO make qGuiApp->focusObject() thread safe !!!
    QObject *focusObject = qGuiApp->focusObject();
    if (!focusObject)
        return;

    if (qGuiApp->thread() == QThread::currentThread()) {
        QCoreApplication::sendEvent(focusObject, event);
    } else {
        QMetaObject::invokeMethod(this,
                                  "sendEvent",
                                  Qt::BlockingQueuedConnection,
                                  Q_ARG(QObject*, focusObject),
                                  Q_ARG(QInputMethodEvent*, event));
    }
}

QT_END_NAMESPACE
