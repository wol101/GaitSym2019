/*
 *  DialogOutputSelect.cpp
 *  GaitSymODE2019
 *
 *  Created by Bill Sellers on 08/10/2018.
 *  Copyright 2018 Bill Sellers. All rights reserved.
 *
 */

#include "DialogOutputSelect.h"
#include "ui_DialogOutputSelect.h"

#include "Simulation.h"
#include "Muscle.h"
#include "Body.h"
#include "Marker.h"
#include "Joint.h"
#include "Geom.h"
#include "Driver.h"
#include "DataTarget.h"
#include "Reporter.h"
#include "Controller.h"
#include "Warehouse.h"
#include "MainWindow.h"
#include "Preferences.h"

#include <QMenu>
#include <QListWidget>
#include <QLabel>
#include <QGridLayout>
#include <QDialogButtonBox>
#include <QDebug>

DialogOutputSelect::DialogOutputSelect(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::DialogOutputSelect)
{
    m_ui->setupUi(this);
    setWindowTitle(tr("Output Select"));
#ifdef Q_OS_MACOS
    // on MacOS High Sierra (and possibly others), the Qt::Dialog flag disables resizing
    // if I clear the Qt::Dialog flag and make sure the Qt::Window flag is set (it should be already)
    // then it seems to let me have resizing without any other side effects
    setWindowFlags(windowFlags() & (~Qt::Dialog) | Qt::Window);
#endif

    gridLayout = new QGridLayout(m_ui->groupBoxOutputs);
    QFont listWidgetFont;
    listWidgetFont.setPointSize(10);

    QListWidgetItem *item;
    QLabel *label;
    int count;
    MainWindow *mainWindow = dynamic_cast<MainWindow *>(parent);
    Simulation *simulation = mainWindow->simulation();
    auto bodyList = simulation->GetBodyList();
    auto markerList = simulation->GetMarkerList();
    auto jointList = simulation->GetJointList();
    auto geomList = simulation->GetGeomList();
    auto muscleList = simulation->GetMuscleList();
    auto driverList = simulation->GetDriverList();
    auto dataTargetList = simulation->GetDataTargetList();
    auto reporterList = simulation->GetReporterList();
    auto controllerList = simulation->GetControllerList();
    auto warehouseList = simulation->GetWarehouseList();

    if (bodyList->size() > 0)
    {
        m_listWidgetBody = new QListWidget(this);
        m_listWidgetBody->setFont(listWidgetFont);
        m_listWidgetBody->setContextMenuPolicy(Qt::CustomContextMenu);
        count = 0;
        m_listWidgetBody->clear();
        for (auto &&bodyIterator : *bodyList)
        {
            m_listWidgetBody->addItem(bodyIterator.first.c_str());
            item = m_listWidgetBody->item(count++);
            if (bodyIterator.second->dump()) item->setCheckState(Qt::Checked);
            else item->setCheckState(Qt::Unchecked);
        }
        gridLayout->addWidget(m_listWidgetBody, 1, m_columns);
        label = new QLabel("Body List", this);
        gridLayout->addWidget(label, 0, m_columns);
        m_columns++;
        QObject::connect(m_listWidgetBody, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(menuRequestBody(QPoint)));
    }

    if (markerList->size() > 0)
    {
        m_listWidgetMarker = new QListWidget(this);
        m_listWidgetMarker->setFont(listWidgetFont);
        m_listWidgetMarker->setContextMenuPolicy(Qt::CustomContextMenu);
        count = 0;
        m_listWidgetMarker->clear();
        for (auto &&markerIterator : *markerList)
        {
            m_listWidgetMarker->addItem(markerIterator.first.c_str());
            item = m_listWidgetMarker->item(count++);
            if (markerIterator.second->dump()) item->setCheckState(Qt::Checked);
            else item->setCheckState(Qt::Unchecked);
        }
        gridLayout->addWidget(m_listWidgetMarker, 1, m_columns);
        label = new QLabel("Marker List", this);
        gridLayout->addWidget(label, 0, m_columns);
        m_columns++;
        QObject::connect(m_listWidgetMarker, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(menuRequestMarker(QPoint)));
    }

    if (jointList->size() > 0)
    {
        m_listWidgetJoint = new QListWidget(this);
        m_listWidgetJoint->setFont(listWidgetFont);
        m_listWidgetJoint->setContextMenuPolicy(Qt::CustomContextMenu);
        count = 0;
        m_listWidgetJoint->clear();
        for (auto &&jointIterator : *jointList)
        {
            m_listWidgetJoint->addItem(jointIterator.first.c_str());
            item = m_listWidgetJoint->item(count++);
            if (jointIterator.second->dump()) item->setCheckState(Qt::Checked);
            else item->setCheckState(Qt::Unchecked);
        }
        gridLayout->addWidget(m_listWidgetJoint, 1, m_columns);
        label = new QLabel("Joint List", this);
        gridLayout->addWidget(label, 0, m_columns);
        m_columns++;
        QObject::connect(m_listWidgetJoint, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(menuRequestJoint(QPoint)));
    }

    if (geomList->size() > 0)
    {
        m_listWidgetGeom = new QListWidget(this);
        m_listWidgetGeom->setFont(listWidgetFont);
        m_listWidgetGeom->setContextMenuPolicy(Qt::CustomContextMenu);
        count = 0;
        m_listWidgetGeom->clear();
        for (auto &&geomIterator : *geomList)
        {
            m_listWidgetGeom->addItem(geomIterator.first.c_str());
            item = m_listWidgetGeom->item(count++);
            if (geomIterator.second->dump()) item->setCheckState(Qt::Checked);
            else item->setCheckState(Qt::Unchecked);
        }
        gridLayout->addWidget(m_listWidgetGeom, 1, m_columns);
        label = new QLabel("Geom List", this);
        gridLayout->addWidget(label, 0, m_columns);
        m_columns++;
        QObject::connect(m_listWidgetGeom, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(menuRequestGeom(QPoint)));
    }

    if (muscleList->size() > 0)
    {
        m_listWidgetMuscle = new QListWidget(this);
        m_listWidgetMuscle->setFont(listWidgetFont);
        m_listWidgetMuscle->setContextMenuPolicy(Qt::CustomContextMenu);
        count = 0;
        m_listWidgetMuscle->clear();
        for (auto &&muscleIterator : *muscleList)
        {
            m_listWidgetMuscle->addItem(muscleIterator.first.c_str());
            item = m_listWidgetMuscle->item(count++);
            if (muscleIterator.second->dump()) item->setCheckState(Qt::Checked);
            else item->setCheckState(Qt::Unchecked);
        }
        gridLayout->addWidget(m_listWidgetMuscle, 1, m_columns);
        label = new QLabel("Muscle List", this);
        gridLayout->addWidget(label, 0, m_columns);
        m_columns++;
        QObject::connect(m_listWidgetMuscle, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(menuRequestMuscle(QPoint)));
    }

    if (driverList->size() > 0)
    {
        m_listWidgetDriver = new QListWidget(this);
        m_listWidgetDriver->setFont(listWidgetFont);
        m_listWidgetDriver->setContextMenuPolicy(Qt::CustomContextMenu);
        count = 0;
        m_listWidgetDriver->clear();
        for (auto &&driverIterator : *driverList)
        {
            m_listWidgetDriver->addItem(driverIterator.first.c_str());
            item = m_listWidgetDriver->item(count++);
            if (driverIterator.second->dump()) item->setCheckState(Qt::Checked);
            else item->setCheckState(Qt::Unchecked);
        }
        gridLayout->addWidget(m_listWidgetDriver, 1, m_columns);
        label = new QLabel("Driver List", this);
        gridLayout->addWidget(label, 0, m_columns);
        m_columns++;
        QObject::connect(m_listWidgetDriver, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(menuRequestDriver(QPoint)));
    }

    if (dataTargetList->size() > 0)
    {
        m_listWidgetDataTarget = new QListWidget(this);
        m_listWidgetDataTarget->setFont(listWidgetFont);
        m_listWidgetDataTarget->setContextMenuPolicy(Qt::CustomContextMenu);
        count = 0;
        m_listWidgetDataTarget->clear();
        for (auto &&dataTargetIterator : *dataTargetList)
        {
            m_listWidgetDataTarget->addItem(dataTargetIterator.first.c_str());
            item = m_listWidgetDataTarget->item(count++);
            if (dataTargetIterator.second->dump()) item->setCheckState(Qt::Checked);
            else item->setCheckState(Qt::Unchecked);
        }
        gridLayout->addWidget(m_listWidgetDataTarget, 1, m_columns);
        label = new QLabel("DataTarget List", this);
        gridLayout->addWidget(label, 0, m_columns);
        m_columns++;
        QObject::connect(m_listWidgetDataTarget, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(menuRequestDataTarget(QPoint)));
    }

    if (reporterList->size() > 0)
    {
        m_listWidgetReporter = new QListWidget(this);
        m_listWidgetReporter->setFont(listWidgetFont);
        m_listWidgetReporter->setContextMenuPolicy(Qt::CustomContextMenu);
        count = 0;
        m_listWidgetReporter->clear();
        for (auto &&reporterIterator : *reporterList)
        {
            m_listWidgetReporter->addItem(reporterIterator.first.c_str());
            item = m_listWidgetReporter->item(count++);
            if (reporterIterator.second->dump()) item->setCheckState(Qt::Checked);
            else item->setCheckState(Qt::Unchecked);
        }
        gridLayout->addWidget(m_listWidgetReporter, 1, m_columns);
        label = new QLabel("Reporter List", this);
        gridLayout->addWidget(label, 0, m_columns);
        m_columns++;
        QObject::connect(m_listWidgetReporter, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(menuRequestReporter(QPoint)));
    }

    if (controllerList->size() > 0)
    {
        m_listWidgetController = new QListWidget(this);
        m_listWidgetController->setFont(listWidgetFont);
        m_listWidgetController->setContextMenuPolicy(Qt::CustomContextMenu);
        count = 0;
        m_listWidgetController->clear();
        for (auto &&controllerIterator : *controllerList)
        {
            m_listWidgetController->addItem(controllerIterator.first.c_str());
            item = m_listWidgetController->item(count++);
            if (controllerIterator.second->dump()) item->setCheckState(Qt::Checked);
            else item->setCheckState(Qt::Unchecked);
        }
        gridLayout->addWidget(m_listWidgetController, 1, m_columns);
        label = new QLabel("Controller List", this);
        gridLayout->addWidget(label, 0, m_columns);
        m_columns++;
        QObject::connect(m_listWidgetController, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(menuRequestController(QPoint)));
    }

    if (warehouseList->size() > 0)
    {
        m_listWidgetWarehouse = new QListWidget(this);
        m_listWidgetWarehouse->setFont(listWidgetFont);
        m_listWidgetWarehouse->setContextMenuPolicy(Qt::CustomContextMenu);
        count = 0;
        m_listWidgetWarehouse->clear();
        for (auto &&warehouseIterator : *warehouseList)
        {
            m_listWidgetWarehouse->addItem(warehouseIterator.first.c_str());
            item = m_listWidgetWarehouse->item(count++);
            if (warehouseIterator.second->dump()) item->setCheckState(Qt::Checked);
            else item->setCheckState(Qt::Unchecked);
        }
        gridLayout->addWidget(m_listWidgetWarehouse, 1, m_columns);
        label = new QLabel("Warehouse List", this);
        gridLayout->addWidget(label, 0, m_columns);
        m_columns++;
        QObject::connect(m_listWidgetWarehouse, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(menuRequestWarehouse(QPoint)));
    }

    connect(m_ui->pushButtonOK, SIGNAL(clicked()), this, SLOT(acceptButtonClicked()));
    connect(m_ui->pushButtonCancel, SIGNAL(clicked()), this, SLOT(rejectButtonClicked()));

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

    QPoint gp = m_listWidgetMuscle->mapToGlobal(p);

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
        for (i = 0; i < m_listWidgetMuscle->count(); i++)
        {
            item = m_listWidgetMuscle->item(i);
            item->setCheckState(state);
        }
    }
}

