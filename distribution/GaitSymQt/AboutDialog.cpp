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
#ifdef dNODEBUG
    QString descriptor = QString("Release");
#else
    QString descriptor = QString("Debug");
#endif
#ifdef EXPERIMENTAL
    descriptor = QString("%1 %2").arg(descriptor).arg("Experimental");
#endif

    descriptor = QString("%1 %2").arg(descriptor).arg(buildDate);
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

