/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the plugins of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qaccessiblemenu.h"

#include <qmenu.h>
#include <qmenubar.h>
#include <QtWidgets/QAction>
#include <qstyle.h>

#ifndef QT_NO_ACCESSIBILITY

QT_BEGIN_NAMESPACE

#ifndef QT_NO_MENU

QString Q_GUI_EXPORT qt_accStripAmp(const QString &text);
QString Q_GUI_EXPORT qt_accHotKey(const QString &text);

QAccessibleMenu::QAccessibleMenu(QWidget *w)
: QAccessibleWidget(w)
{
    Q_ASSERT(menu());
}

QMenu *QAccessibleMenu::menu() const
{
    return qobject_cast<QMenu*>(object());
}

int QAccessibleMenu::childCount() const
{
    return menu()->actions().count();
}

QRect QAccessibleMenu::rect(int child) const
{
    Q_ASSERT(child == 0);
    return QAccessibleWidget::rect(child);
}

int QAccessibleMenu::childAt(int x, int y) const
{
    QAction *act = menu()->actionAt(menu()->mapFromGlobal(QPoint(x,y)));
    if(act && act->isSeparator())
        act = 0;
    return menu()->actions().indexOf(act) + 1;
}

QString QAccessibleMenu::text(Text t, int child) const
{
    Q_ASSERT(child == 0);
    QString tx = QAccessibleWidget::text(t, child);
    if (tx.size())
        return tx;

    if (t == Name)
        return menu()->windowTitle();
    return tx;
}

QAccessible::Role QAccessibleMenu::role(int child) const
{
    Q_ASSERT(child == 0);
    return PopupMenu;
}

QAccessible::State QAccessibleMenu::state(int child) const
{
    Q_ASSERT(child == 0);
    State s = QAccessibleWidget::state(child);
    return s;
}

QAccessibleInterface *QAccessibleMenu::child(int index) const
{
    if (index < childCount())
        return new QAccessibleMenuItem(menu(), menu()->actions().at(index));
    return 0;
}

QAccessibleInterface *QAccessibleMenu::parent() const
{
    QWidget *parent = menu()->parentWidget();
    if (qobject_cast<QMenu*>(parent) || qobject_cast<QMenuBar*>(parent)) {
        return new QAccessibleMenuItem(parent, menu()->menuAction());
    }
    return QAccessibleWidget::parent();
}

int QAccessibleMenu::navigate(RelationFlag relation, int entry, QAccessibleInterface **target) const
{
    Q_ASSERT(entry >= 0);
    switch (relation) {
    case Child:
        *target = child(entry - 1);
        return *target ? 0 : -1;
    case Ancestor:
        *target = parent();
        return *target ? 0 : -1;
    default:
        return QAccessibleWidget::navigate(relation, entry, target);
    }
}

int QAccessibleMenu::indexOfChild( const QAccessibleInterface *child ) const
{
    int index = -1;
    Role r = child->role(0);
    if ((r == MenuItem || r == Separator) && menu()) {
        index = menu()->actions().indexOf(qobject_cast<QAction*>(child->object()));
        if (index != -1)
            ++index;
    }
    return index;
}

#ifndef QT_NO_MENUBAR
QAccessibleMenuBar::QAccessibleMenuBar(QWidget *w)
: QAccessibleWidget(w)
{
    Q_ASSERT(menuBar());
}

QMenuBar *QAccessibleMenuBar::menuBar() const
{
    return qobject_cast<QMenuBar*>(object());
}

int QAccessibleMenuBar::childCount() const
{
    return menuBar()->actions().count();
}

QRect QAccessibleMenuBar::rect(int child) const
{
    Q_ASSERT(child == 0);
    return QAccessibleWidget::rect(child);
}

QAccessibleInterface *QAccessibleMenuBar::child(int index) const
{
    if (index < childCount())
        return new QAccessibleMenuItem(menuBar(), menuBar()->actions().at(index));
    return 0;
}

