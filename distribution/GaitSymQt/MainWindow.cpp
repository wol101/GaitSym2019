/*
 *  MainWindow.cpp
 *  GaitSymODE2019
 *
 *  Created by Bill Sellers on 08/10/2018.
 *  Copyright 2018 Bill Sellers. All rights reserved.
 *
 */

#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "DialogPreferences.h"
#include "DialogOutputSelect.h"
#include "ui_DialogOutputSelect.h"
#include "AboutDialog.h"
#include "ViewControlWidget.h"
#include "ObjectiveMain.h"
#include "Simulation.h"
#include "DataFile.h"
#include "Muscle.h"
#include "Body.h"
#include "Joint.h"
#include "Geom.h"
#include "Marker.h"
#include "FluidSac.h"
#include "Driver.h"
#include "DataTarget.h"
#include "FacetedObject.h"
#include "Reporter.h"
#include "Warehouse.h"
#include "Preferences.h"
#include "SimulationWindow.h"
#include "AVIWriter.h"
#include "do_genetic_algorithm.h"
#include "do_next_ascent_hillclimbing.h"
#include "do_random_ascent_hillclimbing.h"
#include "do_simplex_search.h"
#include "do_simulated_annealling.h"
#include "do_tabu_search.h"
#include "DialogBodyBuilder.h"
#include "DialogGlobal.h"
#include "DialogMarkers.h"
#include "DialogJoints.h"
#include "DialogMuscles.h"
#include "DialogGeoms.h"
#include "DialogDrivers.h"
#include "Colour.h"

#include <QMessageBox>
#include <QTimer>
#include <QFileDialog>
#include <QBoxLayout>
#include <QDesktopWidget>
#include <QListWidgetItem>
#include <QLineEdit>
#include <QFile>
#include <QKeyEvent>
#include <QRegExp>
#include <QDir>
#include <QStringList>
#include <QTemporaryFile>
#include <QSizePolicy>
#include <QApplication>
#include <QProgressDialog>
#include <QThread>
#include <QComboBox>
#include <QTreeWidgetItem>
#include <QtGlobal>

#include <algorithm>

using namespace std::literals::string_literals;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    // read in the Preferences file
    Preferences::Read();

    // create the window elements
    ui->setupUi(this);

    // interface related connections
    connect(ui->actionNew, SIGNAL(triggered()), this, SLOT(menuNew()));
    connect(ui->action_Open, SIGNAL(triggered()), this, SLOT(open()));
    connect(ui->action_Quit, SIGNAL(triggered()), this, SLOT(close()));
    connect(ui->action_Restart, SIGNAL(triggered()), this, SLOT(restart()));
    connect(ui->action_Save, SIGNAL(triggered()), this, SLOT(saveas()));
    connect(ui->action1280x720, SIGNAL(triggered()), this, SLOT(menu1280x720()));
    connect(ui->action1920x1080, SIGNAL(triggered()), this, SLOT(menu1920x1080()));
    connect(ui->action640x480, SIGNAL(triggered()), this, SLOT(menu640x480()));
    connect(ui->action800x600, SIGNAL(triggered()), this, SLOT(menu800x600()));
    connect(ui->actionAbout_GaitSymQt, SIGNAL(triggered()), this, SLOT(about()));
    connect(ui->actionCopy, SIGNAL(triggered()), this, SLOT(copy()));
    connect(ui->actionCut, SIGNAL(triggered()), this, SLOT(cut()));
    connect(ui->actionDefault_View, SIGNAL(triggered()), this, SLOT(menuDefaultView()));
    connect(ui->actionImport_Warehouse, SIGNAL(triggered()), this, SLOT(menuImportWarehouse()));
    connect(ui->actionOutput, SIGNAL(triggered()), this, SLOT(menuOutputs()));
    connect(ui->actionPaste, SIGNAL(triggered()), this, SLOT(paste()));
    connect(ui->actionPreferences, SIGNAL(triggered()), this, SLOT(menuPreferences()));
    connect(ui->actionRecord_Movie, SIGNAL(triggered()), this, SLOT(menuRecordMovie()));
    connect(ui->actionRun, SIGNAL(triggered()), this, SLOT(run()));
    connect(ui->actionSaveAs, SIGNAL(triggered()), this, SLOT(saveas()));
    connect(ui->actionSaveDefaultView, SIGNAL(triggered()), this, SLOT(menuSaveDefaultView()));
    connect(ui->actionSelect_all, SIGNAL(triggered()), this, SLOT(selectAll()));
    connect(ui->actionSnapshot, SIGNAL(triggered()), this, SLOT(snapshot()));
    connect(ui->actionStart_Warehouse_Export, SIGNAL(triggered()), this, SLOT(menuStartWarehouseExport()));
    connect(ui->actionStep, SIGNAL(triggered()), this, SLOT(step()));
    connect(ui->actionStop_Warehouse_Export, SIGNAL(triggered()), this, SLOT(menuStopWarehouseExport()));
    connect(ui->actionToggleFullscreen, SIGNAL(triggered()), this, SLOT(menuToggleFullScreen()));
    connect(ui->actionViewFront, SIGNAL(triggered()), this, SLOT(buttonCameraFront()));
    connect(ui->actionViewRight, SIGNAL(triggered()), this, SLOT(buttonCameraRight()));
    connect(ui->actionViewTop, SIGNAL(triggered()), this, SLOT(buttonCameraTop()));
    connect(ui->actionViewBack, SIGNAL(triggered()), this, SLOT(buttonCameraBack()));
    connect(ui->actionViewLeft, SIGNAL(triggered()), this, SLOT(buttonCameraLeft()));
    connect(ui->actionViewBottom, SIGNAL(triggered()), this, SLOT(buttonCameraBottom()));
    connect(ui->actionSave_OBJ_Snapshot, SIGNAL(triggered()), this, SLOT(objSnapshot()));
    connect(ui->actionStart_OBJ_Sequence, SIGNAL(triggered()), this, SLOT(menuStartOBJSequenceSave()));
    connect(ui->actionStop_OBJ_Sequence, SIGNAL(triggered()), this, SLOT(menuStopOBJSequenceSave()));
    connect(ui->actionGenetic_Algorithm, SIGNAL(triggered()), this, SLOT(menuGeneticAlgorithm()));
    connect(ui->actionNext_Ascent_Hillclimbing, SIGNAL(triggered()), this, SLOT(menuNextAscentHillclimbing()));
    connect(ui->actionRandom_Ascent_Hillclimbing, SIGNAL(triggered()), this, SLOT(menuRandomAscentHillclimbing()));
    connect(ui->actionSimplex_Search, SIGNAL(triggered()), this, SLOT(menuSimplexSearch()));
    connect(ui->actionSimulated_Annealing, SIGNAL(triggered()), this, SLOT(menuSimulatedAnnealing()));
    connect(ui->actionTabu_Search, SIGNAL(triggered()), this, SLOT(menuTabuSearch()));
    connect(ui->actionCreateBody, SIGNAL(triggered()), this, SLOT(menuCreateBody()));
    connect(ui->actionCreateMarker, SIGNAL(triggered()), this, SLOT(menuCreateMarker()));
    connect(ui->actionCreateJoint, SIGNAL(triggered()), this, SLOT(menuCreateJoint()));
    connect(ui->actionCreateMuscle, SIGNAL(triggered()), this, SLOT(menuCreateMuscle()));
    connect(ui->actionCreateGeom, SIGNAL(triggered()), this, SLOT(menuCreateGeom()));
    connect(ui->actionCreateDriver, SIGNAL(triggered()), this, SLOT(menuCreateDriver()));
    connect(ui->actionEditGlobal, SIGNAL(triggered()), this, SLOT(menuEditGlobal()));
    connect(ui->actionRunMode, SIGNAL(triggered()), this, SLOT(enterRunMode()));
    connect(ui->actionConstructionMode, SIGNAL(triggered()), this, SLOT(enterConstructionMode()));
    connect(ui->checkBoxActivationColours, SIGNAL(stateChanged(int)), this, SLOT(checkboxActivationColours(int)));
    connect(ui->checkBoxContactForce, SIGNAL(stateChanged(int)), this, SLOT(checkboxContactForce(int)));
    connect(ui->checkBoxMuscleForce, SIGNAL(stateChanged(int)), this, SLOT(checkboxMuscleForce(int)));
    connect(ui->checkBoxTracking, SIGNAL(stateChanged(int)), this, SLOT(checkboxTracking(int)));
    connect(ui->doubleSpinBoxCOIX, SIGNAL(valueChanged(double)), this, SLOT(spinboxCOIXChanged(double)));
    connect(ui->doubleSpinBoxCOIY, SIGNAL(valueChanged(double)), this, SLOT(spinboxCOIYChanged(double)));
    connect(ui->doubleSpinBoxCOIZ, SIGNAL(valueChanged(double)), this, SLOT(spinboxCOIZChanged(double)));
    connect(ui->doubleSpinBoxCursorNudge, SIGNAL(valueChanged(double)), this, SLOT(spinboxCursorNudgeChanged(double)));
    connect(ui->doubleSpinBoxCursorSize, SIGNAL(valueChanged(double)), this, SLOT(spinboxCursorSizeChanged(double)));
    connect(ui->doubleSpinBoxDistance, SIGNAL(valueChanged(double)), this, SLOT(spinboxDistanceChanged(double)));
    connect(ui->doubleSpinBoxFar, SIGNAL(valueChanged(double)), this, SLOT(spinboxFarChanged(double)));
    connect(ui->doubleSpinBoxFoV, SIGNAL(valueChanged(double)), this, SLOT(spinboxFoVChanged(double)));
    connect(ui->doubleSpinBoxFPS, SIGNAL(valueChanged(double)), this, SLOT(spinboxFPSChanged(double)));
    connect(ui->doubleSpinBoxNear, SIGNAL(valueChanged(double)), this, SLOT(spinboxNearChanged(double)));
    connect(ui->doubleSpinBoxTimeMax, SIGNAL(valueChanged(double)), this, SLOT(spinboxTimeMax(double)));
    connect(ui->doubleSpinBoxTrackingOffset, SIGNAL(valueChanged(double)), this, SLOT(spinboxTrackingOffsetChanged(double)));
    connect(ui->spinBoxSkip, SIGNAL(valueChanged(int)), this, SLOT(spinboxSkip(int)));
    connect(ui->treeWidgetElements, SIGNAL(createNewBody()), this, SLOT(menuCreateBody()));
    connect(ui->treeWidgetElements, SIGNAL(createNewMarker()), this, SLOT(menuCreateMarker()));
    connect(ui->treeWidgetElements, SIGNAL(createNewJoint()), this, SLOT(menuCreateJoint()));
    connect(ui->treeWidgetElements, SIGNAL(createNewMuscle()), this, SLOT(menuCreateMuscle()));
    connect(ui->treeWidgetElements, SIGNAL(createNewGeom()), this, SLOT(menuCreateGeom()));
    connect(ui->treeWidgetElements, SIGNAL(createNewDriver()), this, SLOT(menuCreateDriver()));
    connect(ui->treeWidgetElements, SIGNAL(deleteBody(const QString &)), this, SLOT(deleteExistingBody(const QString &)));
    connect(ui->treeWidgetElements, SIGNAL(deleteMarker(const QString &)), this, SLOT(deleteExistingMarker(const QString &)));
    connect(ui->treeWidgetElements, SIGNAL(deleteJoint(const QString &)), this, SLOT(deleteExistingJoint(const QString &)));
    connect(ui->treeWidgetElements, SIGNAL(deleteMuscle(const QString &)), this, SLOT(deleteExistingMuscle(const QString &)));
    connect(ui->treeWidgetElements, SIGNAL(deleteGeom(const QString &)), this, SLOT(deleteExistingGeom(const QString &)));
    connect(ui->treeWidgetElements, SIGNAL(deleteDriver(const QString &)), this, SLOT(deleteExistingDriver(const QString &)));
    connect(ui->treeWidgetElements, SIGNAL(editBody(const QString &)), this, SLOT(editExistingBody(const QString &)));
    connect(ui->treeWidgetElements, SIGNAL(editMarker(const QString &)), this, SLOT(editExistingMarker(const QString &)));
    connect(ui->treeWidgetElements, SIGNAL(editJoint(const QString &)), this, SLOT(editExistingJoint(const QString &)));
    connect(ui->treeWidgetElements, SIGNAL(editMuscle(const QString &)), this, SLOT(editExistingMuscle(const QString &)));
    connect(ui->treeWidgetElements, SIGNAL(editGeom(const QString &)), this, SLOT(editExistingGeom(const QString &)));
    connect(ui->treeWidgetElements, SIGNAL(editDriver(const QString &)), this, SLOT(editExistingDriver(const QString &)));
    connect(ui->treeWidgetElements, SIGNAL(elementTreeWidgetItemChanged(QTreeWidgetItem *, int)), this, SLOT(handleElementTreeWidgetItemChanged(QTreeWidgetItem *, int)));

    // put SimulationWindow into existing widgetGLWidget
    QBoxLayout *boxLayout = new QBoxLayout(QBoxLayout::LeftToRight, ui->widgetGLWidget);
    boxLayout->setMargin(0);
    m_simulationWindow = new SimulationWindow();
    QWidget *container = QWidget::createWindowContainer(m_simulationWindow);
    boxLayout->addWidget(container);
    m_simulationWindow->setMainWindow(this);
    m_simulationWindow->requestUpdate();

    // connect the ViewControlWidget to the GLWidget
    QObject::connect(ui->widgetViewFrame, SIGNAL(EmitCameraVec(double, double, double)),
                     m_simulationWindow, SLOT(SetCameraVec(double, double, double)));

    // connect the SimulationWindow to the MainWindow
    QObject::connect(m_simulationWindow, SIGNAL(EmitStatusString(const QString &, int)), this,
                     SLOT(setStatusString(const QString &, int)));
    QObject::connect(m_simulationWindow, SIGNAL(EmitCOI(float, float, float)), this,
                     SLOT(setUICOI(float, float, float)));
    QObject::connect(m_simulationWindow, SIGNAL(EmitFoV(float)), this, SLOT(setUIFoV(float)));
    QObject::connect(m_simulationWindow, SIGNAL(EmitCreateMarkerRequest()), this,
                     SLOT(menuCreateMarker()));
    QObject::connect(m_simulationWindow, SIGNAL(EmitEditMarkerRequest(const QString &)), this,
                     SLOT(editExistingMarker(const QString &)));
    QObject::connect(m_simulationWindow, SIGNAL(EmitMoveMarkerRequest(const QString &,
                     const QVector3D &)), this,
                     SLOT(moveExistingMarker(const QString &, const QVector3D &)));
    QObject::connect(m_simulationWindow, SIGNAL(EmitEditBodyRequest(const QString &)), this,
                     SLOT(editExistingBody(const QString &)));
    QObject::connect(m_simulationWindow, SIGNAL(EmitEditGeomRequest(const QString &)), this,
                     SLOT(editExistingGeom(const QString &)));
    QObject::connect(m_simulationWindow, SIGNAL(EmitEditJointRequest(const QString &)), this,
                     SLOT(editExistingJoint(const QString &)));
    QObject::connect(m_simulationWindow, SIGNAL(EmitEditMuscleRequest(const QString &)), this,
                     SLOT(editExistingMuscle(const QString &)));

    // add the combo box to the toolbar
    QComboBox *comboBox = new QComboBox;
    ui->toolBar->addWidget(comboBox);
    comboBox->addItems({ "Mesh 1", "Mesh 2", "Mesh 3"});
    QObject::connect(comboBox, SIGNAL(currentTextChanged(const QString &)), this,
                     SLOT(comboBoxCurrentTextChanged(const QString &)));
    m_simulationWindow->setBodyMeshNumber(1);

    // the treeWidgetElements needs to know about this window
    ui->treeWidgetElements->setMainWindow(this);

    // set up the timer
    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(processOneThing()));

    // zero the timer display
    QString time = QString("%1").arg(double(0), 0, 'f', 5);
    ui->lcdNumberTime->display(time);

    // intialise parts of the interface
    setInterfaceValues();
    setStatusString(tr("Ready"), 2);
    updateEnable();

