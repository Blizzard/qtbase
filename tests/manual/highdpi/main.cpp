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

#include <QMainWindow>
#include <QLabel>
#include <QHBoxLayout>
#include <QApplication>
#include <QAction>
#include <QStyle>
#include <QToolBar>
#include <QPushButton>
#include <QLineEdit>
#include <QScrollBar>
#include <QSlider>
#include <QSpinBox>
#include <QTabBar>
#include <QIcon>
#include <QPainter>
#include <QWindow>
#include <QScreen>
#include <QFile>
#include <QTemporaryDir>
#include <QCommandLineParser>
#include <QCommandLineOption>


class PixmapPainter : public QWidget
{
public:
    PixmapPainter();
    void paintEvent(QPaintEvent *event);

    QPixmap pixmap1X;
    QPixmap pixmap2X;
    QPixmap pixmapLarge;
    QImage image1X;
    QImage image2X;
    QImage imageLarge;
    QIcon qtIcon;
};


PixmapPainter::PixmapPainter()
{
    pixmap1X = QPixmap(":/qticon32.png");
    pixmap2X = QPixmap(":/qticon32@2x.png");
    pixmapLarge = QPixmap(":/qticon64.png");

    image1X = QImage(":/qticon32.png");
    image2X = QImage(":/qticon32@2x.png");
    imageLarge = QImage(":/qticon64.png");

    qtIcon.addFile(":/qticon32.png");
    qtIcon.addFile(":/qticon32@2x.png");
}

void PixmapPainter::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.fillRect(QRect(QPoint(0, 0), size()), QBrush(Qt::gray));

    int pixmapPointSize = 32;
    int y = 30;
    int dy = 90;

    int x = 10;
    int dx = 40;
    // draw at point
//          qDebug() << "paint pixmap" << pixmap1X.devicePixelRatio();
          p.drawPixmap(x, y, pixmap1X);
    x+=dx;p.drawPixmap(x, y, pixmap2X);
    x+=dx;p.drawPixmap(x, y, pixmapLarge);
  x+=dx*2;p.drawPixmap(x, y, qtIcon.pixmap(QSize(pixmapPointSize, pixmapPointSize)));
    x+=dx;p.drawImage(x, y, image1X);
    x+=dx;p.drawImage(x, y, image2X);
    x+=dx;p.drawImage(x, y, imageLarge);

    // draw at 32x32 rect
    y+=dy;
    x = 10;
          p.drawPixmap(QRect(x, y, pixmapPointSize, pixmapPointSize), pixmap1X);
    x+=dx;p.drawPixmap(QRect(x, y, pixmapPointSize, pixmapPointSize), pixmap2X);
    x+=dx;p.drawPixmap(QRect(x, y, pixmapPointSize, pixmapPointSize), pixmapLarge);
    x+=dx;p.drawPixmap(QRect(x, y, pixmapPointSize, pixmapPointSize), qtIcon.pixmap(QSize(pixmapPointSize, pixmapPointSize)));
    x+=dx;p.drawImage(QRect(x, y, pixmapPointSize, pixmapPointSize), image1X);
    x+=dx;p.drawImage(QRect(x, y, pixmapPointSize, pixmapPointSize), image2X);
    x+=dx;p.drawImage(QRect(x, y, pixmapPointSize, pixmapPointSize), imageLarge);


    // draw at 64x64 rect
    y+=dy - 50;
    x = 10;
               p.drawPixmap(QRect(x, y, pixmapPointSize * 2, pixmapPointSize * 2), pixmap1X);
    x+=dx * 2; p.drawPixmap(QRect(x, y, pixmapPointSize * 2, pixmapPointSize * 2), pixmap2X);
    x+=dx * 2; p.drawPixmap(QRect(x, y, pixmapPointSize * 2, pixmapPointSize * 2), pixmapLarge);
    x+=dx * 2; p.drawPixmap(QRect(x, y, pixmapPointSize * 2, pixmapPointSize * 2), qtIcon.pixmap(QSize(pixmapPointSize, pixmapPointSize)));
    x+=dx * 2; p.drawImage(QRect(x, y, pixmapPointSize * 2, pixmapPointSize * 2), image1X);
    x+=dx * 2; p.drawImage(QRect(x, y, pixmapPointSize * 2, pixmapPointSize * 2), image2X);
    x+=dx * 2; p.drawImage(QRect(x, y, pixmapPointSize * 2, pixmapPointSize * 2), imageLarge);
 }

