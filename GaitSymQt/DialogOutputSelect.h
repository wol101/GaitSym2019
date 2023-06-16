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
class Simulation;

namespace Ui {
class DialogOutputSelect;
}

class DialogOutputSelect : public QDialog
{
    Q_OBJECT
public:
    DialogOutputSelect(QWidget *parent = nullptr);
    virtual ~DialogOutputSelect() Q_DECL_OVERRIDE;

    Ui::DialogOutputSelect *ui() { return m_ui; }


    QListWidget *listWidgetBody() const;
    void setListWidgetBody(QListWidget *listWidgetBody);

    QListWidget *listWidgetDataTarget() const;
    void setListWidgetDataTarget(QListWidget *listWidgetDataTarget);

    QListWidget *listWidgetDriver() const;
    void setListWidgetDriver(QListWidget *listWidgetDriver);

    QListWidget *listWidgetGeom() const;
    void setListWidgetGeom(QListWidget *listWidgetGeom);

    QListWidget *listWidgetJoint() const;
    void setListWidgetJoint(QListWidget *listWidgetJoint);

    QListWidget *listWidgetMarker() const;
    void setListWidgetMarker(QListWidget *listWidgetMarker);

    QListWidget *listWidgetMuscle() const;
    void setListWidgetMuscle(QListWidget *listWidgetMuscle);

    QListWidget *listWidgetReporter() const;
    void setListWidgetReporter(QListWidget *listWidgetReporter);

    QListWidget *listWidgetController() const;
    void setListWidgetController(QListWidget *listWidgetController);

    QListWidget *listWidgetWarehouse() const;
    void setListWidgetWarehouse(QListWidget *listWidgetWarehouse);

    Simulation *simulation() const;
    void setSimulation(Simulation *simulation);


public slots:
    void menuRequestMuscle(QPoint);
    void menuRequestBody(QPoint);
    void menuRequestJoint(QPoint);
    void menuRequestMarker(QPoint);
    void menuRequestGeom(QPoint);
    void menuRequestDriver(QPoint);
    void menuRequestDataTarget(QPoint);
    void menuRequestReporter(QPoint);
    void menuRequestController(QPoint);
    void menuRequestWarehouse(QPoint);
    void acceptButtonClicked();
    void rejectButtonClicked();

protected:
    void changeEvent(QEvent *e) Q_DECL_OVERRIDE;
    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;

private:
    QListWidget *m_listWidgetBody = nullptr;
    QListWidget *m_listWidgetDataTarget = nullptr;
    QListWidget *m_listWidgetDriver = nullptr;
    QListWidget *m_listWidgetGeom = nullptr;
    QListWidget *m_listWidgetJoint = nullptr;
    QListWidget *m_listWidgetMarker = nullptr;
    QListWidget *m_listWidgetMuscle = nullptr;
    QListWidget *m_listWidgetReporter = nullptr;
    QListWidget *m_listWidgetController = nullptr;
    QListWidget *m_listWidgetWarehouse = nullptr;

    QGridLayout *gridLayout = nullptr;
    QDialogButtonBox *buttonBox = nullptr;
    int m_columns = 0;
    Ui::DialogOutputSelect *m_ui = nullptr;

    Simulation *m_simulation = nullptr;
};

#endif // DIALOGOUTPUTSELECT_H
