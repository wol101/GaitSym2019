/*
 *  DialogOutputSelect.cpp
 *  GaitSymODE2019
 *
 *  Created by Bill Sellers on 08/10/2018.
 *  Copyright 2018 Bill Sellers. All rights reserved.
 *
 */

#include <QMenu>
#include <QListWidget>
#include <QLabel>
#include <QGridLayout>
#include <QDialogButtonBox>

#include "DialogOutputSelect.h"
#include "ui_DialogOutputSelect.h"

#include "Simulation.h"
#include "Muscle.h"
#include "Body.h"
#include "Joint.h"
#include "Geom.h"
#include "Driver.h"
#include "DataTarget.h"
#include "Reporter.h"
#include "Warehouse.h"
#include "MainWindow.h"
#include "Preferences.h"

DialogOutputSelect::DialogOutputSelect(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::DialogOutputSelect)
{
    m_ui->setupUi(this);
#ifdef Q_OS_MACOS
    // on MacOS High Sierra (and possibly others), the Qt::Dialog flag disables resizing
    // if I clear the Qt::Dialog flag and make sure the Qt::Window flag is set (it should be already)
    // then it seems to let me have resizing without any other side effects
    setWindowFlags(windowFlags() & (~Qt::Dialog) | Qt::Window);
#endif

    listWidgetBody = 0;
    listWidgetDataTarget = 0;
    listWidgetDriver = 0;
    listWidgetGeom = 0;
    listWidgetJoint = 0;
    listWidgetMuscle = 0;
    listWidgetReporter = 0;
    listWidgetWarehouse = 0;

    gridLayout = new QGridLayout(this);
    QFont listWidgetFont;
    listWidgetFont.setPointSize(10);

    QListWidgetItem *item;
    QLabel *label;
    int count;
    MainWindow *mainWindow = dynamic_cast<MainWindow *>(parent);
    Simulation *simulation = mainWindow->simulation();
    std::map<std::string, Body *> *bodyList = simulation->GetBodyList();
    std::map<std::string, Joint *> *jointList = simulation->GetJointList();
    std::map<std::string, Geom *> *geomList = simulation->GetGeomList();
    std::map<std::string, Muscle *> *muscleList = simulation->GetMuscleList();
    std::map<std::string, Driver *> *driverList = simulation->GetDriverList();
    std::map<std::string, DataTarget *> *dataTargetList = simulation->GetDataTargetList();
    std::map<std::string, Reporter *> *reporterList = simulation->GetReporterList();
    std::map<std::string, Warehouse *> *warehouseList = simulation->GetWarehouseList();

    m_columns = 0;
    if (bodyList->size() > 0)
    {
        listWidgetBody = new QListWidget(this);
        listWidgetBody->setFont(listWidgetFont);
        listWidgetBody->setContextMenuPolicy(Qt::CustomContextMenu);
        count = 0;
        listWidgetBody->clear();
        std::map<std::string, Body *>::const_iterator bodyIterator;
        for (bodyIterator = bodyList->begin(); bodyIterator != bodyList->end(); bodyIterator++)
        {
            listWidgetBody->addItem(bodyIterator->first.c_str());
            item = listWidgetBody->item(count++);
            if (bodyIterator->second->GetDump()) item->setCheckState(Qt::Checked);
            else item->setCheckState(Qt::Unchecked);
        }
        gridLayout->addWidget(listWidgetBody, 1, m_columns);
        label = new QLabel("Body List", this);
        gridLayout->addWidget(label, 0, m_columns);
        m_columns++;
        QObject::connect(listWidgetBody, SIGNAL(customContextMenuRequested(QPoint)), this,
                         SLOT(menuRequestBody(QPoint)));
    }

    if (jointList->size() > 0)
    {
        listWidgetJoint = new QListWidget(this);
        listWidgetJoint->setFont(listWidgetFont);
        listWidgetJoint->setContextMenuPolicy(Qt::CustomContextMenu);
        count = 0;
        listWidgetJoint->clear();
        std::map<std::string, Joint *>::const_iterator jointIterator;
        for (jointIterator = jointList->begin(); jointIterator != jointList->end(); jointIterator++)
        {
            listWidgetJoint->addItem(jointIterator->first.c_str());
            item = listWidgetJoint->item(count++);
            if (jointIterator->second->GetDump()) item->setCheckState(Qt::Checked);
            else item->setCheckState(Qt::Unchecked);
        }
        gridLayout->addWidget(listWidgetJoint, 1, m_columns);
        label = new QLabel("Joint List", this);
        gridLayout->addWidget(label, 0, m_columns);
        m_columns++;
        QObject::connect(listWidgetJoint, SIGNAL(customContextMenuRequested(QPoint)), this,
                         SLOT(menuRequestJoint(QPoint)));
    }

    if (geomList->size() > 0)
    {
        listWidgetGeom = new QListWidget(this);
        listWidgetGeom->setFont(listWidgetFont);
        listWidgetGeom->setContextMenuPolicy(Qt::CustomContextMenu);
        count = 0;
        listWidgetGeom->clear();
        std::map<std::string, Geom *>::const_iterator geomIterator;
        for (geomIterator = geomList->begin(); geomIterator != geomList->end(); geomIterator++)
        {
            listWidgetGeom->addItem(geomIterator->first.c_str());
            item = listWidgetGeom->item(count++);
            if (geomIterator->second->GetDump()) item->setCheckState(Qt::Checked);
            else item->setCheckState(Qt::Unchecked);
        }
        gridLayout->addWidget(listWidgetGeom, 1, m_columns);
        label = new QLabel("Geom List", this);
        gridLayout->addWidget(label, 0, m_columns);
        m_columns++;
        QObject::connect(listWidgetGeom, SIGNAL(customContextMenuRequested(QPoint)), this,
                         SLOT(menuRequestGeom(QPoint)));
    }

    if (muscleList->size() > 0)
    {
        listWidgetMuscle = new QListWidget(this);
        listWidgetMuscle->setFont(listWidgetFont);
        listWidgetMuscle->setContextMenuPolicy(Qt::CustomContextMenu);
        count = 0;
        listWidgetMuscle->clear();
        std::map<std::string, Muscle *>::const_iterator muscleIterator;
        for (muscleIterator = muscleList->begin(); muscleIterator != muscleList->end(); muscleIterator++)
        {
            listWidgetMuscle->addItem(muscleIterator->first.c_str());
            item = listWidgetMuscle->item(count++);
            if (muscleIterator->second->GetDump()) item->setCheckState(Qt::Checked);
            else item->setCheckState(Qt::Unchecked);
        }
        gridLayout->addWidget(listWidgetMuscle, 1, m_columns);
        label = new QLabel("Muscle List", this);
        gridLayout->addWidget(label, 0, m_columns);
        m_columns++;
        QObject::connect(listWidgetMuscle, SIGNAL(customContextMenuRequested(QPoint)), this,
                         SLOT(menuRequestMuscle(QPoint)));
    }

    if (driverList->size() > 0)
    {
        listWidgetDriver = new QListWidget(this);
        listWidgetDriver->setFont(listWidgetFont);
        listWidgetDriver->setContextMenuPolicy(Qt::CustomContextMenu);
        count = 0;
        listWidgetDriver->clear();
        std::map<std::string, Driver *>::const_iterator driverIterator;
        for (driverIterator = driverList->begin(); driverIterator != driverList->end(); driverIterator++)
        {
            listWidgetDriver->addItem(driverIterator->first.c_str());
            item = listWidgetDriver->item(count++);
            if (driverIterator->second->GetDump()) item->setCheckState(Qt::Checked);
            else item->setCheckState(Qt::Unchecked);
        }
        gridLayout->addWidget(listWidgetDriver, 1, m_columns);
        label = new QLabel("Driver List", this);
        gridLayout->addWidget(label, 0, m_columns);
        m_columns++;
        QObject::connect(listWidgetDriver, SIGNAL(customContextMenuRequested(QPoint)), this,
                         SLOT(menuRequestDriver(QPoint)));
    }

    if (dataTargetList->size() > 0)
    {
        listWidgetDataTarget = new QListWidget(this);
        listWidgetDataTarget->setFont(listWidgetFont);
        listWidgetDataTarget->setContextMenuPolicy(Qt::CustomContextMenu);
        count = 0;
        listWidgetDataTarget->clear();
        std::map<std::string, DataTarget *>::const_iterator dataTargetIterator;
        for (dataTargetIterator = dataTargetList->begin(); dataTargetIterator != dataTargetList->end();
                dataTargetIterator++)
        {
            listWidgetDataTarget->addItem(dataTargetIterator->first.c_str());
            item = listWidgetDataTarget->item(count++);
            if (dataTargetIterator->second->GetDump()) item->setCheckState(Qt::Checked);
            else item->setCheckState(Qt::Unchecked);
        }
        gridLayout->addWidget(listWidgetDataTarget, 1, m_columns);
        label = new QLabel("DataTarget List", this);
        gridLayout->addWidget(label, 0, m_columns);
        m_columns++;
        QObject::connect(listWidgetDataTarget, SIGNAL(customContextMenuRequested(QPoint)), this,
                         SLOT(menuRequestDataTarget(QPoint)));
    }

    if (reporterList->size() > 0)
    {
        listWidgetReporter = new QListWidget(this);
        listWidgetReporter->setFont(listWidgetFont);
        listWidgetReporter->setContextMenuPolicy(Qt::CustomContextMenu);
        count = 0;
        listWidgetReporter->clear();
        std::map<std::string, Reporter *>::const_iterator reporterIterator;
        for (reporterIterator = reporterList->begin(); reporterIterator != reporterList->end();
                reporterIterator++)
        {
            listWidgetReporter->addItem(reporterIterator->first.c_str());
            item = listWidgetReporter->item(count++);
            if (reporterIterator->second->GetDump()) item->setCheckState(Qt::Checked);
            else item->setCheckState(Qt::Unchecked);
        }
        gridLayout->addWidget(listWidgetReporter, 1, m_columns);
        label = new QLabel("Reporter List", this);
        gridLayout->addWidget(label, 0, m_columns);
        m_columns++;
        QObject::connect(listWidgetReporter, SIGNAL(customContextMenuRequested(QPoint)), this,
                         SLOT(menuRequestReporter(QPoint)));
    }

    if (warehouseList->size() > 0)
    {
        listWidgetWarehouse = new QListWidget(this);
        listWidgetWarehouse->setFont(listWidgetFont);
        listWidgetWarehouse->setContextMenuPolicy(Qt::CustomContextMenu);
        count = 0;
        listWidgetWarehouse->clear();
        std::map<std::string, Warehouse *>::const_iterator warehouseIterator;
        for (warehouseIterator = warehouseList->begin(); warehouseIterator != warehouseList->end();
                warehouseIterator++)
        {
            listWidgetWarehouse->addItem(warehouseIterator->first.c_str());
            item = listWidgetWarehouse->item(count++);
            if (warehouseIterator->second->GetDump()) item->setCheckState(Qt::Checked);
            else item->setCheckState(Qt::Unchecked);
        }
        gridLayout->addWidget(listWidgetWarehouse, 1, m_columns);
        label = new QLabel("Warehouse List", this);
        gridLayout->addWidget(label, 0, m_columns);
        m_columns++;
        QObject::connect(listWidgetWarehouse, SIGNAL(customContextMenuRequested(QPoint)), this,
                         SLOT(menuRequestWarehouse(QPoint)));
    }

    buttonBox = new QDialogButtonBox(this);
    QFont buttonBoxFont;
    buttonBoxFont.setBold(false);
    buttonBoxFont.setWeight(50);
    buttonBox->setFont(buttonBoxFont);
    buttonBox->setOrientation(Qt::Horizontal);
    buttonBox->setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);
    gridLayout->addWidget(buttonBox, 2, 0, 1, m_columns, Qt::AlignRight);
    QObject::connect(buttonBox, SIGNAL(accepted()), this, SLOT(acceptButtonClicked()));
    QObject::connect(buttonBox, SIGNAL(rejected()), this, SLOT(rejectButtonClicked()));

    restoreGeometry(Preferences::valueQByteArray("DialogOutputSelectGeometry"));
}