void DialogOutputSelect::menuRequestBody(QPoint p)
{
    QMenu menu(this);
    menu.addAction(tr("All On"));
    menu.addAction(tr("All Off"));

    QPoint gp = m_listWidgetBody->mapToGlobal(p);

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
        for (i = 0; i < m_listWidgetBody->count(); i++)
        {
            item = m_listWidgetBody->item(i);
            item->setCheckState(state);
        }
    }
}

void DialogOutputSelect::menuRequestJoint(QPoint p)
{
    QMenu menu(this);
    menu.addAction(tr("All On"));
    menu.addAction(tr("All Off"));

    QPoint gp = m_listWidgetJoint->mapToGlobal(p);

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
        for (i = 0; i < m_listWidgetJoint->count(); i++)
        {
            item = m_listWidgetJoint->item(i);
            item->setCheckState(state);
        }
    }
}

void DialogOutputSelect::menuRequestMarker(QPoint p)
{
    QMenu menu(this);
    menu.addAction(tr("All On"));
    menu.addAction(tr("All Off"));

    QPoint gp = m_listWidgetMarker->mapToGlobal(p);

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
        for (i = 0; i < m_listWidgetMarker->count(); i++)
        {
            item = m_listWidgetMarker->item(i);
            item->setCheckState(state);
        }
    }
}

