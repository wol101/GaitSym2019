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

    QString buildDate = QString("Build: %1 %2").arg(__DATE__).arg(__TIME__);
    QString buildType;
#ifdef dNODEBUG
    buildType = "Release";
#else
    buildType = "Debug";
#endif
#ifdef EXPERIMENTAL
    buildType += " experimental";
#endif

    QOffscreenSurface surf;
    surf.create();
    QOpenGLContext ctx;
    ctx.create();
    ctx.makeCurrent(&surf);
    const char *p = reinterpret_cast<const char *>(ctx.functions()->glGetString(GL_VERSION));
    std::string glVersionString = (p ? p : "");

    QString descriptor = QString("%1\n%2\n%3").arg(buildDate).arg(buildType).arg(glVersionString.c_str());
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

