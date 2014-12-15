/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the tools applications of the Qt Toolkit.
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

/*
  cppcodemarker.h
*/

#ifndef CPPCODEMARKER_H
#define CPPCODEMARKER_H

#include "codemarker.h"

QT_BEGIN_NAMESPACE

class CppCodeMarker : public CodeMarker
{
    Q_DECLARE_TR_FUNCTIONS(QDoc::CppCodeMarker)

public:
    CppCodeMarker();
    ~CppCodeMarker();

    virtual bool recognizeCode(const QString& code);
    virtual bool recognizeExtension(const QString& ext);
    virtual bool recognizeLanguage(const QString& lang);
    virtual Atom::Type atomType() const;
    virtual QString markedUpCode(const QString& code,
                                 const Node *relative,
                                 const Location &location);
    virtual QString markedUpSynopsis(const Node *node,
                                     const Node *relative,
                                     SynopsisStyle style);
    virtual QString markedUpQmlItem(const Node *node, bool summary);
    virtual QString markedUpName(const Node *node);
    virtual QString markedUpFullName(const Node *node, const Node *relative);
    virtual QString markedUpEnumValue(const QString &enumValue, const Node *relative);
    virtual QString markedUpIncludes(const QStringList& includes);
    virtual QString functionBeginRegExp(const QString& funcName);
    virtual QString functionEndRegExp(const QString& funcName);
    virtual QList<Section> sections(const InnerNode *innerNode,
                                    SynopsisStyle style,
                                    Status status);
    virtual QList<Section> qmlSections(QmlClassNode* qmlClassNode,
                                       SynopsisStyle style,
                                       Status status = Okay);

private:
    QString addMarkUp(const QString& protectedCode,
                      const Node *relative,
                      const Location &location);
};

QT_END_NAMESPACE

#endif
