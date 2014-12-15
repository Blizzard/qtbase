/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
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

#ifndef QWINRTINPUTCONTEXT_H
#define QWINRTINPUTCONTEXT_H

#include <qpa/qplatforminputcontext.h>
#include <QtCore/QRectF>

#include <wrl.h>
#ifndef Q_OS_WINPHONE
#  include <UIAutomationCore.h>
#endif

namespace ABI {
    namespace Windows {
        namespace UI {
            namespace Core {
                struct ICoreWindow;
            }
            namespace ViewManagement {
                struct IInputPane;
                struct IInputPaneVisibilityEventArgs;
            }
        }
    }
}

QT_BEGIN_NAMESPACE

class QWinRTInputContext : public QPlatformInputContext
#ifndef Q_OS_WINPHONE
                         , public Microsoft::WRL::RuntimeClass<
                           Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::WinRtClassicComMix>,
                           IRawElementProviderSimple, ITextProvider, IValueProvider>
#endif // !Q_OS_WINPHONE
{
public:
    explicit QWinRTInputContext(ABI::Windows::UI::Core::ICoreWindow *window);

    QRectF keyboardRect() const;

    bool isInputPanelVisible() const;

#ifdef Q_OS_WINPHONE
    void showInputPanel();
    void hideInputPanel();
#else // Q_OS_WINPHONE
    // IRawElementProviderSimple
    HRESULT __stdcall get_ProviderOptions(ProviderOptions *retVal);
    HRESULT __stdcall GetPatternProvider(PATTERNID, IUnknown **);
    HRESULT __stdcall GetPropertyValue(PROPERTYID idProp, VARIANT *retVal);
    HRESULT __stdcall get_HostRawElementProvider(IRawElementProviderSimple **retVal);

    // ITextProvider
    HRESULT __stdcall GetSelection(SAFEARRAY **);
    HRESULT __stdcall GetVisibleRanges(SAFEARRAY **);
    HRESULT __stdcall RangeFromChild(IRawElementProviderSimple *,ITextRangeProvider **);
    HRESULT __stdcall RangeFromPoint(UiaPoint, ITextRangeProvider **);
    HRESULT __stdcall get_DocumentRange(ITextRangeProvider **);
    HRESULT __stdcall get_SupportedTextSelection(SupportedTextSelection *);

    // IValueProvider
    HRESULT __stdcall SetValue(LPCWSTR);
    HRESULT __stdcall get_Value(BSTR *);
    HRESULT __stdcall get_IsReadOnly(BOOL *);
#endif // !Q_OS_WINPHONE

private:
    HRESULT onShowing(ABI::Windows::UI::ViewManagement::IInputPane *,
                      ABI::Windows::UI::ViewManagement::IInputPaneVisibilityEventArgs *);
    HRESULT onHiding(ABI::Windows::UI::ViewManagement::IInputPane *,
                     ABI::Windows::UI::ViewManagement::IInputPaneVisibilityEventArgs *);
    void setKeyboardRect(const QRectF rect);

    ABI::Windows::UI::Core::ICoreWindow *m_window;
    QRectF m_keyboardRect;
    bool m_isInputPanelVisible;
};

QT_END_NAMESPACE

#endif // QWINRTINPUTCONTEXT_H
