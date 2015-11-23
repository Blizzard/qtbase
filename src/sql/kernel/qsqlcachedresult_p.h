/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtSql module of the Qt Toolkit.
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

#ifndef QSQLCACHEDRESULT_P_H
#define QSQLCACHEDRESULT_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of other Qt classes.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "QtSql/qsqlresult.h"
#include "QtSql/private/qsqlresult_p.h"

QT_BEGIN_NAMESPACE

class QVariant;
template <typename T> class QVector;

class QSqlCachedResultPrivate;

class Q_SQL_EXPORT QSqlCachedResult: public QSqlResult
{
    Q_DECLARE_PRIVATE(QSqlCachedResult)

public:
    typedef QVector<QVariant> ValueCache;

protected:
    QSqlCachedResult(QSqlCachedResultPrivate &d);

    void init(int colCount);
    void cleanup();
    void clearValues();

    virtual bool gotoNext(ValueCache &values, int index) = 0;

    QVariant data(int i) Q_DECL_OVERRIDE;
    bool isNull(int i) Q_DECL_OVERRIDE;
    bool fetch(int i) Q_DECL_OVERRIDE;
    bool fetchNext() Q_DECL_OVERRIDE;
    bool fetchPrevious() Q_DECL_OVERRIDE;
    bool fetchFirst() Q_DECL_OVERRIDE;
    bool fetchLast() Q_DECL_OVERRIDE;

    int colCount() const;
    ValueCache &cache();

    void virtual_hook(int id, void *data) Q_DECL_OVERRIDE;
    void detachFromResultSet() Q_DECL_OVERRIDE;
    void setNumericalPrecisionPolicy(QSql::NumericalPrecisionPolicy policy) Q_DECL_OVERRIDE;
private:
    bool cacheNext();
};

class Q_SQL_EXPORT QSqlCachedResultPrivate: public QSqlResultPrivate
{
    Q_DECLARE_PUBLIC(QSqlCachedResult)

public:
    QSqlCachedResultPrivate(QSqlCachedResult *q, const QSqlDriver *drv);
    bool canSeek(int i) const;
    inline int cacheCount() const;
    void init(int count, bool fo);
    void cleanup();
    int nextIndex();
    void revertLast();

    QSqlCachedResult::ValueCache cache;
    int rowCacheEnd;
    int colCount;
    bool forwardOnly;
    bool atEnd;
};

QT_END_NAMESPACE

#endif // QSQLCACHEDRESULT_P_H