void DialogOutputSelect::menuRequestGeom(QPoint p)
{
    QMenu menu(this);
    menu.addAction(tr("All On"));
    menu.addAction(tr("All Off"));

    QPoint gp = m_listWidgetGeom->mapToGlobal(p);

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
        for (i = 0; i < m_listWidgetGeom->count(); i++)
        {
            item = m_listWidgetGeom->item(i);
            item->setCheckState(state);
        }
    }
}

void DialogOutputSelect::menuRequestDriver(QPoint p)
{
    QMenu menu(this);
    menu.addAction(tr("All On"));
    menu.addAction(tr("All Off"));

    QPoint gp = m_listWidgetDriver->mapToGlobal(p);

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
        for (i = 0; i < m_listWidgetDriver->count(); i++)
        {
            item = m_listWidgetDriver->item(i);
            item->setCheckState(state);
        }
    }
}

void DialogOutputSelect::menuRequestDataTarget(QPoint p)
{
    QMenu menu(this);
    menu.addAction(tr("All On"));
    menu.addAction(tr("All Off"));

    QPoint gp = m_listWidgetDataTarget->mapToGlobal(p);

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
        for (i = 0; i < m_listWidgetDataTarget->count(); i++)
        {
            item = m_listWidgetDataTarget->item(i);
            item->setCheckState(state);
        }
    }
}