#ifndef EXPERIMENTAL
    ui->actionStart_Warehouse_Export->setVisible(false);
    ui->actionStop_Warehouse_Export->setVisible(false);
    ui->actionImport_Warehouse->setVisible(false);
#endif

    setUnifiedTitleAndToolBarOnMac(false);

    // finally remember the geometry etc.
    restoreGeometry(Preferences::valueQByteArray("MainWindowGeometry"));
    restoreState(Preferences::valueQByteArray("MainWindowState"));
    ui->splitter1->restoreState(Preferences::valueQByteArray("MainWindowSplitter1State"));
    ui->splitter2->restoreState(Preferences::valueQByteArray("MainWindowSplitter2State"));
    Preferences::insert("ElementTreeHeaderState", ui->treeWidgetElements->header()->saveState());

}

MainWindow::~MainWindow()
{
    m_timer->stop();

    if (m_simulation) delete m_simulation;
    delete ui;

}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (m_saveRequired)
    {
        QMessageBox msgBox;
        msgBox.setText("The document has been modified.");
        msgBox.setInformativeText("Click OK to quit, and Cancel to return to continue working on the document");
        msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Cancel);
        int ret = msgBox.exec();
        switch (ret)
        {
        case QMessageBox::Ok:
            writeSettings();
            event->accept();
            break;
        case QMessageBox::Cancel:
            event->ignore();
            break;
        default:
            // should never be reached
            event->ignore();
            break;
        }
    }
    else
    {
        writeSettings();
        event->accept();
    }
}

void MainWindow::open()
{
    QFileInfo info = Preferences::valueQString("LastFileOpened");
    QString fileName;

    fileName = QFileDialog::getOpenFileName(this, tr("Open Config File"), info.absoluteFilePath(),
                                            tr("Config Files (*.xml)"), nullptr);
    if (fileName.isNull() == false)
    {
        Preferences::insert("LastFileOpened", fileName);
        open(fileName);
    }
}

void MainWindow::open(const QString &fileName)
{
    // dispose any simulation cleanly
    m_timer->stop();
    ui->actionRun->setChecked(false);
    m_movieFlag = false;
    if (m_simulationWindow->aviWriter()) menuStopAVISave();
    if (m_simulation)
    {
        delete m_simulation;
        m_simulation = nullptr;
    }
    m_stepCount = 0;
    m_stepFlag = false;

    m_configFile.setFile(fileName);
    QDir::setCurrent(m_configFile.absolutePath());
    Preferences::insert("LastFileOpened", m_configFile.absoluteFilePath());

    setStatusString(fileName + QString(" loading"), 2);
    qApp->processEvents();

    DataFile file;
    int err;
    err = file.ReadFile(m_configFile.absoluteFilePath().toStdString());
    if (err)
    {
        setStatusString(QString("Error reading ") + m_configFile.absoluteFilePath(), 0);
        return;
    }
    m_simulation = new Simulation();
    m_simulation->SetMainWindow(this);

    std::string *errorMessage = m_simulation->LoadModel(file.GetRawData(), file.GetSize());
    if (errorMessage)
    {
        setStatusString(QString::fromStdString(*errorMessage), 0);
        return;
    }
    ui->treeWidgetElements->fillVisibitilityLists(m_simulation);

    // check we can find the meshes
    QStringList searchPath = QString::fromStdString(
                                 m_simulation->GetGlobal().MeshSearchPath()).split(':');
    for (auto iter : *m_simulation->GetBodyList())
    {
        QString path = QString::fromStdString(iter.second->GetGraphicFile1());
        QFileInfo fileInfo(path);
        if (fileInfo.isFile() == false)
        {
            QString file = fileInfo.fileName();
            bool fileFound = false;
            foreach (auto testPath, searchPath)
            {
                QString testFile = QDir(testPath).absoluteFilePath(file);
                QFileInfo testFileInfo(testFile);
                if (testFileInfo.isFile())
                {
                    iter.second->SetGraphicFile1(testFile.toStdString());
                    fileFound = true;
                    break;
                }
            }
            if (fileFound == false)
            {
                int ret = QMessageBox::warning(this, QString("Loading ") + m_configFile.absoluteFilePath(),
                                               QString("Unable to find \"") + path + QString("\"\nDo you want to load a new file?"),
                                               QMessageBox::Yes | QMessageBox::No | QMessageBox::NoToAll, QMessageBox::Yes);
                if (ret == QMessageBox::NoToAll) break;
                if (ret == QMessageBox::Yes)
                {
                    QString newFile = QFileDialog::getOpenFileName(this, tr("Open Mesh File"),
                                      fileInfo.absoluteFilePath(), tr("Mesh Files (*.obj *.ply)"), nullptr);
                    if (newFile.isNull() == false)
                    {
                        iter.second->SetGraphicFile1(newFile.toStdString());
                        searchPath.append(QFileInfo(newFile).absolutePath());
                    }
                }
            }
        }
    }

    m_simulationWindow->setAxesScale(float(m_simulation->GetGlobal().size1()));
    m_simulationWindow->setBackgroundColour(m_simulation->GetGlobal().colour1().GetHexArgb().c_str());
    m_simulationWindow->initialiseScene();
    m_simulationWindow->setSimulation(m_simulation);
    m_simulationWindow->updateModel();
    // m_simulation->setDrawContactForces(Preferences::valueBool("DisplayContactForces"));
    //  m_simulation->Draw(m_simulationWindow);
    if (Preferences::valueBool("TrackingFlag"))
    {
        Body *body = m_simulation->GetBody(Preferences::valueQString("TrackBodyID").toStdString());
        if (body)
        {
            const double *position = dBodyGetPosition(body->GetBodyID());
            m_simulationWindow->setCOIx(position[0] + Preferences::valueDouble("TrackingOffset"));
            ui->doubleSpinBoxCOIX->setValue(position[0] + Preferences::valueDouble("TrackingOffset"));
        }
    }

    m_simulationWindow->updateCamera();

    ui->doubleSpinBoxTimeMax->setValue(m_simulation->GetTimeLimit());
    QString time = QString("%1").arg(double(0), 0, 'f', 5);
    ui->lcdNumberTime->display(time);

    setStatusString(fileName + QString(" loaded"), 1);

    // put the filename as a title
    if (fileName.size() <= 256) setWindowTitle(fileName);
    else setWindowTitle(QString("...") + fileName.right(256));

    // set menu activations for loaded model
    m_noName = false;
    m_saveRequired = false;
    m_mode = runMode;
    updateEnable();
}