int QAccessibleMenuBar::navigate(RelationFlag relation, int entry, QAccessibleInterface **target) const
{
    if (relation == Child) {
        *target = child(entry - 1);
        return *target ? 0 : -1;
    }
    return QAccessibleWidget::navigate(relation, entry, target);
}

int QAccessibleMenuBar::indexOfChild(const QAccessibleInterface *child) const
{
    int index = -1;
    Role r = child->role(0);
    if ((r == MenuItem || r == Separator) && menuBar()) {
        index = menuBar()->actions().indexOf(qobject_cast<QAction*>(child->object()));
        if (index != -1)
            ++index;
    }
    return index;
}

QString QAccessibleMenuBar::text(Text t, int child) const
{
    Q_ASSERT(child == 0);
    return QAccessibleWidget::text(t, child);
}

QAccessible::Role QAccessibleMenuBar::role(int child) const
{
    Q_ASSERT(child == 0);
    return MenuBar;
}

QAccessible::State QAccessibleMenuBar::state(int child) const
{
    Q_ASSERT(child == 0);
    State s = QAccessibleWidget::state(child);
    return s;
}

#endif // QT_NO_MENUBAR

QAccessibleMenuItem::QAccessibleMenuItem(QWidget *owner, QAction *action) : m_action(action), m_owner(owner)
{
}

QAccessibleMenuItem::~QAccessibleMenuItem()
{}

int QAccessibleMenuItem::childAt(int x, int y ) const
{
    for (int i = childCount(); i >= 0; --i) {
        if (rect(i).contains(x,y))
            return i;
    }
    return -1;
}

int QAccessibleMenuItem::childCount() const
{
    return m_action->menu() ? 1 : 0;
}

int QAccessibleMenuItem::indexOfChild(const QAccessibleInterface * child) const
{
    Q_ASSERT(child == 0);
    if (child->role(0) == PopupMenu && child->object() == m_action->menu())
        return 1;

    return -1;
}

bool QAccessibleMenuItem::isValid() const
{
    return m_action ? true : false;
}

QAccessibleInterface *QAccessibleMenuItem::parent() const
{
    return QAccessible::queryAccessibleInterface(owner());
}

QAccessibleInterface *QAccessibleMenuItem::child(int index) const
{
    if (index == 0 && action()->menu())
        return new QAccessibleMenu(action()->menu());
    return 0;
}

int QAccessibleMenuItem::navigate(RelationFlag relation, int entry, QAccessibleInterface **target) const
{
    if (entry < 0) {
        *target = 0;
        return -1;
    }

    switch (relation) {
    case Child:
        *target = child(entry - 1);
        break;
    case Ancestor:
        *target = parent();
        break;
    case Up:
    case Down:{
        QAccessibleInterface *parentIface = parent();
        if (parentIface) {
            int index = parentIface->indexOfChild(this);
            if (index != -1) {
                index += (relation == Down ? +1 : -1);
                *target = parentIface->child(index - 1);
            }
        }
        delete parentIface;
        break;
    }
    case Sibling: {
        QAccessibleInterface *parentIface = parent();
        if (parentIface)
            *target = parentIface->child(entry - 1);
        delete parentIface;
        break;
    }
    default:
        break;

    }
    return *target ? 0 : -1;
}

QObject *QAccessibleMenuItem::object() const
{
    return m_action;
}

QRect QAccessibleMenuItem::rect(int child) const
{
    Q_ASSERT(child == 0);
    QRect rect;
    QWidget *own = owner();
#ifndef QT_NO_MENUBAR
    if (QMenuBar *menuBar = qobject_cast<QMenuBar*>(own)) {
        rect = menuBar->actionGeometry(m_action);
        QPoint globalPos = menuBar->mapToGlobal(QPoint(0,0));
        rect = rect.translated(globalPos);
    } else
#endif // QT_NO_MENUBAR
    if (QMenu *menu = qobject_cast<QMenu*>(own)) {
        rect = menu->actionGeometry(m_action);
        QPoint globalPos = menu->mapToGlobal(QPoint(0,0));
        rect = rect.translated(globalPos);
    }
    return rect;
}

