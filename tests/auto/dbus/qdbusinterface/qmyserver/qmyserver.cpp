/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the test suite of the Qt Toolkit.
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
#include <QtCore/QtCore>
#include <QtDBus/QtDBus>

#include "../myobject.h"

static const char serviceName[] = "org.qtproject.autotests.qmyserver";
static const char objectPath[] = "/org/qtproject/qmyserver";
//static const char *interfaceName = serviceName;

int MyObject::callCount = 0;
QVariantList MyObject::callArgs;

class MyServer : public QDBusServer
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.qtproject.autotests.qmyserver")

public:
    MyServer(QString addr = "unix:tmpdir=/tmp", QObject* parent = 0)
        : QDBusServer(addr, parent),
          m_conn("none")
    {
        connect(this, SIGNAL(newConnection(QDBusConnection)), SLOT(handleConnection(QDBusConnection)));
    }

public slots:
    QString address() const
    {
        return QDBusServer::address();
    }

    bool isConnected() const
    {
        return m_conn.isConnected();
    }

    void emitSignal(const QString &interface, const QString &name, const QString &arg)
    {
        QDBusMessage msg = QDBusMessage::createSignal("/", interface, name);
        msg << arg;
        m_conn.send(msg);
    }

    void reset()
    {
        MyObject::callCount = 0;
        obj.m_complexProp.clear();
    }

    int callCount()
    {
        return MyObject::callCount;
    }

    QVariantList callArgs()
    {
        qDebug() << "callArgs" << MyObject::callArgs.count();
        return MyObject::callArgs;
    }

    void setProp1(int val)
    {
        obj.m_prop1 = val;
    }

    int prop1()
    {
        return obj.m_prop1;
    }

    void setComplexProp(QList<int> val)
    {
        obj.m_complexProp = val;
    }

    QList<int> complexProp()
    {
        return obj.m_complexProp;
    }


private slots:
    void handleConnection(const QDBusConnection& con)
    {
        m_conn = con;
        m_conn.registerObject("/", &obj, QDBusConnection::ExportAllProperties
                       | QDBusConnection::ExportAllSlots
                       | QDBusConnection::ExportAllInvokables);
    }

private:
    QDBusConnection m_conn;
    MyObject obj;
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QDBusConnection con = QDBusConnection::sessionBus();
    if (!con.isConnected())
        exit(1);

    if (!con.registerService(serviceName))
        exit(2);

    MyServer server;
    con.registerObject(objectPath, &server, QDBusConnection::ExportAllSlots);

    printf("ready.\n");

    return app.exec();
}

#include "qmyserver.moc"