void MainWindow::restart()
{
    open(Preferences::valueQString("LastFileOpened"));
}

void MainWindow::saveas()
{
    QFileInfo info = Preferences::valueQString("LastFileOpened");

    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Model State File (Relative)"),
                       info.absolutePath(), tr("Config Files (*.xml)"), nullptr);

    if (fileName.isNull() == false)
    {
        setStatusString(fileName + QString(" saving"), 2);
        m_simulation->SetModelStateRelative(true);
        m_simulation->SetOutputModelStateFile(fileName.toStdString());
        m_simulation->OutputProgramState();
        setStatusString(fileName + QString(" saved"), 1);
        m_configFile.setFile(fileName);
        QDir::setCurrent(m_configFile.absolutePath());
        Preferences::insert("LastFileOpened", m_configFile.absoluteFilePath());
        if (fileName.size() <= 256) setWindowTitle(fileName);
        else setWindowTitle(QString("...") + fileName.right(256));
        m_noName = false;
        m_saveRequired = false;
    }
}

void MainWindow::save()
{
    if (m_noName) return;
    QString fileName = m_configFile.absoluteFilePath();
    setStatusString(fileName + QString(" saving"), 2);
    m_simulation->SetModelStateRelative(false);
    m_simulation->SetOutputModelStateFile(fileName.toStdString());
    m_simulation->OutputProgramState();
    setStatusString(fileName + QString(" saved"), 1);
    m_saveRequired = false;
}


void MainWindow::about()
{
    AboutDialog aboutDialog(this);

    int status = aboutDialog.exec();

    if (status == QDialog::Accepted)
    {
    }
}

void MainWindow::run()
{
    if (ui->actionRun->isChecked())
    {
        if (m_simulation) m_timer->start();
        setStatusString(tr("Simulation running"), 1);
    }
    else
    {
        m_timer->stop();
        setStatusString(tr("Simulation stopped"), 1);
    }
}

void MainWindow::menuRecordMovie()
{
    if (ui->actionRecord_Movie->isChecked())
    {
        m_movieFlag = true;
        menuStartAVISave();
    }
    else
    {
        m_movieFlag = false;
        if (m_simulationWindow->aviWriter()) menuStopAVISave();
    }
}

void MainWindow::step()
{
    m_stepFlag = true;
    if (m_simulation) m_timer->start();
    setStatusString(tr("Simulation stepped"), 2);
}

void MainWindow::processOneThing()
{
    if (m_simulation)
    {
        if (m_simulation->ShouldQuit() || m_simulation->TestForCatastrophy())
        {
            setStatusString(tr("Unable to start simulation"), 1);
            ui->actionRun->setChecked(false);
            run();
            return;
        }

        m_simulation->UpdateSimulation();
        m_stepCount++;

        if ((m_stepCount % Preferences::valueInt("MovieSkip")) == 0)
        {
            if (Preferences::valueBool("TrackingFlag"))
            {
                Body *body = m_simulation->GetBody(Preferences::valueQString("TrackBodyID").toStdString());
                if (body)
                {
                    const double *position = dBodyGetPosition(body->GetBodyID());
                    m_simulationWindow->setCOIx(position[0] + Preferences::valueDouble("TrackingOffset"));
                    ui->doubleSpinBoxCOIX->setValue(position[0] + Preferences::valueDouble("TrackingOffset"));
                }
            }
            if (m_stepFlag)
            {
                m_stepFlag = false;
                m_timer->stop();
            }
            m_simulationWindow->updateModel();
            m_simulationWindow->updateCamera();
            if (m_movieFlag)
            {
                m_simulationWindow->WriteMovieFrame();
            }
            if (m_saveOBJFileSequenceFlag)
            {
                QString filename = QString("%1%2").arg("Frame").arg(m_simulation->GetTime(), 12, 'f', 7,
                                   QChar('0'));
                QString path = QDir(m_objFileSequenceFolder).filePath(filename);
                m_simulationWindow->WriteCADFrame(path);
            }
            QString time = QString("%1").arg(m_simulation->GetTime(), 0, 'f', 5);
            ui->lcdNumberTime->display(time);
        }

        if (m_simulation->ShouldQuit())
        {
            setStatusString(tr("Simulation ended normally"), 1);
            ui->textEditLog->append(QString("Fitness = %1\n").arg(m_simulation->CalculateInstantaneousFitness(),
                                    0, 'f', 5));
            ui->textEditLog->append(QString("Time = %1\n").arg(m_simulation->GetTime(), 0, 'f', 5));
            ui->textEditLog->append(QString("Metabolic Energy = %1\n").arg(m_simulation->GetMetabolicEnergy(),
                                    0, 'f', 5));
            ui->textEditLog->append(QString("Mechanical Energy = %1\n").arg(m_simulation->GetMechanicalEnergy(),
                                    0, 'f', 5));
            m_simulationWindow->updateModel();
            m_simulationWindow->updateCamera();
            QString time = QString("%1").arg(m_simulation->GetTime(), 0, 'f', 5);
            ui->lcdNumberTime->display(time);
            ui->actionRun->setChecked(false);
            run();
            return;
        }
        if (m_simulation->TestForCatastrophy())
        {
            setStatusString(tr("Simulation aborted"), 1);
            ui->textEditLog->append(QString("Fitness = %1\n").arg(m_simulation->CalculateInstantaneousFitness(),
                                    0, 'f', 5));
            m_simulationWindow->updateModel();
            m_simulationWindow->updateCamera();
            QString time = QString("%1").arg(m_simulation->GetTime(), 0, 'f', 5);
            ui->lcdNumberTime->display(time);
            ui->actionRun->setChecked(false);
            run();
            return;
        }
    }
}

void MainWindow::snapshot()
{
    int count = 0;
    QDir dir(m_configFile.absolutePath());
    QStringList list = dir.entryList(QDir::Files | QDir::Dirs, QDir::Name);
    QStringList matches = list.filter(QRegExp(QString("^Snapshot\\d\\d\\d\\d\\d.*")));
    if (matches.size() > 0)
    {
        QString numberString = matches.last().mid(8, 5);
        count = numberString.toInt() + 1;
    }
    QString filename = QString("Snapshot%1").arg(count, 5, 10, QChar('0'));
    if (m_simulationWindow->WriteStillFrame(filename))
    {
        QMessageBox::warning(nullptr, "Snapshot Error",
                             QString("Could write '%1' for OBJ files\n").arg(filename), "Click button to return to simulation");
        return;
    }
    setStatusString(tr("Snapshot taken"), 1);
}

void MainWindow::objSnapshot()
{
    QFileInfo info(Preferences::valueQString("LastFileOpened"));

    QString folder = QFileDialog::getExistingDirectory(this,
                     tr("Choose folder to save current view as OBJ files"), info.absolutePath());

    if (folder.isNull() == false)
    {
        if (m_simulationWindow->WriteCADFrame(folder))
        {
            setStatusString(QString("Error: Folder '%1' write fail\n").arg(folder), 0);
            return;
        }
        setStatusString(QString("Files written in '%1'\n").arg(folder), 1);
    }
}

void MainWindow::buttonCameraRight()
{
    m_simulationWindow->setCameraVecX(0);
    m_simulationWindow->setCameraVecY(1);
    m_simulationWindow->setCameraVecZ(0);
    m_simulationWindow->setUpX(0);
    m_simulationWindow->setUpY(0);
    m_simulationWindow->setUpZ(1);
    m_simulationWindow->updateCamera();
}


void MainWindow::buttonCameraTop()
{
    m_simulationWindow->setCameraVecX(0);
    m_simulationWindow->setCameraVecY(0);
    m_simulationWindow->setCameraVecZ(-1);
    m_simulationWindow->setUpX(0);
    m_simulationWindow->setUpY(1);
    m_simulationWindow->setUpZ(0);
    m_simulationWindow->updateCamera();
}


void MainWindow::buttonCameraFront()
{
    m_simulationWindow->setCameraVecX(-1);
    m_simulationWindow->setCameraVecY(0);
    m_simulationWindow->setCameraVecZ(0);
    m_simulationWindow->setUpX(0);
    m_simulationWindow->setUpY(0);
    m_simulationWindow->setUpZ(1);
    m_simulationWindow->updateCamera();
}


void MainWindow::buttonCameraLeft()
{
    m_simulationWindow->setCameraVecX(0);
    m_simulationWindow->setCameraVecY(-1);
    m_simulationWindow->setCameraVecZ(0);
    m_simulationWindow->setUpX(0);
    m_simulationWindow->setUpY(0);
    m_simulationWindow->setUpZ(1);
    m_simulationWindow->updateCamera();
}


void MainWindow::buttonCameraBottom()
{
    m_simulationWindow->setCameraVecX(0);
    m_simulationWindow->setCameraVecY(0);
    m_simulationWindow->setCameraVecZ(1);
    m_simulationWindow->setUpX(0);
    m_simulationWindow->setUpY(1);
    m_simulationWindow->setUpZ(0);
    m_simulationWindow->updateCamera();
}


void MainWindow::buttonCameraBack()
{
    m_simulationWindow->setCameraVecX(1);
    m_simulationWindow->setCameraVecY(0);
    m_simulationWindow->setCameraVecZ(0);
    m_simulationWindow->setUpX(0);
    m_simulationWindow->setUpY(0);
    m_simulationWindow->setUpZ(1);
    m_simulationWindow->updateCamera();
}


void MainWindow::spinboxDistanceChanged(double v)
{
    Preferences::insert("CameraDistance", v);
    m_simulationWindow->setCameraDistance(v);
    m_simulationWindow->updateCamera();
}


void MainWindow::spinboxFoVChanged(double v)
{
    Preferences::insert("CameraFoV", v);
    m_simulationWindow->setFOV(v);
    m_simulationWindow->updateCamera();
}


void MainWindow::spinboxCOIXChanged(double v)
{
    Preferences::insert("CameraCOIX", v);
    m_simulationWindow->setCOIx(v);
    m_simulationWindow->updateCamera();
}


void MainWindow::spinboxCOIYChanged(double v)
{
    Preferences::insert("CameraCOIY", v);
    m_simulationWindow->setCOIy(v);
    m_simulationWindow->updateCamera();
}


void MainWindow::spinboxCOIZChanged(double v)
{
    Preferences::insert("CameraCOIZ", v);
    m_simulationWindow->setCOIz(v);
    m_simulationWindow->updateCamera();
}