QAccessible::Relation QAccessibleMenuItem::relationTo ( int child, const QAccessibleInterface * other, int otherChild ) const
{
    Q_ASSERT(child == 0);
    if (other->object() == owner()) {
        return Child;
    }
    Q_UNUSED(child)
    Q_UNUSED(other)
    Q_UNUSED(otherChild)
    // ###
    return Unrelated;
}

QAccessible::Role QAccessibleMenuItem::role(int) const
{
    return m_action->isSeparator() ? Separator : MenuItem;
}

void QAccessibleMenuItem::setText ( Text /*t*/, int /*child*/, const QString & /*text */)
{
}

QAccessible::State QAccessibleMenuItem::state(int child) const
{
    Q_ASSERT(child == 0);
    QAccessible::State s = Normal;
    QWidget *own = owner();

    if (own->testAttribute(Qt::WA_WState_Visible) == false || m_action->isVisible() == false) {
        s |= Invisible;
    }

    if (QMenu *menu = qobject_cast<QMenu*>(own)) {
        if (menu->activeAction() == m_action)
            s |= Focused;
#ifndef QT_NO_MENUBAR
    } else if (QMenuBar *menuBar = qobject_cast<QMenuBar*>(own)) {
        if (menuBar->activeAction() == m_action)
            s |= Focused;
#endif
    }
    if (own->style()->styleHint(QStyle::SH_Menu_MouseTracking))
        s |= HotTracked;
    if (m_action->isSeparator() || !m_action->isEnabled())
        s |= Unavailable;
    if (m_action->isChecked())
        s |= Checked;

    return s;
}

QString QAccessibleMenuItem::text ( Text t, int child ) const
{
    Q_ASSERT(child == 0);
    QString str;
    switch (t) {
    case Name:
        str = m_action->text();
        str = qt_accStripAmp(str);
        break;
    case Accelerator: {
#ifndef QT_NO_SHORTCUT
        QKeySequence key = m_action->shortcut();
        if (!key.isEmpty()) {
            str = key.toString();
        } else
#endif
        {
            str = qt_accHotKey(m_action->text());
        }
        break;
    }
    default:
        break;
    }
    return str;
}


QString QAccessibleMenuItem::actionText(int action, Text text, int child) const
{
    Q_ASSERT(child == 0);
    if (!m_action || m_action->isSeparator())
        return QString();

    if (text == Name && ((action == Press) || (action == DefaultAction))) {
        if (m_action->menu()) {
            return QMenu::tr("Open");
        }
        return QMenu::tr("Execute");
    }
    return QString();
}

QStringList QAccessibleMenuItem::actionNames() const
{
    QStringList actions;
    if (!m_action || m_action->isSeparator())
        return actions;

    if (m_action->menu()) {
        actions << showMenuAction();
    } else {
        actions << pressAction();
    }
    return actions;
}

void QAccessibleMenuItem::doAction(const QString &actionName)
{
    if (!m_action->isEnabled())
        return;

    if (actionName == pressAction()) {
        m_action->trigger();
    } else if (actionName == showMenuAction()) {
        if (QMenuBar *bar = qobject_cast<QMenuBar*>(owner())) {
            if (m_action->menu() && m_action->menu()->isVisible()) {
                m_action->menu()->hide();
            } else {
                bar->setActiveAction(m_action);
            }
        } else if (QMenu *menu = qobject_cast<QMenu*>(owner())){
            if (m_action->menu() && m_action->menu()->isVisible()) {
                m_action->menu()->hide();
            } else {
                menu->setActiveAction(m_action);
            }
        }
    }
}

QStringList QAccessibleMenuItem::keyBindingsForAction(const QString &) const
{
    return QStringList();
}


QAction *QAccessibleMenuItem::action() const
{
    return m_action;
}

QWidget *QAccessibleMenuItem::owner() const
{
    return m_owner;
}

#endif // QT_NO_MENU

QT_END_NAMESPACE

#endif // QT_NO_ACCESSIBILITY