class Labels : public QWidget
{
public:
    Labels();

    QPixmap pixmap1X;
    QPixmap pixmap2X;
    QPixmap pixmapLarge;
    QIcon qtIcon;
};

Labels::Labels()
{
    pixmap1X = QPixmap(":/qticon32.png");
    pixmap2X = QPixmap(":/qticon32@2x.png");
    pixmapLarge = QPixmap(":/qticon64.png");

    qtIcon.addFile(":/qticon32.png");
    qtIcon.addFile(":/qticon32@2x.png");
    setWindowIcon(qtIcon);
    setWindowTitle("Labels");

    QLabel *label1x = new QLabel();
    label1x->setPixmap(pixmap1X);
    QLabel *label2x = new QLabel();
    label2x->setPixmap(pixmap2X);
    QLabel *labelIcon = new QLabel();
    labelIcon->setPixmap(qtIcon.pixmap(QSize(32,32)));
    QLabel *labelLarge = new QLabel();
    labelLarge->setPixmap(pixmapLarge);

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->addWidget(label1x);    //expected low-res on high-dpi displays
    layout->addWidget(label2x);    //expected high-res on high-dpi displays
    layout->addWidget(labelIcon);  //expected high-res on high-dpi displays
    layout->addWidget(labelLarge); // expected large size and low-res
    setLayout(layout);
}

class MainWindow : public QMainWindow
{
public:
    MainWindow();

    QIcon qtIcon;
    QIcon qtIcon1x;
    QIcon qtIcon2x;

    QToolBar *fileToolBar;
};

MainWindow::MainWindow()
{
    // beware that QIcon auto-loads the @2x versions.
    qtIcon1x.addFile(":/qticon16.png");
    qtIcon2x.addFile(":/qticon32.png");
    setWindowIcon(qtIcon);
    setWindowTitle("MainWindow");

    fileToolBar = addToolBar(tr("File"));
//    fileToolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    fileToolBar->addAction(new QAction(qtIcon1x, QString("1x"), this));
    fileToolBar->addAction(new QAction(qtIcon2x, QString("2x"), this));
}

class StandardIcons : public QWidget
{
public:
    void paintEvent(QPaintEvent *)
    {
        int x = 10;
        int y = 10;
        int dx = 50;
        int dy = 50;
        int maxX = 500;

        for (int iconIndex = QStyle::SP_TitleBarMenuButton; iconIndex < QStyle::SP_MediaVolumeMuted; ++iconIndex) {
            QIcon icon = qApp->style()->standardIcon(QStyle::StandardPixmap(iconIndex));
            QPainter p(this);
            p.drawPixmap(x, y, icon.pixmap(dx - 5, dy - 5));
            if (x + dx > maxX)
                y+=dy;
            x = ((x + dx) % maxX);
        }
    }
};

class Caching : public QWidget
{
public:
    void paintEvent(QPaintEvent *)
    {
        QSize layoutSize(75, 75);

        QPainter widgetPainter(this);
        widgetPainter.fillRect(QRect(QPoint(0, 0), this->size()), Qt::gray);

        {
            const qreal devicePixelRatio = this->windowHandle()->devicePixelRatio();
            QPixmap cache(layoutSize * devicePixelRatio);
            cache.setDevicePixelRatio(devicePixelRatio);

            QPainter cachedPainter(&cache);
            cachedPainter.fillRect(QRect(0,0, 75, 75), Qt::blue);
            cachedPainter.fillRect(QRect(10,10, 55, 55), Qt::red);
            cachedPainter.drawEllipse(QRect(10,10, 55, 55));

            QPainter widgetPainter(this);
            widgetPainter.drawPixmap(QPoint(10, 10), cache);
        }

        {
            const qreal devicePixelRatio = this->windowHandle()->devicePixelRatio();
            QImage cache = QImage(layoutSize * devicePixelRatio, QImage::Format_ARGB32_Premultiplied);
            cache.setDevicePixelRatio(devicePixelRatio);

            QPainter cachedPainter(&cache);
            cachedPainter.fillRect(QRect(0,0, 75, 75), Qt::blue);
            cachedPainter.fillRect(QRect(10,10, 55, 55), Qt::red);
            cachedPainter.drawEllipse(QRect(10,10, 55, 55));

            QPainter widgetPainter(this);
            widgetPainter.drawImage(QPoint(95, 10), cache);
        }

    }
};