void MainWindow::spinboxNearChanged(double v)
{
    Preferences::insert("CameraFrontClip", v);
    m_simulationWindow->setFrontClip(v);
    m_simulationWindow->updateCamera();
}

void MainWindow::spinboxFarChanged(double v)
{
    Preferences::insert("CameraBackClip", v);
    m_simulationWindow->setBackClip(v);
    m_simulationWindow->updateCamera();
}

void MainWindow::spinboxTrackingOffsetChanged(double v)
{
    Preferences::insert("TrackingOffset", v);
    if (m_simulation)
    {
        if (Preferences::valueBool("TrackingFlag"))
        {
            Body *body = m_simulation->GetBody(Preferences::valueQString("TrackBodyID").toStdString());
            if (body)
            {
                const double *position = dBodyGetPosition(body->GetBodyID());
                ui->doubleSpinBoxCOIX->setValue(position[0] + Preferences::valueDouble("TrackingOffset"));
                ui->doubleSpinBoxCOIX->setValue(position[0] + Preferences::valueDouble("TrackingOffset"));
                m_simulationWindow->updateCamera();
            }
        }
    }
}

void MainWindow::checkboxTracking(int v)
{
    Preferences::insert("TrackingFlag", static_cast<bool>(v));
    if (m_simulation)
    {
        if (Preferences::valueBool("TrackingFlag"))
        {
            Body *body = m_simulation->GetBody(Preferences::valueQString("TrackBodyID").toStdString());
            if (body)
            {
                const double *position = dBodyGetPosition(body->GetBodyID());
                m_simulationWindow->setCOIx(position[0] + Preferences::valueDouble("TrackingOffset"));
                ui->doubleSpinBoxCOIX->setValue(position[0] + Preferences::valueDouble("TrackingOffset"));
                m_simulationWindow->updateCamera();
            }
        }
    }
}

void MainWindow::spinboxCursorSizeChanged(double v)
{
    Preferences::insert("CursorRadius", v);
    m_simulationWindow->setCursorRadius(v);
    m_simulationWindow->updateCamera();
}

void MainWindow::spinboxCursorNudgeChanged(double v)
{
    Preferences::insert("Nudge", v);
    m_simulationWindow->setCursor3DNudge(v);
}

void MainWindow::checkboxContactForce(int v)
{
    Preferences::insert("DisplayContactForces", static_cast<bool>(v));
    if (m_simulation)
    {
//        m_simulation->setDrawContactForces(Preferences::valueBool("DisplayContactForces"));
        m_simulationWindow->updateModel();
        m_simulationWindow->updateCamera();
    }
    m_simulationWindow->updateCamera();
}

void MainWindow::checkboxMuscleForce(int v)
{
    Preferences::insert("DisplayMuscleForces", static_cast<bool>(v));
    if (m_simulation)
    {
        std::map<std::string, Muscle *> *muscleList = m_simulation->GetMuscleList();
        for (std::map<std::string, Muscle *>::const_iterator muscleIter = muscleList->begin();
                muscleIter != muscleList->end(); muscleIter++)
        {
            /*
            muscleIter->second->setDrawMuscleForces(Preferences::valueBool("DisplayMuscleForces"));
            muscleIter->second->GetStrap()->SetLastDrawTime(-1);
            muscleIter->second->Draw(m_simulationWindow);
            */
        }
    }
    m_simulationWindow->updateCamera();
}


void MainWindow::checkboxActivationColours(int v)
{
    Preferences::insert("DisplayActivation", static_cast<bool>(v));
    if (m_simulation)
    {
        std::map<std::string, Muscle *> *muscleList = m_simulation->GetMuscleList();
        std::map<std::string, Muscle *>::const_iterator muscleIter;
        for (muscleIter = muscleList->begin(); muscleIter != muscleList->end(); muscleIter++)
        {
            /*
            muscleIter->second->setActivationDisplay(static_cast<bool>(v));
            muscleIter->second->GetStrap()->SetColour(Preferences::valueQColor("StrapColour"));
            muscleIter->second->GetStrap()->SetForceColour(Colour(Preferences::valueQColor("StrapForceColour")));
            muscleIter->second->GetStrap()->SetLastDrawTime(-1);
            */
        }
    }
    m_simulationWindow->updateCamera();
}


void MainWindow::spinboxSkip(int v)
{
    Preferences::insert("MovieSkip", v);
}


void MainWindow::spinboxTimeMax(double v)
{
    m_simulation->SetTimeLimit(v);
}

void MainWindow::spinboxFPSChanged(double v)
{
    Preferences::insert("MovieFramerate", v);
}

//void MainWindow::listMuscleChecked(QListWidgetItem* item)
//{
//    bool visible = true;
//    if (item->checkState() == Qt::Unchecked) visible = false;
//    // (*m_simulation->GetMuscleList())[std::string(item->text().toUtf8())]->SetAllVisible(visible);
//    // (*m_simulation->GetMuscleList())[std::string(item->text().toUtf8())]->GetStrap()->SetLastDrawTime(-1);
//    // (*m_simulation->GetMuscleList())[std::string(item->text().toUtf8())]->Draw(m_simulationWindow);
//    m_simulationWindow->updateModel();
//    m_simulationWindow->updateCamera();
//}


//void MainWindow::listBodyChecked(QListWidgetItem* item)
//{
//    bool visible = true;
//    if (item->checkState() == Qt::Unchecked) visible = false;
//    // (*m_simulation->GetBodyList())[std::string(item->text().toUtf8())]->SetVisible(visible);
//    // m_simulation->Draw(m_simulationWindow);
//    m_simulationWindow->updateModel();
//    m_simulationWindow->updateCamera();
//}


//void MainWindow::listJointChecked(QListWidgetItem* item)
//{
//    bool visible = true;
//    if (item->checkState() == Qt::Unchecked) visible = false;
////    (*m_simulation->GetJointList())[std::string(item->text().toUtf8())]->SetVisible(visible);
////    m_simulation->Draw(m_simulationWindow);
//    m_simulationWindow->updateModel();
//    m_simulationWindow->updateCamera();
//}


//void MainWindow::listGeomChecked(QListWidgetItem* item)
//{
//   bool visible = true;
//    if (item->checkState() == Qt::Unchecked) visible = false;
////    (*m_simulation->GetGeomList())[std::string(item->text().toUtf8())]->SetVisible(visible);
////    m_simulation->Draw(m_simulationWindow);
//    m_simulationWindow->updateModel();
//    m_simulationWindow->updateCamera();
//}

void MainWindow::menuPreferences()
{
    DialogPreferences dialogPreferences(this);
    dialogPreferences.initialise();

    int status = dialogPreferences.exec();

    if (status == QDialog::Accepted)   // write the new settings
    {
        dialogPreferences.update();
        writeSettings();

        // these settings have immediate effect
        QColor cursorColour = Preferences::valueQColor("CursorColour");
        m_simulationWindow->setCursorColour(QColor(cursorColour.red(), cursorColour.green(),
                                            cursorColour.blue(), cursorColour.alpha()));
        m_simulationWindow->setCursorRadius(Preferences::valueDouble("CursorRadius"));
        m_simulationWindow->setCursor3DNudge(Preferences::valueDouble("Nudge"));
        m_simulationWindow->setFrontClip(Preferences::valueDouble("CameraFrontClip"));
        m_simulationWindow->setBackClip(Preferences::valueDouble("CameraBackClip"));

        m_simulationWindow->updateCamera();

    }

}

void MainWindow::menuOutputs()
{
    if (m_simulation == nullptr) return;

    DialogOutputSelect dialogOutputSelect(this);

    //glWidget->releaseKeyboard();
    int status = dialogOutputSelect.exec();
    //glWidget->grabKeyboard();

    QListWidgetItem *item;
    int i;
    bool dump;

    if (status == QDialog::Accepted)   // write the new settings
    {
        for (i = 0; dialogOutputSelect.listWidgetBody
                && i < dialogOutputSelect.listWidgetBody->count(); i++)
        {
            item = dialogOutputSelect.listWidgetBody->item(i);
            if (item->checkState() == Qt::Unchecked) dump = false;
            else dump = true;
            (*m_simulation->GetBodyList())[std::string(item->text().toUtf8())]->SetDump(dump);
        }

        for (i = 0; dialogOutputSelect.listWidgetMuscle
                && i < dialogOutputSelect.listWidgetMuscle->count(); i++)
        {
            item = dialogOutputSelect.listWidgetMuscle->item(i);
            if (item->checkState() == Qt::Unchecked) dump = false;
            else dump = true;
            (*m_simulation->GetMuscleList())[std::string(item->text().toUtf8())]->SetDump(dump);
            (*m_simulation->GetMuscleList())[std::string(item->text().toUtf8())]->GetStrap()->SetDump(dump);
        }

        for (i = 0; dialogOutputSelect.listWidgetGeom
                && i < dialogOutputSelect.listWidgetGeom->count(); i++)
        {
            item = dialogOutputSelect.listWidgetGeom->item(i);
            if (item->checkState() == Qt::Unchecked) dump = false;
            else dump = true;
            (*m_simulation->GetGeomList())[std::string(item->text().toUtf8())]->SetDump(dump);
        }

        for (i = 0; dialogOutputSelect.listWidgetJoint
                && i < dialogOutputSelect.listWidgetJoint->count(); i++)
        {
            item = dialogOutputSelect.listWidgetJoint->item(i);
            if (item->checkState() == Qt::Unchecked) dump = false;
            else dump = true;
            (*m_simulation->GetJointList())[std::string(item->text().toUtf8())]->SetDump(dump);
        }

        for (i = 0; dialogOutputSelect.listWidgetDriver
                && i < dialogOutputSelect.listWidgetDriver->count(); i++)
        {
            item = dialogOutputSelect.listWidgetDriver->item(i);
            if (item->checkState() == Qt::Unchecked) dump = false;
            else dump = true;
            (*m_simulation->GetDriverList())[std::string(item->text().toUtf8())]->SetDump(dump);
        }

        for (i = 0; dialogOutputSelect.listWidgetDataTarget
                && i < dialogOutputSelect.listWidgetDataTarget->count(); i++)
        {
            item = dialogOutputSelect.listWidgetDataTarget->item(i);
            if (item->checkState() == Qt::Unchecked) dump = false;
            else dump = true;
            (*m_simulation->GetDataTargetList())[std::string(item->text().toUtf8())]->SetDump(dump);
        }

        for (i = 0; dialogOutputSelect.listWidgetReporter
                && i < dialogOutputSelect.listWidgetReporter->count(); i++)
        {
            item = dialogOutputSelect.listWidgetReporter->item(i);
            if (item->checkState() == Qt::Unchecked) dump = false;
            else dump = true;
            (*m_simulation->GetReporterList())[std::string(item->text().toUtf8())]->SetDump(dump);
        }

        for (i = 0; dialogOutputSelect.listWidgetWarehouse
                && i < dialogOutputSelect.listWidgetWarehouse->count(); i++)
        {
            item = dialogOutputSelect.listWidgetWarehouse->item(i);
            if (item->checkState() == Qt::Unchecked) dump = false;
            else dump = true;
            (*m_simulation->GetWarehouseList())[std::string(item->text().toUtf8())]->SetDump(dump);
        }
    }
}