void DialogOutputSelect::menuRequestReporter(QPoint p)
{
    QMenu menu(this);
    menu.addAction(tr("All On"));
    menu.addAction(tr("All Off"));

    QPoint gp = m_listWidgetReporter->mapToGlobal(p);

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
        for (i = 0; i < m_listWidgetReporter->count(); i++)
        {
            item = m_listWidgetReporter->item(i);
            item->setCheckState(state);
        }
    }
}

void DialogOutputSelect::menuRequestController(QPoint p)
{
    QMenu menu(this);
    menu.addAction(tr("All On"));
    menu.addAction(tr("All Off"));

    QPoint gp = m_listWidgetController->mapToGlobal(p);

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
        for (i = 0; i < m_listWidgetController->count(); i++)
        {
            item = m_listWidgetController->item(i);
            item->setCheckState(state);
        }
    }
}

void DialogOutputSelect::menuRequestWarehouse(QPoint p)
{
    QMenu menu(this);
    menu.addAction(tr("All On"));
    menu.addAction(tr("All Off"));

    QPoint gp = m_listWidgetWarehouse->mapToGlobal(p);

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
        for (i = 0; i < m_listWidgetWarehouse->count(); i++)
        {
            item = m_listWidgetWarehouse->item(i);
            item->setCheckState(state);
        }
    }
}