DialogOutputSelect::~DialogOutputSelect()
{
    delete m_ui;
}

void DialogOutputSelect::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type())
    {
    case QEvent::LanguageChange:
        m_ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void DialogOutputSelect::menuRequestMuscle(QPoint p)
{
    QMenu menu(this);
    menu.addAction(tr("All On"));
    menu.addAction(tr("All Off"));

    QPoint gp = listWidgetMuscle->mapToGlobal(p);

    QAction *action = menu.exec(gp);
    QListWidgetItem *item;
    Qt::CheckState state;
    int i;
    bool dump;
    if (action)
    {
        if (action->text() == tr("All On"))
        {
            state = Qt::Checked;
            dump = true;
        }
        else
        {
            state = Qt::Unchecked;
            dump = false;
        }
        for (i = 0; i < listWidgetMuscle->count(); i++)
        {
            item = listWidgetMuscle->item(i);
            item->setCheckState(state);
        }
    }
}

void DialogOutputSelect::menuRequestBody(QPoint p)
{
    QMenu menu(this);
    menu.addAction(tr("All On"));
    menu.addAction(tr("All Off"));

    QPoint gp = listWidgetBody->mapToGlobal(p);

    QAction *action = menu.exec(gp);
    QListWidgetItem *item;
    Qt::CheckState state;
    int i;
    bool dump;
    if (action)
    {
        if (action->text() == tr("All On"))
        {
            state = Qt::Checked;
            dump = true;
        }
        else
        {
            state = Qt::Unchecked;
            dump = false;
        }
        for (i = 0; i < listWidgetBody->count(); i++)
        {
            item = listWidgetBody->item(i);
            item->setCheckState(state);
        }
    }
}

void DialogOutputSelect::menuRequestJoint(QPoint p)
{
    QMenu menu(this);
    menu.addAction(tr("All On"));
    menu.addAction(tr("All Off"));

    QPoint gp = listWidgetJoint->mapToGlobal(p);

    QAction *action = menu.exec(gp);
    QListWidgetItem *item;
    Qt::CheckState state;
    int i;
    bool dump;
    if (action)
    {
        if (action->text() == tr("All On"))
        {
            state = Qt::Checked;
            dump = true;
        }
        else
        {
            state = Qt::Unchecked;
            dump = false;
        }
        for (i = 0; i < listWidgetJoint->count(); i++)
        {
            item = listWidgetJoint->item(i);
            item->setCheckState(state);
        }
    }
}

void DialogOutputSelect::menuRequestGeom(QPoint p)
{
    QMenu menu(this);
    menu.addAction(tr("All On"));
    menu.addAction(tr("All Off"));

    QPoint gp = listWidgetGeom->mapToGlobal(p);

    QAction *action = menu.exec(gp);
    QListWidgetItem *item;
    Qt::CheckState state;
    int i;
    bool dump;
    if (action)
    {
        if (action->text() == tr("All On"))
        {
            state = Qt::Checked;
            dump = true;
        }
        else
        {
            state = Qt::Unchecked;
            dump = false;
        }
        for (i = 0; i < listWidgetGeom->count(); i++)
        {
            item = listWidgetGeom->item(i);
            item->setCheckState(state);
        }
    }
}

void DialogOutputSelect::menuRequestDriver(QPoint p)
{
    QMenu menu(this);
    menu.addAction(tr("All On"));
    menu.addAction(tr("All Off"));

    QPoint gp = listWidgetDriver->mapToGlobal(p);

    QAction *action = menu.exec(gp);
    QListWidgetItem *item;
    Qt::CheckState state;
    int i;
    bool dump;
    if (action)
    {
        if (action->text() == tr("All On"))
        {
            state = Qt::Checked;
            dump = true;
        }
        else
        {
            state = Qt::Unchecked;
            dump = false;
        }
        for (i = 0; i < listWidgetDriver->count(); i++)
        {
            item = listWidgetDriver->item(i);
            item->setCheckState(state);
        }
    }
}

void DialogOutputSelect::menuRequestDataTarget(QPoint p)
{
    QMenu menu(this);
    menu.addAction(tr("All On"));
    menu.addAction(tr("All Off"));

    QPoint gp = listWidgetDataTarget->mapToGlobal(p);

    QAction *action = menu.exec(gp);
    QListWidgetItem *item;
    Qt::CheckState state;
    int i;
    bool dump;
    if (action)
    {
        if (action->text() == tr("All On"))
        {
            state = Qt::Checked;
            dump = true;
        }
        else
        {
            state = Qt::Unchecked;
            dump = false;
        }
        for (i = 0; i < listWidgetDataTarget->count(); i++)
        {
            item = listWidgetDataTarget->item(i);
            item->setCheckState(state);
        }
    }
}

void DialogOutputSelect::menuRequestReporter(QPoint p)
{
    QMenu menu(this);
    menu.addAction(tr("All On"));
    menu.addAction(tr("All Off"));

    QPoint gp = listWidgetReporter->mapToGlobal(p);

    QAction *action = menu.exec(gp);
    QListWidgetItem *item;
    Qt::CheckState state;
    int i;
    bool dump;
    if (action)
    {
        if (action->text() == tr("All On"))
        {
            state = Qt::Checked;
            dump = true;
        }
        else
        {
            state = Qt::Unchecked;
            dump = false;
        }
        for (i = 0; i < listWidgetReporter->count(); i++)
        {
            item = listWidgetReporter->item(i);
            item->setCheckState(state);
        }
    }
}

void DialogOutputSelect::menuRequestWarehouse(QPoint p)
{
    QMenu menu(this);
    menu.addAction(tr("All On"));
    menu.addAction(tr("All Off"));

    QPoint gp = listWidgetWarehouse->mapToGlobal(p);

    QAction *action = menu.exec(gp);
    QListWidgetItem *item;
    Qt::CheckState state;
    int i;
    bool dump;
    if (action)
    {
        if (action->text() == tr("All On"))
        {
            state = Qt::Checked;
            dump = true;
        }
        else
        {
            state = Qt::Unchecked;
            dump = false;
        }
        for (i = 0; i < listWidgetWarehouse->count(); i++)
        {
            item = listWidgetWarehouse->item(i);
            item->setCheckState(state);
        }
    }
}

void DialogOutputSelect::acceptButtonClicked()
{
    Preferences::insert("DialogOutputSelectGeometry", saveGeometry());
    accept();
}

void DialogOutputSelect::rejectButtonClicked()
{
    Preferences::insert("DialogOutputSelectGeometry", saveGeometry());
    reject();
}