void MainWindow::setInterfaceValues()
{
    m_simulationWindow->setCameraDistance(Preferences::valueDouble("CameraDistance"));
    m_simulationWindow->setFOV(Preferences::valueDouble("CameraFoV"));
    m_simulationWindow->setCameraVecX(Preferences::valueDouble("CameraVecX"));
    m_simulationWindow->setCameraVecY(Preferences::valueDouble("CameraVecY"));
    m_simulationWindow->setCameraVecZ(Preferences::valueDouble("CameraVecZ"));
    m_simulationWindow->setCOIx(Preferences::valueDouble("CameraCOIX"));
    m_simulationWindow->setCOIy(Preferences::valueDouble("CameraCOIY"));
    m_simulationWindow->setCOIz(Preferences::valueDouble("CameraCOIZ"));
    m_simulationWindow->setFrontClip(Preferences::valueDouble("CameraFrontClip"));
    m_simulationWindow->setBackClip(Preferences::valueDouble("CameraBackClip"));
    m_simulationWindow->setUpX(Preferences::valueDouble("CameraUpX"));
    m_simulationWindow->setUpY(Preferences::valueDouble("CameraUpY"));
    m_simulationWindow->setUpZ(Preferences::valueDouble("CameraUpZ"));
    m_simulationWindow->setOrthographicProjection(Preferences::valueBool("OrthographicFlag"));

    QColor cursorColour = Preferences::valueQColor("CursorColour");
    m_simulationWindow->setCursorColour(QColor(cursorColour.red(), cursorColour.green(),
                                        cursorColour.blue(), cursorColour.alpha()));
    m_simulationWindow->setCursorRadius(Preferences::valueDouble("CursorRadius"));
    m_simulationWindow->setCursor3DNudge(Preferences::valueDouble("Nudge"));

    ui->doubleSpinBoxDistance->setValue(Preferences::valueDouble("CameraDistance"));
    ui->doubleSpinBoxFoV->setValue(Preferences::valueDouble("CameraFoV"));
    ui->doubleSpinBoxCOIX->setValue(Preferences::valueDouble("CameraCOIX"));
    ui->doubleSpinBoxCOIY->setValue(Preferences::valueDouble("CameraCOIY"));
    ui->doubleSpinBoxCOIZ->setValue(Preferences::valueDouble("CameraCOIZ"));
    ui->doubleSpinBoxFar->setValue(Preferences::valueDouble("CameraBackClip"));
    ui->doubleSpinBoxNear->setValue(Preferences::valueDouble("CameraFrontClip"));
    ui->doubleSpinBoxTrackingOffset->setValue(Preferences::valueDouble("TrackingOffset"));

    ui->checkBoxTracking->setChecked(Preferences::valueBool("TrackingFlag"));
    ui->checkBoxContactForce->setChecked(Preferences::valueBool("DisplayContactForces"));
    ui->checkBoxMuscleForce->setChecked(Preferences::valueBool("DisplayMuscleForces"));
    ui->checkBoxActivationColours->setChecked(Preferences::valueBool("DisplayActivation"));

    ui->spinBoxSkip->setValue(Preferences::valueInt("MovieSkip"));
    ui->doubleSpinBoxFPS->setValue(Preferences::valueDouble("MovieFramerate"));

    ui->doubleSpinBoxCursorNudge->setValue(Preferences::valueDouble("Nudge"));
    ui->doubleSpinBoxCursorSize->setValue(Preferences::valueDouble("CursorRadius"));

}

void MainWindow::writeSettings()
{
    Preferences::insert("MainWindowGeometry", saveGeometry());
    Preferences::insert("MainWindowState", saveState());
    Preferences::insert("MainWindowSplitter1State", ui->splitter1->saveState());
    Preferences::insert("MainWindowSplitter2State", ui->splitter2->saveState());
    Preferences::insert("ElementTreeHeaderState", ui->treeWidgetElements->header()->saveState());
    Preferences::Write();
}


void MainWindow::menuDefaultView()
{
    ui->doubleSpinBoxTrackingOffset->setValue(Preferences::valueDouble("DefaultTrackingOffset"));

    m_simulationWindow->setCameraDistance(Preferences::valueDouble("DefaultCameraDistance"));
    m_simulationWindow->setFOV(Preferences::valueDouble("DefaultCameraFoV"));
    m_simulationWindow->setCameraVecX(Preferences::valueDouble("DefaultCameraVecX"));
    m_simulationWindow->setCameraVecY(Preferences::valueDouble("DefaultCameraVecY"));
    m_simulationWindow->setCameraVecZ(Preferences::valueDouble("DefaultCameraVecZ"));
    m_simulationWindow->setCOIx(Preferences::valueDouble("DefaultCameraCOIX"));
    m_simulationWindow->setCOIy(Preferences::valueDouble("DefaultCameraCOIY"));
    m_simulationWindow->setCOIz(Preferences::valueDouble("DefaultCameraCOIZ"));
    m_simulationWindow->setUpX(Preferences::valueDouble("DefaultCameraUpX"));
    m_simulationWindow->setUpY(Preferences::valueDouble("DefaultCameraUpY"));
    m_simulationWindow->setUpZ(Preferences::valueDouble("DefaultCameraUpZ"));
    m_simulationWindow->setBackClip(Preferences::valueDouble("DefaultCameraBackClip"));
    m_simulationWindow->setFrontClip(Preferences::valueDouble("DefaultCameraFrontClip"));

    m_simulationWindow->updateCamera();
}

void MainWindow::menuSaveDefaultView()
{
    Preferences::insert("DefaultTrackingOffset", ui->doubleSpinBoxTrackingOffset->value());

    Preferences::insert("DefaultCameraDistance", m_simulationWindow->cameraDistance());
    Preferences::insert("DefaultCameraFoV", m_simulationWindow->FOV());
    Preferences::insert("DefaultCameraCOIX", m_simulationWindow->COIx());
    Preferences::insert("DefaultCameraCOIY", m_simulationWindow->COIy());
    Preferences::insert("DefaultCameraCOIZ", m_simulationWindow->COIz());
    Preferences::insert("DefaultCameraVecX", m_simulationWindow->cameraVecX());
    Preferences::insert("DefaultCameraVecY", m_simulationWindow->cameraVecY());
    Preferences::insert("DefaultCameraVecZ", m_simulationWindow->cameraVecZ());
    Preferences::insert("DefaultCameraUpX", m_simulationWindow->upX());
    Preferences::insert("DefaultCameraUpY", m_simulationWindow->upY());
    Preferences::insert("DefaultCameraUpZ", m_simulationWindow->upZ());
    Preferences::insert("DefaultCameraBackClip", m_simulationWindow->backClip());
    Preferences::insert("DefaultCameraFrontClip", m_simulationWindow->frontClip());
    Preferences::Write();
}

void MainWindow::setStatusString(const QString &s, int logLevel)
{
    statusBar()->showMessage(s);
    if (logLevel <= m_logLevel) log(s);
}

void MainWindow::setUICOI(float x, float y, float z)
{
    ui->doubleSpinBoxCOIX->setValue(double(x));
    ui->doubleSpinBoxCOIY->setValue(double(y));
    ui->doubleSpinBoxCOIZ->setValue(double(z));
}

void MainWindow::setUIFoV(float v)
{
    ui->doubleSpinBoxFoV->setValue(double(v));
}

void MainWindow::menu640x480()
{
    resizeSimulationWindow(640, 480);
}

void MainWindow::menu800x600()
{
    resizeSimulationWindow(800, 600);
}

void MainWindow::menu1280x720()
{
    resizeSimulationWindow(1280, 720);
}

void MainWindow::menu1920x1080()
{
    resizeSimulationWindow(1920, 1080);
}

void MainWindow::resizeSimulationWindow(int w, int h)
{
    int deltaW = w - ui->widgetGLWidget->width();
    int deltaH = h - ui->widgetGLWidget->height();
    resize(width() + deltaW, height() + deltaH);
    m_simulationWindow->updateCamera();
}

SimulationWindow *MainWindow::simulationWindow() const
{
    return m_simulationWindow;
}

Simulation *MainWindow::simulation() const
{
    return m_simulation;
}

void MainWindow::resizeAndCentre(int w, int h)
{
    QDesktopWidget *desktop = qApp->desktop();
    QRect available = desktop->availableGeometry(-1);

    // Need to find how big the central widget is compared to the window
    int heightDiff = height() - ui->widgetGLWidget->height();
    int widthDiff = width() - ui->widgetGLWidget->width();
    int newWidth = w + widthDiff;
    int newHeight = h + heightDiff;

    // centre window
    int topLeftX = available.left() + (available.width() / 2) - (newWidth / 2);
    int topLeftY = available.top() + (available.height() / 2) - (newHeight / 2);
    // but don't start off screen
    if (topLeftX < available.left()) topLeftX = available.left();
    if (topLeftY < available.top()) topLeftY = available.top();

    move(topLeftX, topLeftY);
    resize(newWidth, newHeight);
    m_simulationWindow->updateCamera();
}

void MainWindow::menuStartAVISave()
{
    QFileInfo info(Preferences::valueQString("LastFileOpened"));

    QString fileName = QFileDialog::getSaveFileName(this, tr("Save output as AVI file"),
                       info.absolutePath(), tr("AVI Files (*.avi)"), nullptr);

    if (fileName.isNull() == false)
    {
        m_movieFlag = true;
        m_simulationWindow->StartAVISave(fileName);
    }
}

void MainWindow::menuStopAVISave()
{
    m_movieFlag = false;
    m_simulationWindow->StopAVISave();
}

void MainWindow::menuStartOBJSequenceSave()
{
    QFileInfo info(Preferences::valueQString("LastFileOpened"));

    m_objFileSequenceFolder = QFileDialog::getExistingDirectory(this,
                              tr("Choose folder to the OBJ file sequence"), info.absolutePath());

    if (m_objFileSequenceFolder.isNull() == false)
    {
        m_saveOBJFileSequenceFlag = true;
        ui->actionStart_OBJ_Sequence->setEnabled(false);
        ui->actionStop_OBJ_Sequence->setEnabled(true);
    }
}

void MainWindow::menuStopOBJSequenceSave()
{
    m_saveOBJFileSequenceFlag = false;
    ui->actionStart_OBJ_Sequence->setEnabled(true);
    ui->actionStop_OBJ_Sequence->setEnabled(false);
}

