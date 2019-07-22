/*
 *  main.cpp
 *  GaitSymODE2019
 *
 *  Created by Bill Sellers on 08/10/2018.
 *  Copyright 2018 Bill Sellers. All rights reserved.
 *
 */

#include "MainWindow.h"

#include <QApplication>
#include <QSurfaceFormat>
#include <QStyleFactory>
#include <QDebug>

#if defined(QT_DEBUG) && defined(Q_OS_WIN)
#include <crtdbg.h>
#endif

int main(int argc, char *argv[])
{
#if defined(QT_DEBUG) && defined(Q_OS_WIN)
//    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF | _CRTDBG_CHECK_ALWAYS_DF);
//    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF | _CRTDBG_CHECK_EVERY_1024_DF);
#endif


    QSurfaceFormat fmt = QSurfaceFormat::defaultFormat();
    fmt.setRenderableType(QSurfaceFormat::OpenGL);
    fmt.setDepthBufferSize(24);
    fmt.setAlphaBufferSize(8);
    fmt.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
    fmt.setSwapInterval(1);
    fmt.setSamples(8);
//    fmt.setVersion(3, 3); // OpenGL 3.3
//    fmt.setProfile(QSurfaceFormat::CoreProfile); // only use the core functions
//    fmt.setOption(QSurfaceFormat::DebugContext, true);
    QSurfaceFormat::setDefaultFormat(fmt);

    QApplication a(argc, argv);

#ifdef USE_COFFEE_STYLE
    QFile file(":/stylesheets/coffee.qss");
    file.open(QFile::ReadOnly);
    QString styleSheet = QLatin1String(file.readAll());
    a.setStyleSheet(styleSheet);
#endif
#ifdef USE_FUSION_DARK
    a.setStyle(QStyleFactory::create("Fusion"));
    // increase font size for better reading
    QFont defaultFont = QApplication::font();
    defaultFont.setPointSize(defaultFont.pointSize()+2);
    a.setFont(defaultFont);
    // modify palette to dark
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window,QColor(53,53,53));
    darkPalette.setColor(QPalette::WindowText,Qt::white);
    darkPalette.setColor(QPalette::Disabled,QPalette::WindowText,QColor(127,127,127));
    darkPalette.setColor(QPalette::Base,QColor(42,42,42));
    darkPalette.setColor(QPalette::AlternateBase,QColor(66,66,66));
    darkPalette.setColor(QPalette::ToolTipBase,Qt::white);
    darkPalette.setColor(QPalette::ToolTipText,Qt::white);
    darkPalette.setColor(QPalette::Text,Qt::white);
    darkPalette.setColor(QPalette::Disabled,QPalette::Text,QColor(127,127,127));
    darkPalette.setColor(QPalette::Dark,QColor(35,35,35));
    darkPalette.setColor(QPalette::Shadow,QColor(20,20,20));
    darkPalette.setColor(QPalette::Button,QColor(53,53,53));
    darkPalette.setColor(QPalette::ButtonText,Qt::white);
    darkPalette.setColor(QPalette::Disabled,QPalette::ButtonText,QColor(127,127,127));
    darkPalette.setColor(QPalette::BrightText,Qt::red);
    darkPalette.setColor(QPalette::Link,QColor(42,130,218));
    darkPalette.setColor(QPalette::Highlight,QColor(42,130,218));
    darkPalette.setColor(QPalette::Disabled,QPalette::Highlight,QColor(80,80,80));
    darkPalette.setColor(QPalette::HighlightedText,Qt::white);
    darkPalette.setColor(QPalette::Disabled,QPalette::HighlightedText,QColor(127,127,127));
    a.setPalette(darkPalette);
#endif
#ifdef USE_WINDOWS_STYLE
    // styles available
    // Windows: "Windows", "windowsvista", "Fusion"
    // Others: "gtk" and "macintosh"
    qDebug() << QStyleFactory::keys();
    a.setStyle(QStyleFactory::create("Windows"));
#endif

    MainWindow w;
    w.show();
    return a.exec();
}