class Style : public QWidget {
public:
    QPushButton *button;
    QLineEdit *lineEdit;
    QSlider *slider;
    QHBoxLayout *row1;

    Style() {
        row1 = new QHBoxLayout();
        setLayout(row1);

        button = new QPushButton();
        button->setText("Test Button");
        row1->addWidget(button);

        lineEdit = new QLineEdit();
        lineEdit->setText("Test Lineedit");
        row1->addWidget(lineEdit);

        slider = new QSlider();
        row1->addWidget(slider);

        row1->addWidget(new QSpinBox);
        row1->addWidget(new QScrollBar);

        QTabBar *tab  = new QTabBar();
        tab->addTab("Foo");
        tab->addTab("Bar");
        row1->addWidget(tab);
    }
};

class Fonts : public QWidget
{
public:
    void paintEvent(QPaintEvent *)
    {
        QPainter painter(this);
        int y = 40;
        for (int fontSize = 2; fontSize < 18; fontSize += 2) {
            QFont font;
            font.setPointSize(fontSize);
            QString string = QString(QStringLiteral("%1 The quick brown fox jumped over the lazy Doug.")).arg(fontSize);
            painter.setFont(font);
            painter.drawText(10, y, string);
            y += (fontSize  * 2.5);
        }
    }
};


template <typename T>
void apiTestdevicePixelRatioGetter()
{
    if (0) {
        T *t = 0;
        t->devicePixelRatio();
    }
}

template <typename T>
void apiTestdevicePixelRatioSetter()
{
    if (0) {
        T *t = 0;
        t->setDevicePixelRatio(2.0);
    }
}

void apiTest()
{
    // compile call to devicePixelRatio getter and setter (verify spelling)
    apiTestdevicePixelRatioGetter<QWindow>();
    apiTestdevicePixelRatioGetter<QScreen>();
    apiTestdevicePixelRatioGetter<QGuiApplication>();

    apiTestdevicePixelRatioGetter<QImage>();
    apiTestdevicePixelRatioSetter<QImage>();
    apiTestdevicePixelRatioGetter<QPixmap>();
    apiTestdevicePixelRatioSetter<QPixmap>();
}

// Request and draw an icon at different sizes
class IconDrawing : public QWidget
{
public:
    IconDrawing()
    {
        const QString tempPath = m_temporaryDir.path();
        const QString path32 = tempPath + "/qticon32.png";
        const QString path32_2 = tempPath + "/qticon32-2.png";
        const QString path32_2x = tempPath + "/qticon32@2x.png";

        QFile::copy(":/qticon32.png", path32_2);
        QFile::copy(":/qticon32.png", path32);
        QFile::copy(":/qticon32@2x.png", path32_2x);

        iconHighDPI.reset(new QIcon(path32)); // will auto-load @2x version.
        iconNormalDpi.reset(new QIcon(path32_2)); // does not have a 2x version.
    }

    void paintEvent(QPaintEvent *)
    {
        int x = 10;
        int y = 10;
        int dx = 50;
        int dy = 50;
        int maxX = 600;
        int minSize = 5;
        int maxSize = 64;
        int sizeIncrement = 5;

        // Disable high-dpi icons
        QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps, false);

        // normal icon
        for (int size = minSize; size < maxSize; size += sizeIncrement) {
            QPainter p(this);
            p.drawPixmap(x, y, iconNormalDpi->pixmap(size, size));
            if (x + dx > maxX)
                y+=dy;
            x = ((x + dx) % maxX);
        }
        x = 10;
        y+=dy;

        // high-dpi icon
        for (int size = minSize; size < maxSize; size += sizeIncrement) {
            QPainter p(this);
            p.drawPixmap(x, y, iconHighDPI->pixmap(size, size));
            if (x + dx > maxX)
                y+=dy;
            x = ((x + dx) % maxX);
        }

        x = 10;
        y+=dy;

        // Enable high-dpi icons
        QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps, true);

        // normal icon
        for (int size = minSize; size < maxSize; size += sizeIncrement) {
            QPainter p(this);
            p.drawPixmap(x, y, iconNormalDpi->pixmap(size, size));
            if (x + dx > maxX)
                y+=dy;
            x = ((x + dx) % maxX);
        }
        x = 10;
        y+=dy;

        // high-dpi icon (draw point)
        for (int size = minSize; size < maxSize; size += sizeIncrement) {
            QPainter p(this);
            p.drawPixmap(x, y, iconHighDPI->pixmap(size, size));
            if (x + dx > maxX)
                y+=dy;
            x = ((x + dx) % maxX);
        }

        x = 10;
        y+=dy;

    }

