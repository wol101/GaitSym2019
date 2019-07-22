/*
 *  DialogOutputSelect.h
 *  GaitSymODE2019
 *
 *  Created by Bill Sellers on 08/10/2018.
 *  Copyright 2018 Bill Sellers. All rights reserved.
 *
 */

#ifndef DIALOGOUTPUTSELECT_H
#define DIALOGOUTPUTSELECT_H

#include "Preferences.h"

#include <QDialog>
#include <QFont>

class QListWidget;
class QLabel;
class QDialogButtonBox;
class QGridLayout;

namespace Ui {
class DialogOutputSelect;
}

class DialogOutputSelect : public QDialog
{
    Q_OBJECT
public:
    DialogOutputSelect(QWidget *parent = nullptr);
    virtual ~DialogOutputSelect() Q_DECL_OVERRIDE;

    Ui::DialogOutputSelect *ui()
    {
        return m_ui;
    }

    QListWidget *listWidgetBody;
    QListWidget *listWidgetDataTarget;
    QListWidget *listWidgetDriver;
    QListWidget *listWidgetGeom;
    QListWidget *listWidgetJoint;
    QListWidget *listWidgetMuscle;
    QListWidget *listWidgetReporter;
    QListWidget *listWidgetWarehouse;

public slots:
    void menuRequestMuscle(QPoint);
    void menuRequestBody(QPoint);
    void menuRequestJoint(QPoint);
    void menuRequestGeom(QPoint);
    void menuRequestDriver(QPoint);
    void menuRequestDataTarget(QPoint);
    void menuRequestReporter(QPoint);
    void menuRequestWarehouse(QPoint);
    void acceptButtonClicked();
    void rejectButtonClicked();

protected:
    void changeEvent(QEvent *e);

private:

    QGridLayout *gridLayout;
    QDialogButtonBox *buttonBox;
    int m_columns;
    Ui::DialogOutputSelect *m_ui;
};

#endif // DIALOGOUTPUTSELECT_H