void MainWindow::menuNew()
{
    DialogGlobal dialogGlobal(this);
    Global global;
    dialogGlobal.setGlobal(&global);

    int status = dialogGlobal.exec();

    if (status == QDialog::Accepted)
    {
        if (m_simulation) delete m_simulation;
        m_simulation = nullptr;
        m_stepCount = 0;
        ui->actionRun->setChecked(false);
        m_timer->stop();
        m_simulationWindow->initialiseScene();
        m_simulation = new Simulation();
        m_simulation->SetGlobal(global);
        m_simulation->SetMainWindow(this);
        m_simulationWindow->setSimulation(m_simulation);
        m_simulationWindow->updateModel();
        m_simulationWindow->updateCamera();
        ui->treeWidgetElements->setSimulation(m_simulation);
        Marker *marker = new Marker(nullptr);
        marker->SetName("WorldMarker"s);
        std::map<std::string, Marker *> *markersMap = m_simulation->GetMarkerList();
        (*markersMap)[marker->GetName()] = marker;
        m_noName = true;
        m_saveRequired = false;
        m_mode = constructionMode;
        updateEnable();
        setStatusString(tr("New document created"), 1);
    }
    else
    {
        setStatusString(tr("New document cancelled"), 1);
    }
}

void MainWindow::log(const QString &text)
{
    ui->textEditLog->append(text);
}

void MainWindow::copy()
{
    QWidget *focused = QApplication::focusWidget();
    if ( focused != nullptr )
    {
        QApplication::postEvent( focused, new QKeyEvent( QEvent::KeyPress, Qt::Key_C,
                                 Qt::ControlModifier ));
        QApplication::postEvent( focused, new QKeyEvent( QEvent::KeyRelease, Qt::Key_C,
                                 Qt::ControlModifier ));
    }

}

void MainWindow::cut()
{
    QWidget *focused = QApplication::focusWidget();
    if ( focused != nullptr )
    {
        QApplication::postEvent( focused, new QKeyEvent( QEvent::KeyPress, Qt::Key_X,
                                 Qt::ControlModifier ));
        QApplication::postEvent( focused, new QKeyEvent( QEvent::KeyRelease, Qt::Key_X,
                                 Qt::ControlModifier ));
    }

}

void MainWindow::paste()
{
    QWidget *focused = QApplication::focusWidget();
    if ( focused != nullptr )
    {
        QApplication::postEvent( focused, new QKeyEvent( QEvent::KeyPress, Qt::Key_V,
                                 Qt::ControlModifier ));
        QApplication::postEvent( focused, new QKeyEvent( QEvent::KeyRelease, Qt::Key_V,
                                 Qt::ControlModifier ));
    }

}

void MainWindow::selectAll()
{
    QWidget *focused = QApplication::focusWidget();
    if ( focused != nullptr )
    {
        QApplication::postEvent( focused, new QKeyEvent( QEvent::KeyPress, Qt::Key_A,
                                 Qt::ControlModifier ));
        QApplication::postEvent( focused, new QKeyEvent( QEvent::KeyRelease, Qt::Key_A,
                                 Qt::ControlModifier ));
    }

}

QByteArray MainWindow::readResource(const QString &resource)
{
    QFile file(resource);
    bool ok = file.open(QIODevice::ReadOnly);
    Q_ASSERT(ok);
    return file.readAll();
}

void MainWindow::menuStartWarehouseExport()
{
    if (m_simulation == nullptr) return;

    QFileInfo info(Preferences::valueQString("LastFileOpened"));
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save output as Warehouse file"),
                       info.absolutePath(), tr("Text Files (*.txt)"), nullptr);

    if (fileName.isNull() == false)
    {
        ui->actionStart_Warehouse_Export->setEnabled(false);
        ui->actionStop_Warehouse_Export->setEnabled(true);
        m_simulation->SetOutputWarehouseFile(fileName.toStdString());
    }
}

void MainWindow::menuStopWarehouseExport()
{
    if (m_simulation == nullptr) return;

    ui->actionStart_Warehouse_Export->setEnabled(true);
    ui->actionStop_Warehouse_Export->setEnabled(false);
    m_simulation->SetOutputWarehouseFile(nullptr);
}

void MainWindow::menuImportWarehouse()
{
    if (m_simulation == nullptr) return;
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Warehouse File"), "",
                       tr("Warehouse Files (*.txt)"), nullptr);

    if (fileName.isNull() == false)
    {
        m_simulation->AddWarehouse(fileName.toUtf8());
        setStatusString(QString("Warehouse %1 added").arg(fileName), 1);
    }
}

void MainWindow::menuToggleFullScreen()
{
    setWindowState(windowState() ^ Qt::WindowFullScreen);
}

void MainWindow::menuGeneticAlgorithm()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                       tr("Open Genetic Algorithm Search Control File"), "", tr("Search Control Files (*.txt)"), nullptr);
    if (fileName.isNull() == false)
    {
        DataFile file;
        bool err;
        err = file.ReadFile(fileName.toStdString());
        if (err == false)
        {
            QString currentWD = QDir::currentPath();
            QDir::setCurrent(QFileInfo(fileName).absolutePath());
            do_genetic_algorithm(&file);
            QDir::setCurrent(currentWD);
        }
    }
}

void MainWindow::menuNextAscentHillclimbing()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                       tr("Open Next Ascent Hill Climbing Search Control File"), "", tr("Search Control Files (*.txt)"),
                       nullptr);
    if (fileName.isNull() == false)
    {
        DataFile file;
        bool err;
        err = file.ReadFile(fileName.toStdString());
        if (err == false)
        {
            QString currentWD = QDir::currentPath();
            QDir::setCurrent(QFileInfo(fileName).absolutePath());
            do_next_ascent_hillclimbing(&file);
            QDir::setCurrent(currentWD);
        }
    }
}

void MainWindow::menuRandomAscentHillclimbing()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                       tr("Open Random Ascent Hill Climbing Search Control File"), "", tr("Search Control Files (*.txt)"),
                       nullptr);
    if (fileName.isNull() == false)
    {
        DataFile file;
        bool err;
        err = file.ReadFile(fileName.toStdString());
        if (err == false)
        {
            QString currentWD = QDir::currentPath();
            QDir::setCurrent(QFileInfo(fileName).absolutePath());
            do_random_ascent_hillclimbing(&file);
            QDir::setCurrent(currentWD);
        }
    }
}

void MainWindow::menuSimplexSearch()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Simplex Search Control File"), "",
                       tr("Search Control Files (*.txt)"), nullptr);
    if (fileName.isNull() == false)
    {
        DataFile file;
        bool err;
        err = file.ReadFile(fileName.toStdString());
        if (err == false)
        {
            QString currentWD = QDir::currentPath();
            QDir::setCurrent(QFileInfo(fileName).absolutePath());
            do_simplex_search(&file);
            QDir::setCurrent(currentWD);
        }
    }
}

void MainWindow::menuSimulatedAnnealing()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                       tr("Open Simulated Annealing Search Control File"), "", tr("Search Control Files (*.txt)"),
                       nullptr);
    if (fileName.isNull() == false)
    {
        DataFile file;
        bool err;
        err = file.ReadFile(fileName.toStdString());
        if (err == false)
        {
            QString currentWD = QDir::currentPath();
            QDir::setCurrent(QFileInfo(fileName).absolutePath());
            do_simulated_annealling(&file);
            QDir::setCurrent(currentWD);
        }
    }
}

void MainWindow::menuTabuSearch()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Tabu Search Control File"), "",
                       tr("Search Control Files (*.txt)"), nullptr);
    if (fileName.isNull() == false)
    {
        DataFile file;
        bool err;
        err = file.ReadFile(fileName.toStdString());
        if (err == false)
        {
            QString currentWD = QDir::currentPath();
            QDir::setCurrent(QFileInfo(fileName).absolutePath());
            do_tabu_search(&file);
            QDir::setCurrent(currentWD);
        }
    }
}

void MainWindow::updateEnable()
{
    ui->actionOutput->setEnabled(m_simulation != nullptr);
    ui->action_Restart->setEnabled(m_simulation != nullptr && m_mode == runMode && m_noName == false && m_saveRequired == false);
    ui->action_Save->setEnabled(m_simulation != nullptr && m_noName == false && m_saveRequired == true);
    ui->actionSaveAs->setEnabled(m_simulation != nullptr);
    ui->actionStart_Warehouse_Export->setEnabled(m_simulation != nullptr && m_mode == runMode);
    ui->actionStop_Warehouse_Export->setEnabled(m_simulation != nullptr && m_mode == runMode);
    ui->actionImport_Warehouse->setEnabled(m_simulation != nullptr && m_mode == runMode);
    ui->actionRecord_Movie->setEnabled(m_simulation != nullptr && m_mode == runMode && m_saveRequired == false);
    ui->actionRun->setEnabled(m_simulation != nullptr && m_mode == runMode && m_saveRequired == false);
    ui->actionStep->setEnabled(m_simulation != nullptr && m_mode == runMode && m_saveRequired == false);
    ui->actionSnapshot->setEnabled(m_simulation != nullptr);
    ui->actionSave_OBJ_Snapshot->setEnabled(m_simulation != nullptr && m_mode == runMode && m_saveRequired == false);
    ui->actionStart_OBJ_Sequence->setEnabled(m_simulation != nullptr && m_mode == runMode && m_saveRequired == false);
    ui->actionStop_OBJ_Sequence->setEnabled(m_simulation != nullptr && m_mode == runMode && m_saveRequired == false);
    ui->actionCreateBody->setEnabled(m_simulation != nullptr && m_mode == constructionMode);
    ui->actionCreateMarker->setEnabled(m_simulation != nullptr && m_mode == constructionMode && m_simulation->GetBodyList()->size() > 0);
    ui->actionCreateJoint->setEnabled(m_simulation != nullptr && m_mode == constructionMode && m_simulation->GetBodyList()->size() > 1 && m_simulation->GetMarkerList()->size() > 0);
    ui->actionCreateMuscle->setEnabled(m_simulation != nullptr && m_mode == constructionMode && m_simulation->GetBodyList()->size() > 1 && m_simulation->GetMarkerList()->size() > 0);
    ui->actionCreateGeom->setEnabled(m_simulation != nullptr && m_mode == constructionMode && m_simulation->GetBodyList()->size() > 0 && m_simulation->GetMarkerList()->size() > 0);
    ui->actionCreateDriver->setEnabled(m_simulation != nullptr && m_mode == constructionMode && (m_simulation->GetMuscleList()->size() > 0 || m_simulation->GetControllerList()->size() > 0));
    ui->actionEditGlobal->setEnabled(m_simulation != nullptr && m_mode == constructionMode);
    ui->actionConstructionMode->setEnabled(m_simulation != nullptr && m_mode == runMode && m_stepCount == 0);
    ui->actionRunMode->setEnabled(m_simulation != nullptr && m_mode == constructionMode);
}

void MainWindow::menuCreateBody()
{
    menuCreateEditBody(nullptr);
}