void DialogOutputSelect::acceptButtonClicked()
{
    Q_ASSERT_X(m_simulation, "DialogOutputSelect::acceptButtonClicked", "m_simulation undefined");
    qDebug() << "DialogOutputSelect::acceptButtonClicked()";

    QListWidgetItem *item;
    int i;
    bool dump;
    for (i = 0; listWidgetBody() && i < listWidgetBody()->count(); i++)
    {
        item = listWidgetBody()->item(i);
        if (item->checkState() == Qt::Unchecked) dump = false;
        else dump = true;
        (*m_simulation->GetBodyList())[std::string(item->text().toUtf8())]->setDump(dump);
    }

    for (i = 0; listWidgetMuscle() && i < listWidgetMuscle()->count(); i++)
    {
        item = listWidgetMuscle()->item(i);
        if (item->checkState() == Qt::Unchecked) dump = false;
        else dump = true;
        (*m_simulation->GetMuscleList())[std::string(item->text().toUtf8())]->setDump(dump);
        (*m_simulation->GetMuscleList())[std::string(item->text().toUtf8())]->GetStrap()->setDump(dump);
    }

    for (i = 0; listWidgetGeom() && i < listWidgetGeom()->count(); i++)
    {
        item = listWidgetGeom()->item(i);
        if (item->checkState() == Qt::Unchecked) dump = false;
        else dump = true;
        (*m_simulation->GetGeomList())[std::string(item->text().toUtf8())]->setDump(dump);
    }

    for (i = 0; listWidgetJoint() && i < listWidgetJoint()->count(); i++)
    {
        item = listWidgetJoint()->item(i);
        if (item->checkState() == Qt::Unchecked) dump = false;
        else dump = true;
        (*m_simulation->GetJointList())[std::string(item->text().toUtf8())]->setDump(dump);
    }

    for (i = 0; listWidgetDriver() && i < listWidgetDriver()->count(); i++)
    {
        item = listWidgetDriver()->item(i);
        if (item->checkState() == Qt::Unchecked) dump = false;
        else dump = true;
        (*m_simulation->GetDriverList())[std::string(item->text().toUtf8())]->setDump(dump);
    }

    for (i = 0; listWidgetDataTarget() && i < listWidgetDataTarget()->count(); i++)
    {
        item = listWidgetDataTarget()->item(i);
        if (item->checkState() == Qt::Unchecked) dump = false;
        else dump = true;
        (*m_simulation->GetDataTargetList())[std::string(item->text().toUtf8())]->setDump(dump);
    }

    for (i = 0; listWidgetReporter() && i < listWidgetReporter()->count(); i++)
    {
        item = listWidgetReporter()->item(i);
        if (item->checkState() == Qt::Unchecked) dump = false;
        else dump = true;
        (*m_simulation->GetReporterList())[std::string(item->text().toUtf8())]->setDump(dump);
    }

    for (i = 0; listWidgetController() && i < listWidgetController()->count(); i++)
    {
        item = listWidgetController()->item(i);
        if (item->checkState() == Qt::Unchecked) dump = false;
        else dump = true;
        (*m_simulation->GetControllerList())[std::string(item->text().toUtf8())]->setDump(dump);
    }

    for (i = 0; listWidgetWarehouse() && i < listWidgetWarehouse()->count(); i++)
    {
        item = listWidgetWarehouse()->item(i);
        if (item->checkState() == Qt::Unchecked) dump = false;
        else dump = true;
        (*m_simulation->GetWarehouseList())[std::string(item->text().toUtf8())]->setDump(dump);
    }

    Preferences::insert("DialogOutputSelectGeometry", saveGeometry());
    accept();
}