private:
    QTemporaryDir m_temporaryDir;
    QScopedPointer<QIcon> iconHighDPI;
    QScopedPointer<QIcon> iconNormalDpi;
};

// Icons on buttons
class Buttons : public QWidget
{
public:
    Buttons()
    {
        QIcon icon;
        icon.addFile(":/qticon16@2x.png");

        QPushButton *button =  new QPushButton(this);
        button->setIcon(icon);
        button->setText("16@2x");

        QTabBar *tab = new QTabBar(this);
        tab->addTab(QIcon(":/qticon16.png"), "16@1x");
        tab->addTab(QIcon(":/qticon16@2x.png"), "16@2x");
        tab->addTab(QIcon(":/qticon16.png"), "");
        tab->addTab(QIcon(":/qticon16@2x.png"), "");
        tab->move(10, 100);
        tab->show();

        QToolBar *toolBar = new QToolBar(this);
        toolBar->addAction(QIcon(":/qticon16.png"), "16");
        toolBar->addAction(QIcon(":/qticon16@2x.png"), "16@2x");
        toolBar->addAction(QIcon(":/qticon32.png"), "32");
        toolBar->addAction(QIcon(":/qticon32@2x.png"), "32@2x");

        toolBar->move(10, 200);
        toolBar->show();
    }
};


int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QCoreApplication::setApplicationVersion(QT_VERSION_STR);

    QCommandLineParser parser;
    parser.setApplicationDescription("High DPI tester");
    parser.addHelpOption();
    parser.addVersionOption();
    QCommandLineOption pixmapPainterOption("pixmap", "Test pixmap painter");
    parser.addOption(pixmapPainterOption);
    QCommandLineOption labelOption("label", "Test Labels");
    parser.addOption(labelOption);
    QCommandLineOption mainWindowOption("mainwindow", "Test QMainWindow");
    parser.addOption(mainWindowOption);
    QCommandLineOption standardIconsOption("standard-icons", "Test standard icons");
    parser.addOption(standardIconsOption);
    QCommandLineOption cachingOption("caching", "Test caching");
    parser.addOption(cachingOption);
    QCommandLineOption styleOption("styles", "Test style");
    parser.addOption(styleOption);
    QCommandLineOption fontsOption("fonts", "Test fonts");
    parser.addOption(fontsOption);
    QCommandLineOption iconDrawingOption("icondrawing", "Test icon drawing");
    parser.addOption(iconDrawingOption);
    QCommandLineOption buttonsOption("buttons", "Test buttons");
    parser.addOption(buttonsOption);

    parser.process(app);

    QScopedPointer<PixmapPainter> pixmapPainter;
    if (parser.isSet(pixmapPainterOption)) {
        pixmapPainter.reset(new PixmapPainter);
        pixmapPainter->show();
    }

    QScopedPointer<Labels> label;
    if (parser.isSet(labelOption)) {
        label.reset(new Labels);
        label->resize(200, 200);
        label->show();
    }

    QScopedPointer<MainWindow> mainWindow;
    if (parser.isSet(mainWindowOption)) {
        mainWindow.reset(new MainWindow);
        mainWindow->show();
    }

    QScopedPointer<StandardIcons> icons;
    if (parser.isSet(standardIconsOption)) {
        icons.reset(new StandardIcons);
        icons->resize(510, 510);
        icons->show();
    }

    QScopedPointer<Caching> caching;
    if (parser.isSet(cachingOption)) {
        caching.reset(new Caching);
        caching->resize(300, 300);
        caching->show();
    }

    QScopedPointer<Style> style;
    if (parser.isSet(styleOption)) {
        style.reset(new Style);
        style->show();
    }

    QScopedPointer<Fonts> fonts;
    if (parser.isSet(fontsOption)) {
        fonts.reset(new Fonts);
        fonts->show();
    }

    QScopedPointer<IconDrawing> iconDrawing;
    if (parser.isSet(iconDrawingOption)) {
        iconDrawing.reset(new IconDrawing);
        iconDrawing->show();
    }

    QScopedPointer<Buttons> buttons;
    if (parser.isSet(buttonsOption)) {
        buttons.reset(new Buttons);
        buttons->show();
    }

    if (QApplication::topLevelWidgets().isEmpty())
        parser.showHelp(0);

    return app.exec();
}
