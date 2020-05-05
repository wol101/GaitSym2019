/*
 *  main.cpp
 *  GaitSymODE2019
 *
 *  Created by Bill Sellers on 08/10/2018.
 *  Copyright 2018 Bill Sellers. All rights reserved.
 *
 */

#include "MainWindow.h"
#include "Preferences.h"

#include <QApplication>
#include <QSurfaceFormat>
#include <QStyleFactory>
#include <QDebug>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QMessageBox>

#if defined(QT_DEBUG) && defined(Q_OS_WIN)
#include <crtdbg.h>
#endif

int main(int argc, char *argv[])
{
#if defined(QT_DEBUG) && defined(Q_OS_WIN)
//    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF | _CRTDBG_CHECK_ALWAYS_DF);
//    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF | _CRTDBG_CHECK_EVERY_1024_DF);
#endif

    // read in the Preferences file
    Preferences::Read();

    QCoreApplication::setAttribute(Qt::AA_UseDesktopOpenGL); // AA_UseOpenGLES not currently supported

    QSurfaceFormat fmt = QSurfaceFormat::defaultFormat();
    fmt.setRenderableType(QSurfaceFormat::OpenGL);
    fmt.setDepthBufferSize(24);
    fmt.setRedBufferSize(8);
    fmt.setGreenBufferSize(8);
    fmt.setBlueBufferSize(8);
    fmt.setAlphaBufferSize(8);
    fmt.setStencilBufferSize(8);
    fmt.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
    fmt.setSwapInterval(1);
    fmt.setSamples(8);
    fmt.setColorSpace(QSurfaceFormat::sRGBColorSpace);
    fmt.setVersion(3, 3); // OpenGL 3.3
    fmt.setProfile(QSurfaceFormat::CoreProfile); // only use the core functions
    fmt.setOption(QSurfaceFormat::DebugContext, true);
    QSurfaceFormat::setDefaultFormat(fmt);

    QApplication a(argc, argv);

    try
    {
        QOffscreenSurface surf;
        surf.create();
        QOpenGLContext ctx;
        ctx.create();
        ctx.makeCurrent(&surf);
        const char *p = reinterpret_cast<const char *>(ctx.functions()->glGetString(GL_VERSION));
        std::string glVersionString(p ? p : "");
        p  = reinterpret_cast<const char *>(ctx.functions()->glGetString(GL_EXTENSIONS));
        std::string glExtensionsString(p ? p : "");
        double glVersion = std::stod(glVersionString);
        qDebug () << glVersionString.c_str();
        qDebug () << glExtensionsString.c_str();
        if (glVersion <= 3.2)
        {
            QString errorMessage = QString("This application requires OpenGL 3.3 or greater.\nCurrent version is %1.\nApplication will abort.").arg(glVersionString.c_str());
            QMessageBox::critical(nullptr, "GaitSym2019", errorMessage);
            exit(EXIT_FAILURE);
        }
    }
    catch (...)
    {
        QString errorMessage = QString("This application requires OpenGL 3.3 or greater.\nUnknown failure initialising OpenGL.\nApplication will abort.");
        QMessageBox::critical(nullptr, "GaitSym2019", errorMessage);
        exit(EXIT_FAILURE);
    }

    int styleCode = Preferences::valueInt("StyleCode");
    QStringList styles = QStyleFactory::keys();
    qDebug() << styles;
    if (styleCode == 0 && styles.contains("Fusion", Qt::CaseInsensitive))
    {
        a.setStyle(QStyleFactory::create("Fusion"));
    }
    else if (styleCode == 1 && styles.contains("Windows", Qt::CaseInsensitive))
    {
        a.setStyle(QStyleFactory::create("Windows"));
    }
    else if (styleCode == 2 && styles.contains("windowsvista", Qt::CaseInsensitive))
    {
        a.setStyle(QStyleFactory::create("windowsvista"));
    }
    else if (styleCode == 2 && styles.contains("gtk", Qt::CaseInsensitive))
    {
        a.setStyle(QStyleFactory::create("gtk"));
    }
    else if (styleCode == 2 && styles.contains("macintosh", Qt::CaseInsensitive))
    {
        a.setStyle(QStyleFactory::create("macintosh"));
    }
    else if (styleCode == 3)
    {
        QFile file(":/stylesheets/coffee.qss");
        file.open(QFile::ReadOnly);
        QString styleSheet = QLatin1String(file.readAll());
        a.setStyleSheet(styleSheet);
    }
    else if (styleCode == 4)
    {
        QFile file(":/stylesheets/pagefold.qss");
        file.open(QFile::ReadOnly);
        QString styleSheet = QLatin1String(file.readAll());
        a.setStyleSheet(styleSheet);
    }
    if (styleCode == 5 && styles.contains("Fusion", Qt::CaseInsensitive))
    {
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
    }

    MainWindow w;
    w.show();
    return a.exec();
}