void DialogOutputSelect::rejectButtonClicked()
{
    qDebug() << "DialogOutputSelect::rejectButtonClicked()";
    Preferences::insert("DialogOutputSelectGeometry", saveGeometry());
    reject();
}

void DialogOutputSelect::closeEvent(QCloseEvent *event)
{
    qDebug() << "DialogOutputSelect::closeEvent()";
    Preferences::insert("DialogOutputSelectGeometry", saveGeometry());

    QDialog::closeEvent(event);
}

Simulation *DialogOutputSelect::simulation() const
{
    return m_simulation;
}

void DialogOutputSelect::setSimulation(Simulation *simulation)
{
    m_simulation = simulation;
}

QListWidget *DialogOutputSelect::listWidgetWarehouse() const
{
    return m_listWidgetWarehouse;
}

void DialogOutputSelect::setListWidgetWarehouse(QListWidget *listWidgetWarehouse)
{
    m_listWidgetWarehouse = listWidgetWarehouse;
}

QListWidget *DialogOutputSelect::listWidgetController() const
{
    return m_listWidgetController;
}

void DialogOutputSelect::setListWidgetController(QListWidget *listWidgetController)
{
    m_listWidgetController = listWidgetController;
}

QListWidget *DialogOutputSelect::listWidgetReporter() const
{
    return m_listWidgetReporter;
}

void DialogOutputSelect::setListWidgetReporter(QListWidget *listWidgetReporter)
{
    m_listWidgetReporter = listWidgetReporter;
}

QListWidget *DialogOutputSelect::listWidgetMuscle() const
{
    return m_listWidgetMuscle;
}

void DialogOutputSelect::setListWidgetMuscle(QListWidget *listWidgetMuscle)
{
    m_listWidgetMuscle = listWidgetMuscle;
}

QListWidget *DialogOutputSelect::listWidgetMarker() const
{
    return m_listWidgetMarker;
}

void DialogOutputSelect::setListWidgetMarker(QListWidget *listWidgetMarker)
{
    m_listWidgetMarker = listWidgetMarker;
}

QListWidget *DialogOutputSelect::listWidgetJoint() const
{
    return m_listWidgetJoint;
}

void DialogOutputSelect::setListWidgetJoint(QListWidget *listWidgetJoint)
{
    m_listWidgetJoint = listWidgetJoint;
}

QListWidget *DialogOutputSelect::listWidgetGeom() const
{
    return m_listWidgetGeom;
}

void DialogOutputSelect::setListWidgetGeom(QListWidget *listWidgetGeom)
{
    m_listWidgetGeom = listWidgetGeom;
}

QListWidget *DialogOutputSelect::listWidgetDriver() const
{
    return m_listWidgetDriver;
}

void DialogOutputSelect::setListWidgetDriver(QListWidget *listWidgetDriver)
{
    m_listWidgetDriver = listWidgetDriver;
}

QListWidget *DialogOutputSelect::listWidgetDataTarget() const
{
    return m_listWidgetDataTarget;
}

void DialogOutputSelect::setListWidgetDataTarget(QListWidget *listWidgetDataTarget)
{
    m_listWidgetDataTarget = listWidgetDataTarget;
}

QListWidget *DialogOutputSelect::listWidgetBody() const
{
    return m_listWidgetBody;
}

void DialogOutputSelect::setListWidgetBody(QListWidget *listWidgetBody)
{
    m_listWidgetBody = listWidgetBody;
}


