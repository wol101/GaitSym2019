/*
 *  AboutDialog.cpp
 *  GaitSymODE2019
 *
 *  Created by Bill Sellers on 08/10/2018.
 *  Copyright 2018 Bill Sellers. All rights reserved.
 *
 */

#include "AboutDialog.h"
#include "ui_AboutDialog.h"

#include "Preferences.h"

#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QSysInfo>
#include <QLibraryInfo>
#include <QVersionNumber>

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);
    setWindowTitle(tr("About GaitSym2019"));
#ifdef Q_OS_MACOS
    setWindowFlags(windowFlags() & (~Qt::Dialog) | Qt::Window); // allows the window to be resized on macs
#endif
    restoreGeometry(Preferences::valueQByteArray("AboutDialogGeometry"));

    QString buildDate = QString("Build: %1 %2").arg(__DATE__, __TIME__);
    QString buildType;
#ifdef dNODEBUG
    buildType = "Release";
#else
    buildType = "Debug";
#endif
#ifdef EXPERIMENTAL
    buildType += " Experimental";
#endif

    QString buildInformation = QSysInfo::buildAbi();

    QVersionNumber libraryVersionNumber = QLibraryInfo::version();
    bool libraryDebugBuild = QLibraryInfo::isDebugBuild();
    QString libraryString = QString("Qt%1").arg(libraryVersionNumber.toString());
    if (libraryDebugBuild) libraryString += " Debug";
    else libraryString += " Release";
#if (QT_VERSION >= QT_VERSION_CHECK(6, 5, 0))
    bool librarySharedBuild = QLibraryInfo::isSharedBuild();
    if (librarySharedBuild) libraryString += " Shared";
    else libraryString += " Static";
#endif

    QOffscreenSurface surf;
    surf.create();
    QOpenGLContext ctx;
    ctx.create();
    ctx.makeCurrent(&surf);
    const char *p = reinterpret_cast<const char *>(ctx.functions()->glGetString(GL_VERSION));
    std::string glVersionString = (p ? p : "");

    QString descriptor = QString("%1\n%2\n%3\n%4\n%5").arg(buildDate, buildType, glVersionString.c_str(), buildInformation, libraryString);
    ui->labelDescriptor->setText(descriptor);
}

AboutDialog::~AboutDialog()
{
    delete ui;
}

// note: need to trap closeEvent, accept and reject to be sure of getting all the things that might dismiss the dialog

void AboutDialog::closeEvent(QCloseEvent *event)
{
    Preferences::insert("AboutDialogGeometry", saveGeometry());
    QDialog::closeEvent(event);
}

void AboutDialog::accept() // this catches cancel, close and escape key
{
    Preferences::insert("AboutDialogGeometry", saveGeometry());
    QDialog::accept();
}

void AboutDialog::reject() // this catches cancel, close and escape key
{
    Preferences::insert("AboutDialogGeometry", saveGeometry());
    QDialog::reject();
}

