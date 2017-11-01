/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the test suite of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <qdebug.h>
#include <QtTest/QtTest>

#include <qglobal.h>

class tst_QGetPutEnv : public QObject
{
Q_OBJECT
private slots:
    void getSetCheck();
    void intValue_data();
    void intValue();
};

void tst_QGetPutEnv::getSetCheck()
{
    const char varName[] = "should_not_exist";

    bool ok;

    QVERIFY(!qEnvironmentVariableIsSet(varName));
    QVERIFY(qEnvironmentVariableIsEmpty(varName));
    ok = true;
    QCOMPARE(qEnvironmentVariableIntValue(varName), 0);
    QCOMPARE(qEnvironmentVariableIntValue(varName, &ok), 0);
    QVERIFY(!ok);
    QByteArray result = qgetenv(varName);
    QCOMPARE(result, QByteArray());

#ifndef Q_OS_WIN
    QVERIFY(qputenv(varName, "")); // deletes varName instead of making it empty, on Windows

    QVERIFY(qEnvironmentVariableIsSet(varName));
    QVERIFY(qEnvironmentVariableIsEmpty(varName));
    ok = true;
    QCOMPARE(qEnvironmentVariableIntValue(varName), 0);
    QCOMPARE(qEnvironmentVariableIntValue(varName, &ok), 0);
    QVERIFY(!ok);
#endif

    QVERIFY(qputenv(varName, QByteArray("supervalue")));

    QVERIFY(qEnvironmentVariableIsSet(varName));
    QVERIFY(!qEnvironmentVariableIsEmpty(varName));
    ok = true;
    QCOMPARE(qEnvironmentVariableIntValue(varName), 0);
    QCOMPARE(qEnvironmentVariableIntValue(varName, &ok), 0);
    QVERIFY(!ok);
    result = qgetenv(varName);
    QCOMPARE(result, QByteArrayLiteral("supervalue"));

    qputenv(varName,QByteArray());

    // Now test qunsetenv
    QVERIFY(qunsetenv(varName));
    QVERIFY(!qEnvironmentVariableIsSet(varName));
    QVERIFY(qEnvironmentVariableIsEmpty(varName));
    ok = true;
    QCOMPARE(qEnvironmentVariableIntValue(varName), 0);
    QCOMPARE(qEnvironmentVariableIntValue(varName, &ok), 0);
    QVERIFY(!ok);
    result = qgetenv(varName);
    QCOMPARE(result, QByteArray());
}

void tst_QGetPutEnv::intValue_data()
{
    QTest::addColumn<QByteArray>("value");
    QTest::addColumn<int>("expected");
    QTest::addColumn<bool>("ok");

    // some repetition from what is tested in getSetCheck()
    QTest::newRow("empty") << QByteArray() << 0 << false;
    QTest::newRow("spaces-heading") << QByteArray(" 1") << 1 << true;
    QTest::newRow("spaces-trailing") << QByteArray("1 ") << 0 << false;

#define ROW(x, i, b) \
    QTest::newRow(#x) << QByteArray(#x) << (i) << (b)
    ROW(auto, 0, false);
    ROW(1auto, 0, false);
    ROW(0, 0, true);
    ROW(+0, 0, true);
    ROW(1, 1, true);
    ROW(+1, 1, true);
    ROW(09, 0, false);
    ROW(010, 8, true);
    ROW(0x10, 16, true);
    ROW(0x, 0, false);
    ROW(0xg, 0, false);
    ROW(0x1g, 0, false);
    ROW(000000000000000000000000000000000000000000000000001, 0, false);
    ROW(+000000000000000000000000000000000000000000000000001, 0, false);
    ROW(000000000000000000000000000000000000000000000000001g, 0, false);
    ROW(-0, 0, true);
    ROW(-1, -1, true);
    ROW(-010, -8, true);
    ROW(-000000000000000000000000000000000000000000000000001, 0, false);
    ROW(2147483648, 0, false);
    // ROW(0xffffffff, -1, true); // could be expected, but not how QByteArray::toInt() works
    ROW(0xffffffff, 0, false);
    const int bases[] = {10, 8, 16};
    for (size_t i = 0; i < sizeof bases / sizeof *bases; ++i) {
        QTest::addRow("INT_MAX, base %d", bases[i])
                << QByteArray::number(INT_MAX) << INT_MAX << true;
        QTest::addRow("INT_MAX+1, base %d", bases[i])
                << QByteArray::number(qlonglong(INT_MAX) + 1) << 0 << false;
        QTest::addRow("INT_MIN, base %d", bases[i])
                << QByteArray::number(INT_MIN) << INT_MIN << true;
        QTest::addRow("INT_MIN-1, base %d", bases[i])
                << QByteArray::number(qlonglong(INT_MIN) - 1) << 0 << false;
    };
}

void tst_QGetPutEnv::intValue()
{
    const int maxlen = (sizeof(int) * CHAR_BIT + 2) / 3;
    const char varName[] = "should_not_exist";

    QFETCH(QByteArray, value);
    QFETCH(int, expected);
    QFETCH(bool, ok);

    bool actualOk = !ok;

    // Self-test: confirm that it was like the docs said it should be
    if (value.length() < maxlen) {
        QCOMPARE(value.toInt(&actualOk, 0), expected);
        QCOMPARE(actualOk, ok);
    }

    actualOk = !ok;
    QVERIFY(qputenv(varName, value));
    QCOMPARE(qEnvironmentVariableIntValue(varName), expected);
    QCOMPARE(qEnvironmentVariableIntValue(varName, &actualOk), expected);
    QCOMPARE(actualOk, ok);
}

QTEST_MAIN(tst_QGetPutEnv)
#include "tst_qgetputenv.moc"