void MainWindow::menuCreateEditBody(Body *body)
{
    Q_ASSERT_X(m_simulation, "MainWindow::menuCreateBody", "m_simulation undefined");
    bool newBody = false;
    DialogBodyBuilder dialogBodyBuilder(this);
    if (!body)
    {
        body = new Body(m_simulation->GetWorldID());
        newBody = true;
        body->setSimulation(m_simulation);
        body->setSize1(Preferences::valueDouble("BodyAxesSize"));
        body->setColour1(Colour(Preferences::valueQColor("BodyColour1").name(
                                    QColor::HexArgb).toStdString()));
        body->setColour2(Colour(Preferences::valueQColor("BodyColour2").name(
                                    QColor::HexArgb).toStdString()));
        body->setColour3(Colour(Preferences::valueQColor("BodyColour3").name(
                                    QColor::HexArgb).toStdString()));
        dialogBodyBuilder.setCreateMode(true);
    }
    dialogBodyBuilder.setSimulation(m_simulation);
    dialogBodyBuilder.setBody(body);
    dialogBodyBuilder.lateInitialise();
    int status = dialogBodyBuilder.exec();
    if (status == QDialog::Accepted)
    {
        std::map<std::string, Body *> *bodiesMap = m_simulation->GetBodyList();
        (*bodiesMap)[body->GetName()] = body;
        this->setStatusString(QString("New body created: %1").arg(QString::fromStdString(body->GetName())),
                              1);
        ui->treeWidgetElements->insertBody(QString().fromStdString(body->GetName()));
        Marker *cmMarker = new Marker(body);
        std::map<std::string, Marker *> *markersMap = m_simulation->GetMarkerList();
        std::map<std::string, Marker *>::iterator it = markersMap->find(cmMarker->GetName());
        if (it != markersMap->end()) delete it->second;
        (*markersMap)[cmMarker->GetName()] = cmMarker;
        ui->treeWidgetElements->insertMarker(QString().fromStdString(cmMarker->GetName()));
        m_saveRequired = true;
        this->updateEnable();
        m_simulationWindow->updateModel();
    }
    else
    {
        if (newBody) delete body;
        this->setStatusString(tr("Body creation cancelled"), 2);
    }
}

void MainWindow::menuCreateMarker()
{
    menuCreateEditMarker(nullptr);
}

void MainWindow::menuCreateEditMarker(Marker *marker)
{
    Q_ASSERT_X(m_simulation, "MainWindow::menuCreateEditMarker", "m_simulation undefined");
    std::map<std::string, Body *> *bodiesMap = m_simulation->GetBodyList();
    Q_ASSERT_X(bodiesMap->size(), "MainWindow::menuCreateEditMarker", "No bodies defined");
    DialogMarkers dialogMarkers(this);
    QVector3D cursor = m_simulationWindow->cursor3DPosition();
    bool newMarker = false;
    if (!marker)
    {
        marker = new Marker(nullptr);
        newMarker = true;
        marker->setSimulation(m_simulation);
        marker->setSize1(Preferences::valueDouble("MarkerRadius"));
        marker->setColour1(Colour(Preferences::valueQColor("MarkerColour").name(
                                      QColor::HexArgb).toStdString()));
        dialogMarkers.setCreateMode(true);
        if (sender() == m_simulationWindow)
        {
            Q_ASSERT_X(m_simulationWindow->getHits().size(), "MainWindow::menuCreateEditMarker",
                       "m_simulationWindow->getHits().size() zero");
            QVector3D worldIntersection = m_simulationWindow->getHits().at(
                                              m_simulationWindow->getClosestHitIndex()).worldIntersection();
            marker->SetPosition(double(worldIntersection[0]), double(worldIntersection[1]),
                                double(worldIntersection[2]));
        }
        else
        {
            marker->SetPosition(double(cursor[0]), double(cursor[1]), double(cursor[2]));
        }
    }
    dialogMarkers.setCursor3DPosition(cursor);
    dialogMarkers.setMarker(marker);
    dialogMarkers.setSimulation(m_simulation);
    dialogMarkers.lateInitialise();
    int status = dialogMarkers.exec();
    if (status == QDialog::Accepted)
    {
        if (dialogMarkers.createMode())
        {
            std::map<std::string, Marker *> *markersMap = m_simulation->GetMarkerList();
            (*markersMap)[marker->GetName()] = marker;
            this->setStatusString(QString("New marker created: \"%1\"").arg(QString::fromStdString(marker->GetName())), 1);
            ui->treeWidgetElements->insertMarker(QString().fromStdString(marker->GetName()));
            this->updateEnable();
        }
        else
        {
            for (auto nb : *marker->dependentList())
            {
                qDebug() << nb->GetName().c_str();
                nb->CreateFromAttributes();
                Strap *strap = dynamic_cast<Strap *>(nb);
                if (strap) strap->Calculate(-1);
            }
        }
        m_saveRequired = true;
        updateEnable();
        m_simulationWindow->updateModel();
    }
    else
    {
        if (newMarker) delete marker;
        this->setStatusString(tr("Marker creation cancelled"), 2);
    }
}

MainWindow::Mode MainWindow::mode() const
{
    return m_mode;
}

void MainWindow::menuCreateJoint()
{
    menuCreateEditJoint(nullptr);
}

void MainWindow::menuCreateEditJoint(Joint *joint)
{
    Q_ASSERT_X(m_simulation, "MainWindow::menuCreateJoint", "m_simulation undefined");
    std::map<std::string, Body *> *bodiesMap = m_simulation->GetBodyList();
    Q_ASSERT_X(bodiesMap->size(), "MainWindow::menuCreateEditMarker", "No bodies defined");
    DialogJoints dialogJoints(this);
    dialogJoints.setSimulation(m_simulation);
    dialogJoints.setJoint(joint);
    dialogJoints.lateInitialise();
    int status = dialogJoints.exec();
    if (status == QDialog::Accepted && dialogJoints.joint())
    {
        std::map<std::string, Joint *> *jointsMap = m_simulation->GetJointList();
        (*jointsMap)[dialogJoints.joint()->GetName()] = dialogJoints.joint();
        if (joint)
        {
            this->setStatusString(QString("Joint edited: %1").arg(QString::fromStdString(dialogJoints.joint()->GetName())), 1);
        }
        else
        {
            this->setStatusString(QString("New joint created: %1").arg(QString::fromStdString(dialogJoints.joint()->GetName())), 1);
            ui->treeWidgetElements->insertJoint(QString().fromStdString(dialogJoints.joint()->GetName()));
        }
        m_saveRequired = true;
        this->updateEnable();
        m_simulationWindow->updateModel();
    }
    else
    {
        this->setStatusString(tr("Joint creation cancelled"), 2);
    }
}

void MainWindow::menuCreateMuscle()
{
    menuCreateEditMuscle(nullptr);
}

void MainWindow::menuCreateEditMuscle(Muscle *muscle)
{
    Q_ASSERT_X(m_simulation, "MainWindow::menuCreateMuscle", "m_simulation undefined");
    std::map<std::string, Body *> *bodiesMap = m_simulation->GetBodyList();
    Q_ASSERT_X(bodiesMap->size(), "MainWindow::menuCreateEditMarker", "No bodies defined");
    DialogMuscles dialogMuscles(this);
    dialogMuscles.setSimulation(m_simulation);
    dialogMuscles.setMuscle(muscle);
    dialogMuscles.lateInitialise();
    int status = dialogMuscles.exec();
    if (status == QDialog::Accepted && dialogMuscles.muscle())
    {
        (*m_simulation->GetMuscleList())[dialogMuscles.muscle()->GetName()] = dialogMuscles.muscle();
        (*m_simulation->GetStrapList())[dialogMuscles.muscle()->GetStrap()->GetName()] = dialogMuscles.muscle()->GetStrap();
        if (muscle)
        {
            this->setStatusString(QString("Muscle edited: %1").arg(QString::fromStdString(dialogMuscles.muscle()->GetName())), 1);
        }
        else
        {
            this->setStatusString(QString("New muscle created: %1").arg(QString::fromStdString(dialogMuscles.muscle()->GetName())), 1);
            ui->treeWidgetElements->insertMuscle(QString().fromStdString(dialogMuscles.muscle()->GetName()));
        }
        m_saveRequired = true;
        this->updateEnable();
        m_simulationWindow->updateModel();
    }
    else
    {
        this->setStatusString(tr("Muscle creation cancelled"), 2);
    }
}

void MainWindow::menuCreateGeom()
{
    menuCreateEditGeom(nullptr);
}

void MainWindow::menuCreateEditGeom(Geom *geom)
{
    Q_ASSERT_X(m_simulation, "MainWindow::menuCreateGeom", "m_simulation undefined");
    std::map<std::string, Body *> *bodiesMap = m_simulation->GetBodyList();
    Q_ASSERT_X(bodiesMap->size(), "MainWindow::menuCreateEditMarker", "No bodies defined");
    DialogGeoms dialogGeoms(this);
    dialogGeoms.setSimulation(m_simulation);
    dialogGeoms.setGeom(geom);
    dialogGeoms.lateInitialise();
    int status = dialogGeoms.exec();
    if (status == QDialog::Accepted && dialogGeoms.geom())
    {
        std::map<std::string, Geom *> *geomsMap = m_simulation->GetGeomList();
        (*geomsMap)[geom->GetName()] = dialogGeoms.geom();
        if (geom)
        {
            this->setStatusString(QString("Geom edited: %1").arg(QString::fromStdString(dialogGeoms.geom()->GetName())), 1);
        }
        else
        {
            this->setStatusString(QString("New geom created: %1").arg(QString::fromStdString(dialogGeoms.geom()->GetName())), 1);
            ui->treeWidgetElements->insertGeom(QString().fromStdString(dialogGeoms.geom()->GetName()));
        }
        m_saveRequired = true;
        this->updateEnable();
        m_simulationWindow->updateModel();
    }
    else
    {
        this->setStatusString(tr("Geom creation cancelled"), 2);
    }
}

void MainWindow::menuCreateDriver()
{
    menuCreateEditDriver(nullptr);
}

void MainWindow::menuCreateEditDriver(Driver *driver)
{
    Q_ASSERT_X(m_simulation, "MainWindow::menuCreateDriver", "m_simulation undefined");
    std::map<std::string, Body *> *bodiesMap = m_simulation->GetBodyList();
    Q_ASSERT_X(bodiesMap->size(), "MainWindow::menuCreateEditMarker", "No bodies defined");
    DialogDrivers dialogDrivers(this);
    dialogDrivers.setSimulation(m_simulation);
    dialogDrivers.setDriver(driver);
    dialogDrivers.lateInitialise();
    int status = dialogDrivers.exec();
    if (status == QDialog::Accepted && dialogDrivers.driver())
    {
        std::map<std::string, Driver *> *driversMap = m_simulation->GetDriverList();
        (*driversMap)[driver->GetName()] = dialogDrivers.driver();
        if (driver)
        {
            this->setStatusString(QString("Driver edited: %1").arg(QString::fromStdString(dialogDrivers.driver()->GetName())), 1);
        }
        else
        {
            this->setStatusString(QString("New driver created: %1").arg(QString::fromStdString(dialogDrivers.driver()->GetName())), 1);
            ui->treeWidgetElements->insertDriver(QString().fromStdString(dialogDrivers.driver()->GetName()));
        }
        m_saveRequired = true;
        this->updateEnable();
        m_simulationWindow->updateModel();
    }
    else
    {
        this->setStatusString(tr("Driver creation cancelled"), 2);
    }
}

void MainWindow::deleteExistingBody(const QString &name)
{
    int ret = QMessageBox::warning(this, tr("Delete Body %1").arg(name),
                                   tr("This action cannot be undone.\n"
                                      "Are you sure you want to continue?"),
                                   QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel);
    if (ret == QMessageBox::Ok)
    {
        // first delete dependencies
        std::string nameStr = name.toStdString();
        // we need to explicitly delete the joints
        for (auto it = m_simulation->GetJointList()->begin(); it != m_simulation->GetJointList()->end();)
        {
            auto oldIt = it;
            size_t oldSize = m_simulation->GetJointList()->size();
            if (it->second->GetBody1()->GetName() == nameStr
                    || it->second->GetBody2()->GetName() == nameStr) deleteExistingJoint(QString::fromStdString(
                                    it->second->GetName()));
            if (m_simulation->GetJointList()->size() == oldSize)
                return; // nothing was deleted so the user cancelled
            it = ++oldIt;
        }
        // and deleting markers will clear up everything else
        for (auto it = m_simulation->GetMarkerList()->begin(); it != m_simulation->GetMarkerList()->end();
            )
        {
            auto oldIt = it;
            size_t oldSize = m_simulation->GetMarkerList()->size();
            if (it->second->GetBody()->GetName() == nameStr) deleteExistingMarker(QString::fromStdString(
                            it->second->GetName()));
            if (m_simulation->GetMarkerList()->size() == oldSize)
                return; // nothing was deleted so the user cancelled
            it = ++oldIt;
        }
        // now delete the body itself
        auto it = m_simulation->GetBodyList()->find(nameStr);
        if (it != m_simulation->GetBodyList()->end())
        {
            delete (it->second);
            m_simulation->GetBodyList()->erase(it);
            ui->treeWidgetElements->removeBody(name);
        }
        m_saveRequired = true;
        updateEnable();
        m_simulationWindow->updateModel();
    }
}

void MainWindow::deleteExistingMarker(const QString &name)
{
    int ret = QMessageBox::warning(this, tr("Delete Marker %1").arg(name),
                                   tr("This action cannot be undone.\n"
                                      "Are you sure you want to continue?"),
                                   QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel);
    if (ret == QMessageBox::Ok)
    {
        // first delete dependencies (lots of things can depend on markers)
        std::string nameStr = name.toStdString();
        Marker *marker = m_simulation->GetMarkerList()->at(nameStr);
        // we need to explicitly delete the joints
        for (auto it = m_simulation->GetJointList()->begin(); it != m_simulation->GetJointList()->end();)
        {
            auto oldIt = it;
            size_t oldSize = m_simulation->GetJointList()->size();
            if (it->second->body1Marker() == marker
                    || it->second->body2Marker() == marker) deleteExistingJoint(QString::fromStdString(
                                    it->second->GetName()));
            if (m_simulation->GetJointList()->size() == oldSize)
                return; // nothing was deleted so the user cancelled
            it = ++oldIt;
        }
        // we need to explicitly delete the muscles
        for (auto it = m_simulation->GetMuscleList()->begin(); it != m_simulation->GetMuscleList()->end();
            )
        {
            auto oldIt = it;
            size_t oldSize = m_simulation->GetMuscleList()->size();
            Strap *strap = it->second->GetStrap();
            strap->updateDependentMarkers();
            auto it2 = std::find(strap->dependentMarkers()->begin(), strap->dependentMarkers()->end(), marker);
            if (it2 != strap->dependentMarkers()->end())
            {
                deleteExistingMuscle(QString::fromStdString(it->second->GetName()));
                if (m_simulation->GetMuscleList()->size() == oldSize)
                    return; // nothing was deleted so the user cancelled
            }
            it = ++oldIt;
        }
        // we need to explicitly delete the geoms
        for (auto it = m_simulation->GetGeomList()->begin(); it != m_simulation->GetGeomList()->end();)
        {
            auto oldIt = it;
            size_t oldSize = m_simulation->GetGeomList()->size();
            if (it->second->geomMarker() == marker ) deleteExistingGeom(QString::fromStdString(
                            it->second->GetName()));
            if (m_simulation->GetGeomList()->size() == oldSize)
                return; // nothing was deleted so the user cancelled
            it = ++oldIt;
        }
        // now delete the marker itself
        auto it = m_simulation->GetMarkerList()->find(nameStr);
        if (it != m_simulation->GetMarkerList()->end())
        {
            delete (it->second);
            m_simulation->GetMarkerList()->erase(it);
            ui->treeWidgetElements->removeMarker(name);
        }
        m_saveRequired = true;
        updateEnable();
        m_simulationWindow->updateModel();
    }

}

void MainWindow::deleteExistingJoint(const QString &name)
{
    int ret = QMessageBox::warning(this, tr("Delete Joint %1").arg(name),
                                   tr("This action cannot be undone.\n"
                                      "Are you sure you want to continue?"),
                                   QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel);
    if (ret == QMessageBox::Ok)
    {
        std::string nameStr = name.toStdString();
        auto it = m_simulation->GetJointList()->find(nameStr);
        if (it != m_simulation->GetJointList()->end())
        {
            delete (it->second);
            m_simulation->GetJointList()->erase(it);
            ui->treeWidgetElements->removeJoint(name);
        }
        m_saveRequired = true;
        updateEnable();
        m_simulationWindow->updateModel();
    }
}

void MainWindow::deleteExistingMuscle(const QString &name)
{
    int ret = QMessageBox::warning(this, tr("Delete Muscle %1").arg(name),
                                   tr("This action cannot be undone.\n"
                                      "Are you sure you want to continue?"),
                                   QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel);
    if (ret == QMessageBox::Ok)
    {
        std::string nameStr = name.toStdString();
        auto it = m_simulation->GetMuscleList()->find(nameStr);
        if (it != m_simulation->GetMuscleList()->end())
        {
            delete (it->second);
            m_simulation->GetMuscleList()->erase(it);
            ui->treeWidgetElements->removeMuscle(name);
        }
        m_saveRequired = true;
        updateEnable();
        m_simulationWindow->updateModel();
    }
}

void MainWindow::deleteExistingDriver(const QString &name)
{
    int ret = QMessageBox::warning(this, tr("Delete Driver %1").arg(name),
                                   tr("This action cannot be undone.\n"
                                      "Are you sure you want to continue?"),
                                   QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel);
    if (ret == QMessageBox::Ok)
    {
        std::string nameStr = name.toStdString();
        auto it = m_simulation->GetDriverList()->find(nameStr);
        if (it != m_simulation->GetDriverList()->end())
        {
            delete (it->second);
            m_simulation->GetDriverList()->erase(it);
            ui->treeWidgetElements->removeDriver(name);
        }
        m_saveRequired = true;
        updateEnable();
        m_simulationWindow->updateModel();
    }
}

void MainWindow::deleteExistingGeom(const QString &name)
{
    int ret = QMessageBox::warning(this, tr("Delete Geom %1").arg(name),
                                   tr("This action cannot be undone.\n"
                                      "Are you sure you want to continue?"),
                                   QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel);
    if (ret == QMessageBox::Ok)
    {
        std::string nameStr = name.toStdString();
        auto it = m_simulation->GetGeomList()->find(nameStr);
        if (it != m_simulation->GetGeomList()->end())
        {
            delete (it->second);
            m_simulation->GetGeomList()->erase(it);
            ui->treeWidgetElements->removeGeom(name);
        }
        m_saveRequired = true;
        updateEnable();
        m_simulationWindow->updateModel();
    }
}

void MainWindow::editExistingBody(const QString &name)
{
    Body *body = m_simulation->GetBody(name.toStdString());
    menuCreateEditBody(body);
}

void MainWindow::editExistingMarker(const QString &name)
{
    Marker *marker = m_simulation->GetMarker(name.toStdString());
    menuCreateEditMarker(marker);
}

void MainWindow::editExistingJoint(const QString &name)
{
    Joint *joint = m_simulation->GetJoint(name.toStdString());
    menuCreateEditJoint(joint);
}

void MainWindow::editExistingMuscle(const QString &name)
{
    Muscle *muscle = m_simulation->GetMuscle(name.toStdString());
    menuCreateEditMuscle(muscle);
}

void MainWindow::editExistingGeom(const QString &name)
{
    Geom *geom = m_simulation->GetGeom(name.toStdString());
    menuCreateEditGeom(geom);
}

void MainWindow::editExistingDriver(const QString &name)
{
    Driver *driver = m_simulation->GetDriver(name.toStdString());
    menuCreateEditDriver(driver);
}

void MainWindow::menuEditGlobal()
{
    Q_ASSERT_X(m_simulation, "MainWindow::menuEditGlobal", "m_simulation undefined");
    DialogGlobal dialogGlobal(this);
    Global global = m_simulation->GetGlobal();
    dialogGlobal.setGlobal(&global);

    int status = dialogGlobal.exec();

    if (status == QDialog::Accepted)   // write the new settings
    {
        m_simulation->SetGlobal(global);
        m_simulation->SetMainWindow(this);
        setStatusString(tr("Global values edited"), 1);
        m_saveRequired = true;
        updateEnable();
        m_simulationWindow->updateModel();
    }
    else
    {
        setStatusString(tr("Global values unchanged"), 2);
    }
}


void MainWindow::comboBoxCurrentTextChanged(const QString &text)
{
    if (text == QString("Mesh 1")) m_simulationWindow->setBodyMeshNumber(1);
    if (text == QString("Mesh 2")) m_simulationWindow->setBodyMeshNumber(2);
    if (text == QString("Mesh 3")) m_simulationWindow->setBodyMeshNumber(3);
    m_simulationWindow->updateModel();
}

void MainWindow::handleElementTreeWidgetItemChanged(QTreeWidgetItem * /* item */, int column)
{
    if (column == 1) m_simulationWindow->updateModel();
}

void MainWindow::enterRunMode()
{
    if (!m_simulation) return;
    m_mode = runMode;
    for (auto it : *m_simulation->GetBodyList()) it.second->EnterRunMode();
    for (auto it : *m_simulation->GetMuscleList()) it.second->LateInitialisation();
    updateEnable();
    m_simulationWindow->updateModel();
}

void MainWindow::enterConstructionMode()
{
    if (!m_simulation) return;
    m_mode = constructionMode;
    for (auto it : *m_simulation->GetBodyList()) it.second->EnterConstructionMode();
    for (auto it : *m_simulation->GetMuscleList()) it.second->LateInitialisation();
    updateEnable();
    m_simulationWindow->updateModel();
}

void MainWindow::moveExistingMarker(const QString &s, const QVector3D &p)
{
    auto it = m_simulation->GetMarkerList()->find(s.toStdString());
    if (it == m_simulation->GetMarkerList()->end()) return;
    it->second->SetWorldPosition(double(p.x()), double(p.y()), double(p.z()));
    for (auto nb : *it->second->dependentList())
    {
        qDebug() << nb->GetName().c_str();
        nb->CreateFromAttributes();
        Strap *strap = dynamic_cast<Strap *>(nb);
        if (strap) strap->Calculate(-1);
    }
    m_saveRequired = true;
    updateEnable();
    m_simulationWindow->updateModel();
}
