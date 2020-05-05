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
#include "Controller.h"
#include "Warehouse.h"
#include "Preferences.h"
#include "SimulationWidget.h"
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
#include "DialogAssembly.h"
#include "DialogMarkerImportExport.h"
#include "Colour.h"
#include "AMotorJoint.h"
#include "LMotorJoint.h"
#include "TegotaeDriver.h"

#include <QMessageBox>
#include <QTimer>
#include <QFileDialog>
#include <QBoxLayout>
#include <QDesktopWidget>
#include <QScreen>
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
    // create the window elements
    ui->setupUi(this);

    // interface related connections
    connect(ui->action1280x720, SIGNAL(triggered()), this, SLOT(menu1280x720()));
    connect(ui->action1920x1080, SIGNAL(triggered()), this, SLOT(menu1920x1080()));
    connect(ui->action640x480, SIGNAL(triggered()), this, SLOT(menu640x480()));
    connect(ui->action800x600, SIGNAL(triggered()), this, SLOT(menu800x600()));
    connect(ui->actionAboutGaitSymQt, SIGNAL(triggered()), this, SLOT(menuAbout()));
    connect(ui->actionConstructionMode, SIGNAL(triggered()), this, SLOT(enterConstructionMode()));
    connect(ui->actionCopy, SIGNAL(triggered()), this, SLOT(copy()));
    connect(ui->actionCreateAssembly, SIGNAL(triggered()), this, SLOT(menuCreateAssembly()));
    connect(ui->actionCreateBody, SIGNAL(triggered()), this, SLOT(menuCreateBody()));
    connect(ui->actionCreateDriver, SIGNAL(triggered()), this, SLOT(menuCreateDriver()));
    connect(ui->actionCreateGeom, SIGNAL(triggered()), this, SLOT(menuCreateGeom()));
    connect(ui->actionCreateJoint, SIGNAL(triggered()), this, SLOT(menuCreateJoint()));
    connect(ui->actionCreateMarker, SIGNAL(triggered()), this, SLOT(menuCreateMarker()));
    connect(ui->actionCreateMuscle, SIGNAL(triggered()), this, SLOT(menuCreateMuscle()));
    connect(ui->actionCut, SIGNAL(triggered()), this, SLOT(cut()));
    connect(ui->actionDefaultView, SIGNAL(triggered()), this, SLOT(menuDefaultView()));
    connect(ui->actionDeleteAssembly, SIGNAL(triggered()), this, SLOT(menuDeleteAssembly()));
    connect(ui->actionEditGlobal, SIGNAL(triggered()), this, SLOT(menuEditGlobal()));
    connect(ui->actionExportMarkers, SIGNAL(triggered()), this, SLOT(menuExportMarkers()));
    connect(ui->actionGeneticAlgorithm, SIGNAL(triggered()), this, SLOT(menuGeneticAlgorithm()));
    connect(ui->actionImportWarehouse, SIGNAL(triggered()), this, SLOT(menuImportWarehouse()));
    connect(ui->actionNew, SIGNAL(triggered()), this, SLOT(menuNew()));
    connect(ui->actionNextAscentHillclimbing, SIGNAL(triggered()), this, SLOT(menuNextAscentHillclimbing()));
    connect(ui->actionOpen, SIGNAL(triggered()), this, SLOT(menuOpen()));
    connect(ui->actionOutput, SIGNAL(triggered()), this, SLOT(menuOutputs()));
    connect(ui->actionPaste, SIGNAL(triggered()), this, SLOT(paste()));
    connect(ui->actionPreferences, SIGNAL(triggered()), this, SLOT(menuPreferences()));
    connect(ui->actionQuit, SIGNAL(triggered()), this, SLOT(close()));
    connect(ui->actionRandomAscentHillclimbing, SIGNAL(triggered()), this, SLOT(menuRandomAscentHillclimbing()));
    connect(ui->actionRecordMovie, SIGNAL(triggered()), this, SLOT(menuRecordMovie()));
    connect(ui->actionResetView, SIGNAL(triggered()), this, SLOT(menuResetView()));
    connect(ui->actionRestart, SIGNAL(triggered()), this, SLOT(menuRestart()));
    connect(ui->actionRun, SIGNAL(triggered()), this, SLOT(run()));
    connect(ui->actionRunMode, SIGNAL(triggered()), this, SLOT(enterRunMode()));
    connect(ui->actionSave, SIGNAL(triggered()), this, SLOT(menuSave()));
    connect(ui->actionSaveAs, SIGNAL(triggered()), this, SLOT(menuSaveAs()));
    connect(ui->actionSaveDefaultView, SIGNAL(triggered()), this, SLOT(menuSaveDefaultView()));
    connect(ui->actionSaveOBJSnapshot, SIGNAL(triggered()), this, SLOT(objSnapshot()));
    connect(ui->actionSelectAll, SIGNAL(triggered()), this, SLOT(selectAll()));
    connect(ui->actionSimplexSearch, SIGNAL(triggered()), this, SLOT(menuSimplexSearch()));
    connect(ui->actionSimulatedAnnealing, SIGNAL(triggered()), this, SLOT(menuSimulatedAnnealing()));
    connect(ui->actionSnapshot, SIGNAL(triggered()), this, SLOT(snapshot()));
    connect(ui->actionStartOBJSequence, SIGNAL(triggered()), this, SLOT(menuStartOBJSequenceSave()));
    connect(ui->actionStartWarehouseExport, SIGNAL(triggered()), this, SLOT(menuStartWarehouseExport()));
    connect(ui->actionStep, SIGNAL(triggered()), this, SLOT(step()));
    connect(ui->actionStopOBJSequence, SIGNAL(triggered()), this, SLOT(menuStopOBJSequenceSave()));
    connect(ui->actionStopWarehouseExport, SIGNAL(triggered()), this, SLOT(menuStopWarehouseExport()));
    connect(ui->actionTabuSearch, SIGNAL(triggered()), this, SLOT(menuTabuSearch()));
    connect(ui->actionToggleFullscreen, SIGNAL(triggered()), this, SLOT(menuToggleFullScreen()));
    connect(ui->actionViewBack, SIGNAL(triggered()), this, SLOT(buttonCameraBack()));
    connect(ui->actionViewBottom, SIGNAL(triggered()), this, SLOT(buttonCameraBottom()));
    connect(ui->actionViewFront, SIGNAL(triggered()), this, SLOT(buttonCameraFront()));
    connect(ui->actionViewLeft, SIGNAL(triggered()), this, SLOT(buttonCameraLeft()));
    connect(ui->actionViewRight, SIGNAL(triggered()), this, SLOT(buttonCameraRight()));
    connect(ui->actionViewTop, SIGNAL(triggered()), this, SLOT(buttonCameraTop()));
    connect(ui->radioButtonTrackingNone, SIGNAL(stateChanged(int)), this, SLOT(radioButtonTrackingNone(int)));
    connect(ui->radioButtonTrackingX, SIGNAL(stateChanged(int)), this, SLOT(radioButtonTrackingX(int)));
    connect(ui->radioButtonTrackingY, SIGNAL(stateChanged(int)), this, SLOT(radioButtonTrackingY(int)));
    connect(ui->radioButtonTrackingZ, SIGNAL(stateChanged(int)), this, SLOT(radioButtonTrackingZ(int)));
    connect(ui->comboBoxMeshDisplay, SIGNAL(currentTextChanged(const QString &)), this, SLOT(comboBoxMeshDisplayMapCurrentTextChanged(const QString &)));
    connect(ui->comboBoxMuscleColourMap, SIGNAL(currentTextChanged(const QString &)), this, SLOT(comboBoxMuscleColourMapCurrentTextChanged(const QString &)));
    connect(ui->comboBoxTrackingBody, SIGNAL(currentTextChanged(const QString &)), this, SLOT(comboBoxTrackingBodyCurrentTextChanged(const QString &)));
    connect(ui->doubleSpinBoxCOIX, SIGNAL(valueChanged(double)), this, SLOT(spinboxCOIXChanged(double)));
    connect(ui->doubleSpinBoxCOIY, SIGNAL(valueChanged(double)), this, SLOT(spinboxCOIYChanged(double)));
    connect(ui->doubleSpinBoxCOIZ, SIGNAL(valueChanged(double)), this, SLOT(spinboxCOIZChanged(double)));
    connect(ui->doubleSpinBoxCursorNudge, SIGNAL(valueChanged(double)), this, SLOT(spinboxCursorNudgeChanged(double)));
    connect(ui->doubleSpinBoxCursorSize, SIGNAL(valueChanged(double)), this, SLOT(spinboxCursorSizeChanged(double)));
    connect(ui->doubleSpinBoxDistance, SIGNAL(valueChanged(double)), this, SLOT(spinboxDistanceChanged(double)));
    connect(ui->doubleSpinBoxFPS, SIGNAL(valueChanged(double)), this, SLOT(spinboxFPSChanged(double)));
    connect(ui->doubleSpinBoxFar, SIGNAL(valueChanged(double)), this, SLOT(spinboxFarChanged(double)));
    connect(ui->doubleSpinBoxFoV, SIGNAL(valueChanged(double)), this, SLOT(spinboxFoVChanged(double)));
    connect(ui->doubleSpinBoxNear, SIGNAL(valueChanged(double)), this, SLOT(spinboxNearChanged(double)));
    connect(ui->doubleSpinBoxTimeMax, SIGNAL(valueChanged(double)), this, SLOT(spinboxTimeMax(double)));
    connect(ui->doubleSpinBoxTrackingOffset, SIGNAL(valueChanged(double)), this, SLOT(spinboxTrackingOffsetChanged(double)));
    connect(ui->spinBoxSkip, SIGNAL(valueChanged(int)), this, SLOT(spinboxSkip(int)));
    connect(ui->treeWidgetElements, SIGNAL(createNewBody()), this, SLOT(menuCreateBody()));
    connect(ui->treeWidgetElements, SIGNAL(createNewDriver()), this, SLOT(menuCreateDriver()));
    connect(ui->treeWidgetElements, SIGNAL(createNewGeom()), this, SLOT(menuCreateGeom()));
    connect(ui->treeWidgetElements, SIGNAL(createNewJoint()), this, SLOT(menuCreateJoint()));
    connect(ui->treeWidgetElements, SIGNAL(createNewMarker()), this, SLOT(menuCreateMarker()));
    connect(ui->treeWidgetElements, SIGNAL(createNewMuscle()), this, SLOT(menuCreateMuscle()));
    connect(ui->treeWidgetElements, SIGNAL(deleteBody(const QString &)), this, SLOT(deleteExistingBody(const QString &)));
    connect(ui->treeWidgetElements, SIGNAL(deleteDriver(const QString &)), this, SLOT(deleteExistingDriver(const QString &)));
    connect(ui->treeWidgetElements, SIGNAL(deleteGeom(const QString &)), this, SLOT(deleteExistingGeom(const QString &)));
    connect(ui->treeWidgetElements, SIGNAL(deleteJoint(const QString &)), this, SLOT(deleteExistingJoint(const QString &)));
    connect(ui->treeWidgetElements, SIGNAL(deleteMarker(const QString &)), this, SLOT(deleteExistingMarker(const QString &)));
    connect(ui->treeWidgetElements, SIGNAL(deleteMuscle(const QString &)), this, SLOT(deleteExistingMuscle(const QString &)));
    connect(ui->treeWidgetElements, SIGNAL(editBody(const QString &)), this, SLOT(editExistingBody(const QString &)));
    connect(ui->treeWidgetElements, SIGNAL(editDriver(const QString &)), this, SLOT(editExistingDriver(const QString &)));
    connect(ui->treeWidgetElements, SIGNAL(editGeom(const QString &)), this, SLOT(editExistingGeom(const QString &)));
    connect(ui->treeWidgetElements, SIGNAL(editJoint(const QString &)), this, SLOT(editExistingJoint(const QString &)));
    connect(ui->treeWidgetElements, SIGNAL(editMarker(const QString &)), this, SLOT(editExistingMarker(const QString &)));
    connect(ui->treeWidgetElements, SIGNAL(editMuscle(const QString &)), this, SLOT(editExistingMuscle(const QString &)));
    connect(ui->treeWidgetElements, SIGNAL(elementTreeWidgetItemChanged(QTreeWidgetItem *, int)), this, SLOT(handleElementTreeWidgetItemChanged(QTreeWidgetItem *, int)));

    // put SimulationWindow into existing widgetGLWidget
//    QBoxLayout *boxLayout = new QBoxLayout(QBoxLayout::LeftToRight, ui->widgetGLWidget);
//    boxLayout->setMargin(0);
//    ui->widgetSimulation = new SimulationWidget();
//    boxLayout->addWidget(ui->widgetSimulation);
//    ui->widgetSimulation->setMainWindow(this);
    ui->widgetSimulation->setMainWindow(this);

    // connect the ViewControlWidget to the GLWidget
    QObject::connect(ui->widgetViewFrame, SIGNAL(EmitCameraVec(double, double, double)), ui->widgetSimulation, SLOT(SetCameraVec(double, double, double)));

    // connect the SimulationWindow to the MainWindow
    QObject::connect(ui->widgetSimulation, SIGNAL(EmitStatusString(const QString &, int)), this, SLOT(setStatusString(const QString &, int)));
    QObject::connect(ui->widgetSimulation, SIGNAL(EmitCOI(float, float, float)), this, SLOT(setUICOI(float, float, float)));
    QObject::connect(ui->widgetSimulation, SIGNAL(EmitFoV(float)), this, SLOT(setUIFoV(float)));
    QObject::connect(ui->widgetSimulation, SIGNAL(EmitCreateMarkerRequest()), this, SLOT(menuCreateMarker()));
    QObject::connect(ui->widgetSimulation, SIGNAL(EmitEditMarkerRequest(const QString &)), this, SLOT(editExistingMarker(const QString &)));
    QObject::connect(ui->widgetSimulation, SIGNAL(EmitMoveMarkerRequest(const QString &, const QVector3D &)), this, SLOT(moveExistingMarker(const QString &, const QVector3D &)));
    QObject::connect(ui->widgetSimulation, SIGNAL(EmitEditBodyRequest(const QString &)), this, SLOT(editExistingBody(const QString &)));
    QObject::connect(ui->widgetSimulation, SIGNAL(EmitEditGeomRequest(const QString &)), this, SLOT(editExistingGeom(const QString &)));
    QObject::connect(ui->widgetSimulation, SIGNAL(EmitEditJointRequest(const QString &)), this, SLOT(editExistingJoint(const QString &)));
    QObject::connect(ui->widgetSimulation, SIGNAL(EmitEditMuscleRequest(const QString &)), this, SLOT(editExistingMuscle(const QString &)));

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
    ui->actionStartWarehouseExport->setVisible(false);
    ui->actionStopWarehouseExport->setVisible(false);
    ui->actionImportWarehouse->setVisible(false);
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
        msgBox.setInformativeText("Click OK to quit, and Cancel to continue working on the document");
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

void MainWindow::menuOpen()
{
    if (m_saveRequired)
    {
        int ret = QMessageBox::warning(this, tr("Current document contains unsaved changes"), tr("Opening a new document will delete the current document.\nAre you sure you want to continue?"),
                                       QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel);
        if (ret == QMessageBox::Cancel) return;
    }

    QFileInfo info = Preferences::valueQString("LastFileOpened");
    QString fileName;

    fileName = QFileDialog::getOpenFileName(this, tr("Open Config File"), info.absoluteFilePath(), tr("Config Files (*.xml);;Any File (*.* *)"), nullptr);
    if (fileName.isNull() == false)
    {
        Preferences::insert("LastFileOpened", fileName);
        menuOpen(fileName);
    }
}

void MainWindow::menuOpen(const QString &fileName)
{
    // dispose any simulation cleanly
    m_timer->stop();
    ui->actionRun->setChecked(false);
    m_movieFlag = false;
    if (ui->widgetSimulation->aviWriter()) menuStopAVISave();
    if (m_simulation)
    {
        delete m_simulation;
        m_simulation = nullptr;
        ui->widgetSimulation->setSimulation(m_simulation);
    }
    m_stepCount = 0;
    m_stepFlag = false;

    m_configFile.setFile(fileName);
    QDir::setCurrent(m_configFile.absolutePath());
    QString canonicalFilePath = m_configFile.canonicalFilePath();
    QDir currentDir(m_configFile.absolutePath());
    Preferences::insert("LastFileOpened", canonicalFilePath);

    setStatusString(fileName + QString(" loading"), 2);
    qApp->processEvents();

    DataFile file;
    int err;
    err = file.ReadFile(canonicalFilePath.toStdString());
    if (err)
    {
        setStatusString(QString("Error reading ") + canonicalFilePath, 0);
        return;
    }
    m_simulation = new Simulation();
    m_simulation->SetMainWindow(this);

    std::string *errorMessage = m_simulation->LoadModel(file.GetRawData(), file.GetSize());
    if (errorMessage)
    {
        setStatusString(QString::fromStdString(*errorMessage), 0);
        delete m_simulation;
        m_simulation = nullptr;
        ui->widgetSimulation->setSimulation(m_simulation);
        ui->widgetSimulation->update();
        return;
    }
    ui->treeWidgetElements->fillVisibitilityLists(m_simulation);


    // check we can find the meshes
    QStringList searchPath = QString::fromStdString(m_simulation->GetGlobal()->MeshSearchPath()).split(':');
    bool noToAll = false;
    bool meshPathChanged = false;
    for (auto &&iter : *m_simulation->GetBodyList())
    {
        std::vector<std::string> meshNames = {iter.second->GetGraphicFile1(), iter.second->GetGraphicFile2(), iter.second->GetGraphicFile3()};
        for (auto meshName : meshNames)
        {
            if (meshName.size() == 0) continue;
            bool fileFound = false;
            QString graphicsFile = QString::fromStdString(meshName);
            for (int i = 0; i < searchPath.size(); i++)
            {
                QDir dir(searchPath[i]);
                if (dir.exists(graphicsFile))
                {
                    fileFound = true;
                    break;
                }
            }
            if (fileFound == false)
            {
                int ret = QMessageBox::warning(this, QString("Loading ") + canonicalFilePath,
                                               QString("Unable to find \"") + graphicsFile + QString("\"\nDo you want to load a new file?"),
                                               QMessageBox::Yes | QMessageBox::No | QMessageBox::NoToAll, QMessageBox::Yes);
                if (ret == QMessageBox::NoToAll) noToAll = true;
                if (ret == QMessageBox::Yes)
                {
                    QString newFile = QFileDialog::getOpenFileName(this, tr("Open Mesh File"), graphicsFile, tr("Mesh Files (*.obj *.ply)"), nullptr);
                    if (newFile.isNull() == false)
                    {
                        QFileInfo newFileInfo(newFile);
                        iter.second->SetGraphicFile1(newFileInfo.fileName().toStdString());
                        QString newPath = currentDir.relativeFilePath(newFileInfo.absolutePath());
                        searchPath.append(newPath);
                        meshPathChanged = true;
                    }
                }
            }
            if (noToAll) break;
        }
        if (noToAll) break;
    }
    if (meshPathChanged) m_simulation->GetGlobal()->setMeshSearchPath(searchPath.join(":").toStdString());

    ui->widgetSimulation->setAxesScale(float(m_simulation->GetGlobal()->size1()));
    QString backgroundColour = QString::fromStdString(m_simulation->GetGlobal()->colour1().GetHexArgb());
    ui->widgetSimulation->setSimulation(m_simulation);
    ui->widgetSimulation->setBackgroundColour(QColor(backgroundColour));
    ui->widgetSimulation->update();
    // m_simulation->setDrawContactForces(Preferences::valueBool("DisplayContactForces"));
    //  m_simulation->Draw(ui->widgetSimulation);
    radioButtonTrackingNone(ui->radioButtonTrackingNone->isChecked());
    radioButtonTrackingX(ui->radioButtonTrackingX->isChecked());
    radioButtonTrackingY(ui->radioButtonTrackingY->isChecked());
    radioButtonTrackingZ(ui->radioButtonTrackingZ->isChecked());

    ui->doubleSpinBoxTimeMax->setValue(m_simulation->GetTimeLimit());
    QString time = QString("%1").arg(double(0), 0, 'f', 5);
    ui->lcdNumberTime->display(time);

    setStatusString(fileName + QString(" loaded"), 1);

    comboBoxMuscleColourMapCurrentTextChanged(ui->comboBoxMuscleColourMap->currentText());
    comboBoxMeshDisplayMapCurrentTextChanged(ui->comboBoxMeshDisplay->currentText());

    updateComboBoxTrackingBody();

    // put the filename as a title
    if (canonicalFilePath.size() <= 256) setWindowTitle(canonicalFilePath);
    else setWindowTitle(QString("...") + canonicalFilePath.right(256));

    // set menu activations for loaded model
    m_noName = false;
    m_saveRequired = meshPathChanged;
    enterRunMode();
    updateEnable();
}


void MainWindow::menuRestart()
{
    menuOpen(m_configFile.absoluteFilePath());
}

void MainWindow::menuSaveAs()
{
    QString fileName;
    if (m_configFile.absoluteFilePath().isEmpty())
    {
        QFileInfo info = Preferences::valueQString("LastFileOpened");
        fileName = QFileDialog::getSaveFileName(this, tr("Save Model State File (Relative)"), info.absoluteFilePath(),
                                                tr("Config Files (*.xml)"), nullptr);
    }
    else
    {
        fileName = QFileDialog::getSaveFileName(this, tr("Save Model State File (Relative)"), m_configFile.absoluteFilePath(),
                                                tr("Config Files (*.xml)"), nullptr);
    }

    if (fileName.isNull() == false)
    {
        setStatusString(fileName + QString(" saving"), 2);
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

void MainWindow::menuSave()
{
    if (m_noName) return;
    QString fileName = m_configFile.absoluteFilePath();
    setStatusString(fileName + QString(" saving"), 2);
    m_simulation->SetOutputModelStateFile(fileName.toStdString());
    m_simulation->OutputProgramState();
    setStatusString(fileName + QString(" saved"), 1);
    m_saveRequired = false;
}


void MainWindow::menuAbout()
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
    if (ui->actionRecordMovie->isChecked())
    {
        m_movieFlag = true;
        menuStartAVISave();
    }
    else
    {
        m_movieFlag = false;
        if (ui->widgetSimulation->aviWriter()) menuStopAVISave();
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

        if ((m_stepCount % size_t(Preferences::valueInt("MovieSkip"))) == 0)
        {
            Body *body = m_simulation->GetBody(ui->comboBoxTrackingBody->currentText().toStdString());
            if (body)
            {
                const double *position = dBodyGetPosition(body->GetBodyID());
                if (ui->radioButtonTrackingX->isChecked())
                {
                    ui->widgetSimulation->setCOIx(float(position[0] + ui->doubleSpinBoxTrackingOffset->value()));
                    ui->doubleSpinBoxCOIX->setValue(position[0] + ui->doubleSpinBoxTrackingOffset->value());
                }
                if (ui->radioButtonTrackingY->isChecked())
                {
                    ui->widgetSimulation->setCOIy(float(position[1] + ui->doubleSpinBoxTrackingOffset->value()));
                    ui->doubleSpinBoxCOIY->setValue(position[1] + ui->doubleSpinBoxTrackingOffset->value());
                }
                if (ui->radioButtonTrackingZ->isChecked())
                {
                    ui->widgetSimulation->setCOIz(float(position[2] + ui->doubleSpinBoxTrackingOffset->value()));
                    ui->doubleSpinBoxCOIZ->setValue(position[2] + ui->doubleSpinBoxTrackingOffset->value());
                }
            }
            if (m_stepFlag)
            {
                m_stepFlag = false;
                m_timer->stop();
            }
            ui->widgetSimulation->update();
            if (m_movieFlag)
            {
                ui->widgetSimulation->WriteMovieFrame();
            }
            if (m_saveOBJFileSequenceFlag)
            {
                QString filename = QString("%1%2").arg("Frame").arg(m_simulation->GetTime(), 12, 'f', 7, QChar('0'));
                QString path = QDir(m_objFileSequenceFolder).filePath(filename);
                ui->widgetSimulation->WriteCADFrame(path);
            }
            QString time = QString("%1").arg(m_simulation->GetTime(), 0, 'f', 5);
            ui->lcdNumberTime->display(time);
//            qDebug() << m_simulation->CalculateInstantaneousFitness() << "\n";
        }

        if (m_simulation->ShouldQuit())
        {
            setStatusString(tr("Simulation ended normally"), 1);
            ui->textEditLog->append(QString("Fitness = %1\n").arg(m_simulation->CalculateInstantaneousFitness(), 0, 'f', 5));
            ui->textEditLog->append(QString("Time = %1\n").arg(m_simulation->GetTime(), 0, 'f', 5));
            ui->textEditLog->append(QString("Metabolic Energy = %1\n").arg(m_simulation->GetMetabolicEnergy(), 0, 'f', 5));
            ui->textEditLog->append(QString("Mechanical Energy = %1\n").arg(m_simulation->GetMechanicalEnergy(), 0, 'f', 5));
            ui->widgetSimulation->update();
            QString time = QString("%1").arg(m_simulation->GetTime(), 0, 'f', 5);
            ui->lcdNumberTime->display(time);
            ui->actionRun->setChecked(false);
            run();
            return;
        }
        if (m_simulation->TestForCatastrophy())
        {
            setStatusString(tr("Simulation aborted"), 1);
            ui->textEditLog->append(QString("Fitness = %1\n").arg(m_simulation->CalculateInstantaneousFitness(), 0, 'f', 5));
            ui->widgetSimulation->update();
            QString time = QString("%1").arg(m_simulation->GetTime(), 0, 'f', 5);
            ui->lcdNumberTime->display(time);
            ui->actionRun->setChecked(false);
            run();
            return;
        }
    }
    updateEnable();
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
    QString filename = dir.absoluteFilePath(QString("Snapshot%1.png").arg(count, 5, 10, QChar('0')));
    if (ui->widgetSimulation->WriteStillFrame(filename))
    {
        QMessageBox::warning(nullptr, "Snapshot Error", QString("Could not write '%1'\n").arg(filename), "Click button to return to simulation");
        return;
    }
    setStatusString(QString("\"%1\" saved").arg(filename), 1);
}

void MainWindow::objSnapshot()
{
    QFileInfo info(Preferences::valueQString("LastFileOpened"));

    QString folder = QFileDialog::getExistingDirectory(this, tr("Choose folder to save current view as OBJ files"), info.absolutePath());

    if (folder.isNull() == false)
    {
        if (ui->widgetSimulation->WriteCADFrame(folder))
        {
            setStatusString(QString("Error: Folder '%1' write fail\n").arg(folder), 0);
            return;
        }
        setStatusString(QString("Files written in '%1'\n").arg(folder), 1);
    }
}

void MainWindow::buttonCameraRight()
{
    ui->widgetSimulation->setCameraVecX(0);
    ui->widgetSimulation->setCameraVecY(1);
    ui->widgetSimulation->setCameraVecZ(0);
    ui->widgetSimulation->setUpX(0);
    ui->widgetSimulation->setUpY(0);
    ui->widgetSimulation->setUpZ(1);
    ui->widgetSimulation->update();
}


void MainWindow::buttonCameraTop()
{
    ui->widgetSimulation->setCameraVecX(0);
    ui->widgetSimulation->setCameraVecY(0);
    ui->widgetSimulation->setCameraVecZ(-1);
    ui->widgetSimulation->setUpX(0);
    ui->widgetSimulation->setUpY(1);
    ui->widgetSimulation->setUpZ(0);
    ui->widgetSimulation->update();
}


void MainWindow::buttonCameraFront()
{
    ui->widgetSimulation->setCameraVecX(-1);
    ui->widgetSimulation->setCameraVecY(0);
    ui->widgetSimulation->setCameraVecZ(0);
    ui->widgetSimulation->setUpX(0);
    ui->widgetSimulation->setUpY(0);
    ui->widgetSimulation->setUpZ(1);
    ui->widgetSimulation->update();
}


void MainWindow::buttonCameraLeft()
{
    ui->widgetSimulation->setCameraVecX(0);
    ui->widgetSimulation->setCameraVecY(-1);
    ui->widgetSimulation->setCameraVecZ(0);
    ui->widgetSimulation->setUpX(0);
    ui->widgetSimulation->setUpY(0);
    ui->widgetSimulation->setUpZ(1);
    ui->widgetSimulation->update();
}


void MainWindow::buttonCameraBottom()
{
    ui->widgetSimulation->setCameraVecX(0);
    ui->widgetSimulation->setCameraVecY(0);
    ui->widgetSimulation->setCameraVecZ(1);
    ui->widgetSimulation->setUpX(0);
    ui->widgetSimulation->setUpY(1);
    ui->widgetSimulation->setUpZ(0);
    ui->widgetSimulation->update();
}


void MainWindow::buttonCameraBack()
{
    ui->widgetSimulation->setCameraVecX(1);
    ui->widgetSimulation->setCameraVecY(0);
    ui->widgetSimulation->setCameraVecZ(0);
    ui->widgetSimulation->setUpX(0);
    ui->widgetSimulation->setUpY(0);
    ui->widgetSimulation->setUpZ(1);
    ui->widgetSimulation->update();
}


void MainWindow::spinboxDistanceChanged(double v)
{
    Preferences::insert("CameraDistance", v);
    ui->widgetSimulation->setCameraDistance(float(v));
    ui->widgetSimulation->update();
}


void MainWindow::spinboxFoVChanged(double v)
{
    Preferences::insert("CameraFoV", v);
    ui->widgetSimulation->setFOV(float(v));
    ui->widgetSimulation->update();
}


void MainWindow::spinboxCOIXChanged(double v)
{
    Preferences::insert("CameraCOIX", v);
    ui->widgetSimulation->setCOIx(float(v));
    ui->widgetSimulation->update();
}


void MainWindow::spinboxCOIYChanged(double v)
{
    Preferences::insert("CameraCOIY", v);
    ui->widgetSimulation->setCOIy(float(v));
    ui->widgetSimulation->update();
}


void MainWindow::spinboxCOIZChanged(double v)
{
    Preferences::insert("CameraCOIZ", v);
    ui->widgetSimulation->setCOIz(float(v));
    ui->widgetSimulation->update();
}

void MainWindow::spinboxNearChanged(double v)
{
    Preferences::insert("CameraFrontClip", v);
    ui->widgetSimulation->setFrontClip(float(v));
    ui->widgetSimulation->update();
}

void MainWindow::spinboxFarChanged(double v)
{
    Preferences::insert("CameraBackClip", v);
    ui->widgetSimulation->setBackClip(float(v));
    ui->widgetSimulation->update();
}

void MainWindow::spinboxTrackingOffsetChanged(double v)
{
    Preferences::insert("TrackingOffset", v);
    if (m_simulation)
    {
        radioButtonTrackingX(ui->radioButtonTrackingX->isChecked());
        radioButtonTrackingY(ui->radioButtonTrackingY->isChecked());
        radioButtonTrackingZ(ui->radioButtonTrackingZ->isChecked());
    }
}

void MainWindow::radioButtonTrackingNone(int v)
{
    Preferences::insert("TrackingFlagNone", static_cast<bool>(v));
    if (v)
    {
        Preferences::insert("TrackingFlagX", false);
        Preferences::insert("TrackingFlagY", false);
        Preferences::insert("TrackingFlagZ", false);
    }
}

void MainWindow::radioButtonTrackingX(int v)
{
    Preferences::insert("TrackingFlagX", static_cast<bool>(v));
    if (m_simulation)
    {
        if (v)
        {
            Body *body = m_simulation->GetBody(Preferences::valueQString("TrackBodyID").toStdString());
            if (body)
            {
                const double *position = dBodyGetPosition(body->GetBodyID());
                ui->widgetSimulation->setCOIx(float(position[0] + ui->doubleSpinBoxTrackingOffset->value()));
                ui->doubleSpinBoxCOIX->setValue(position[0] + ui->doubleSpinBoxTrackingOffset->value());
                ui->widgetSimulation->update();
            }
        }
    }
}

void MainWindow::radioButtonTrackingY(int v)
{
    Preferences::insert("TrackingFlagY", static_cast<bool>(v));
    if (m_simulation)
    {
        if (v)
        {
            Body *body = m_simulation->GetBody(Preferences::valueQString("TrackBodyID").toStdString());
            if (body)
            {
                const double *position = dBodyGetPosition(body->GetBodyID());
                ui->widgetSimulation->setCOIy(float(position[1] + ui->doubleSpinBoxTrackingOffset->value()));
                ui->doubleSpinBoxCOIX->setValue(position[1] + ui->doubleSpinBoxTrackingOffset->value());
                ui->widgetSimulation->update();
            }
        }
    }
}

void MainWindow::radioButtonTrackingZ(int v)
{
    Preferences::insert("TrackingFlagZ", static_cast<bool>(v));
    if (m_simulation)
    {
        if (v)
        {
            Body *body = m_simulation->GetBody(Preferences::valueQString("TrackBodyID").toStdString());
            if (body)
            {
                const double *position = dBodyGetPosition(body->GetBodyID());
                ui->widgetSimulation->setCOIz(float(position[1] + ui->doubleSpinBoxTrackingOffset->value()));
                ui->doubleSpinBoxCOIX->setValue(position[1] + ui->doubleSpinBoxTrackingOffset->value());
                ui->widgetSimulation->update();
            }
        }
    }
}

void MainWindow::spinboxCursorSizeChanged(double v)
{
    Preferences::insert("CursorRadius", v);
    ui->widgetSimulation->setCursorRadius(float(v));
    ui->widgetSimulation->update();
}

void MainWindow::spinboxCursorNudgeChanged(double v)
{
    Preferences::insert("CursorNudge", v);
    ui->widgetSimulation->setCursor3DNudge(float(v));
}

void MainWindow::comboBoxMuscleColourMapCurrentTextChanged(const QString &text)
{
    Muscle::StrapColourControl colourControl = Muscle::fixedColour;
    if (text == "Strap Colour") colourControl = Muscle::fixedColour;
    else if (text == "Activation Colour") colourControl = Muscle::activationMap;
    else if (text == "Strain Colour") colourControl = Muscle::strainMap;
    else if (text == "Force Colour") colourControl = Muscle::forceMap;
    Preferences::insert("StrapColourControl", static_cast<int>(colourControl));
    if (m_simulation)
    {
        for (auto &&iter : *m_simulation->GetMuscleList()) iter.second->setStrapColourControl(colourControl);
    }
    ui->widgetSimulation->update();
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
        ui->widgetSimulation->setCursorColour(QColor(cursorColour.red(), cursorColour.green(), cursorColour.blue(), cursorColour.alpha()));
        ui->widgetSimulation->setCursorRadius(float(Preferences::valueDouble("CursorRadius")));
        ui->widgetSimulation->setCursor3DNudge(float(Preferences::valueDouble("CursorNudge")));
        ui->widgetSimulation->setFrontClip(float(Preferences::valueDouble("CameraFrontClip")));
        ui->widgetSimulation->setBackClip(float(Preferences::valueDouble("CameraBackClip")));

        ui->widgetSimulation->update();

    }

}

void MainWindow::menuOutputs()
{
    if (m_simulation == nullptr) return;
    DialogOutputSelect dialogOutputSelect(this);
    dialogOutputSelect.setSimulation(m_simulation);
    int status = dialogOutputSelect.exec();
    if (status == QDialog::Accepted)  { setStatusString(tr("Outputs set"), 1); }
    else { setStatusString(tr("Outputs setting cancelled"), 1); }
}

void MainWindow::setInterfaceValues()
{
    ui->widgetSimulation->setCameraDistance(float(Preferences::valueDouble("CameraDistance")));
    ui->widgetSimulation->setFOV(float(Preferences::valueDouble("CameraFoV")));
    ui->widgetSimulation->setCameraVecX(float(Preferences::valueDouble("CameraVecX")));
    ui->widgetSimulation->setCameraVecY(float(Preferences::valueDouble("CameraVecY")));
    ui->widgetSimulation->setCameraVecZ(float(Preferences::valueDouble("CameraVecZ")));
    ui->widgetSimulation->setCOIx(float(Preferences::valueDouble("CameraCOIX")));
    ui->widgetSimulation->setCOIy(float(Preferences::valueDouble("CameraCOIY")));
    ui->widgetSimulation->setCOIz(float(Preferences::valueDouble("CameraCOIZ")));
    ui->widgetSimulation->setFrontClip(float(Preferences::valueDouble("CameraFrontClip")));
    ui->widgetSimulation->setBackClip(float(Preferences::valueDouble("CameraBackClip")));
    ui->widgetSimulation->setUpX(float(Preferences::valueDouble("CameraUpX")));
    ui->widgetSimulation->setUpY(float(Preferences::valueDouble("CameraUpY")));
    ui->widgetSimulation->setUpZ(float(Preferences::valueDouble("CameraUpZ")));
    ui->widgetSimulation->setOrthographicProjection(Preferences::valueBool("OrthographicFlag"));

    QColor cursorColour = Preferences::valueQColor("CursorColour");
    ui->widgetSimulation->setCursorColour(QColor(cursorColour.red(), cursorColour.green(), cursorColour.blue(), cursorColour.alpha()));
    ui->widgetSimulation->setCursorRadius(float(Preferences::valueDouble("CursorRadius")));
    ui->widgetSimulation->setCursor3DNudge(float(Preferences::valueDouble("CursorNudge")));

    ui->doubleSpinBoxDistance->setValue(Preferences::valueDouble("CameraDistance"));
    ui->doubleSpinBoxFoV->setValue(Preferences::valueDouble("CameraFoV"));
    ui->doubleSpinBoxCOIX->setValue(Preferences::valueDouble("CameraCOIX"));
    ui->doubleSpinBoxCOIY->setValue(Preferences::valueDouble("CameraCOIY"));
    ui->doubleSpinBoxCOIZ->setValue(Preferences::valueDouble("CameraCOIZ"));
    ui->doubleSpinBoxFar->setValue(Preferences::valueDouble("CameraBackClip"));
    ui->doubleSpinBoxNear->setValue(Preferences::valueDouble("CameraFrontClip"));
    ui->doubleSpinBoxTrackingOffset->setValue(Preferences::valueDouble("TrackingOffset"));

    ui->radioButtonTrackingNone->setChecked(Preferences::valueBool("TrackingFlagNone"));
    ui->radioButtonTrackingX->setChecked(Preferences::valueBool("TrackingFlagX"));
    ui->radioButtonTrackingY->setChecked(Preferences::valueBool("TrackingFlagY"));
    ui->radioButtonTrackingZ->setChecked(Preferences::valueBool("TrackingFlagZ"));

    ui->comboBoxMuscleColourMap->setCurrentIndex(Preferences::valueInt("StrapColourControl"));

    ui->spinBoxSkip->setValue(Preferences::valueInt("MovieSkip"));
    ui->doubleSpinBoxFPS->setValue(Preferences::valueDouble("MovieFramerate"));

    ui->doubleSpinBoxCursorNudge->setValue(Preferences::valueDouble("CursorNudge"));
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

    ui->widgetSimulation->setCameraDistance(float(Preferences::valueDouble("DefaultCameraDistance")));
    ui->widgetSimulation->setFOV(float(Preferences::valueDouble("DefaultCameraFoV")));
    ui->widgetSimulation->setCameraVecX(float(Preferences::valueDouble("DefaultCameraVecX")));
    ui->widgetSimulation->setCameraVecY(float(Preferences::valueDouble("DefaultCameraVecY")));
    ui->widgetSimulation->setCameraVecZ(float(Preferences::valueDouble("DefaultCameraVecZ")));
    ui->widgetSimulation->setCOIx(float(Preferences::valueDouble("DefaultCameraCOIX")));
    ui->widgetSimulation->setCOIy(float(Preferences::valueDouble("DefaultCameraCOIY")));
    ui->widgetSimulation->setCOIz(float(Preferences::valueDouble("DefaultCameraCOIZ")));
    ui->widgetSimulation->setUpX(float(Preferences::valueDouble("DefaultCameraUpX")));
    ui->widgetSimulation->setUpY(float(Preferences::valueDouble("DefaultCameraUpY")));
    ui->widgetSimulation->setUpZ(float(Preferences::valueDouble("DefaultCameraUpZ")));
    ui->widgetSimulation->setBackClip(float(Preferences::valueDouble("DefaultCameraBackClip")));
    ui->widgetSimulation->setFrontClip(float(Preferences::valueDouble("DefaultCameraFrontClip")));

    ui->widgetSimulation->update();
}

void MainWindow::menuSaveDefaultView()
{
    Preferences::insert("DefaultTrackingOffset", ui->doubleSpinBoxTrackingOffset->value());

    Preferences::insert("DefaultCameraDistance", ui->widgetSimulation->cameraDistance());
    Preferences::insert("DefaultCameraFoV", ui->widgetSimulation->FOV());
    Preferences::insert("DefaultCameraCOIX", ui->widgetSimulation->COIx());
    Preferences::insert("DefaultCameraCOIY", ui->widgetSimulation->COIy());
    Preferences::insert("DefaultCameraCOIZ", ui->widgetSimulation->COIz());
    Preferences::insert("DefaultCameraVecX", ui->widgetSimulation->cameraVecX());
    Preferences::insert("DefaultCameraVecY", ui->widgetSimulation->cameraVecY());
    Preferences::insert("DefaultCameraVecZ", ui->widgetSimulation->cameraVecZ());
    Preferences::insert("DefaultCameraUpX", ui->widgetSimulation->upX());
    Preferences::insert("DefaultCameraUpY", ui->widgetSimulation->upY());
    Preferences::insert("DefaultCameraUpZ", ui->widgetSimulation->upZ());
    Preferences::insert("DefaultCameraBackClip", ui->widgetSimulation->backClip());
    Preferences::insert("DefaultCameraFrontClip", ui->widgetSimulation->frontClip());
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
    int deltaW = w - ui->widgetSimulation->width();
    int deltaH = h - ui->widgetSimulation->height();
    resize(width() + deltaW, height() + deltaH);
    ui->widgetSimulation->update();
}

SimulationWidget *MainWindow::simulationWidget() const
{
    return ui->widgetSimulation;
}

Simulation *MainWindow::simulation() const
{
    return m_simulation;
}

void MainWindow::resizeAndCentre(int w, int h)
{
    int screenNumber = QApplication::desktop()->screenNumber(this);
    if (screenNumber < 0) return;
    QList<QScreen *> screens = QGuiApplication::screens();
    QRect available = screens[screenNumber]->availableGeometry();

    // Need to find how big the central widget is compared to the window
    int heightDiff = height() - ui->widgetSimulation->height();
    int widthDiff = width() - ui->widgetSimulation->width();
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
    ui->widgetSimulation->update();
}

void MainWindow::menuStartAVISave()
{
    QFileInfo info(Preferences::valueQString("LastFileOpened"));

    QString fileName = QFileDialog::getSaveFileName(this, tr("Save output as AVI file"),
                       info.absolutePath(), tr("AVI Files (*.avi)"), nullptr);

    if (fileName.isNull() == false)
    {
        m_movieFlag = true;
        ui->widgetSimulation->StartAVISave(fileName);
    }
}

void MainWindow::menuStopAVISave()
{
    m_movieFlag = false;
    ui->widgetSimulation->StopAVISave();
}

void MainWindow::menuStartOBJSequenceSave()
{
    QFileInfo info(Preferences::valueQString("LastFileOpened"));

    m_objFileSequenceFolder = QFileDialog::getExistingDirectory(this,
                              tr("Choose folder to the OBJ file sequence"), info.absolutePath());

    if (m_objFileSequenceFolder.isNull() == false)
    {
        m_saveOBJFileSequenceFlag = true;
        ui->actionStartOBJSequence->setEnabled(false);
        ui->actionStopOBJSequence->setEnabled(true);
    }
}

void MainWindow::menuStopOBJSequenceSave()
{
    m_saveOBJFileSequenceFlag = false;
    ui->actionStartOBJSequence->setEnabled(true);
    ui->actionStopOBJSequence->setEnabled(false);
}

void MainWindow::menuNew()
{
    if (m_saveRequired)
    {
        QMessageBox msgBox;
        msgBox.setText("The document has been modified.");
        msgBox.setInformativeText("Click OK to quit, and Cancel to continue working on the document");
        msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Cancel);
        int ret = msgBox.exec();
        switch (ret)
        {
        case QMessageBox::Ok:
            break;
        case QMessageBox::Cancel:
            return;
        default:
            // should never be reached
            return;
        }
    }

    DialogGlobal dialogGlobal(this);
    Global global;
    dialogGlobal.setInputGlobal(&global);
    dialogGlobal.setExistingBodies(nullptr);
    dialogGlobal.lateInitialise();

    int status = dialogGlobal.exec();

    if (status == QDialog::Accepted)
    {
        if (m_simulation) delete m_simulation;
        m_simulation = nullptr;
        ui->widgetSimulation->setSimulation(m_simulation);
        m_stepCount = 0;
        ui->actionRun->setChecked(false);
        m_timer->stop();
        m_simulation = new Simulation();
        std::unique_ptr<Global> newGlobal = dialogGlobal.outputGlobal();
        newGlobal->setSimulation(m_simulation);
        m_simulation->SetGlobal(std::move(newGlobal));
        m_simulation->SetMainWindow(this);
        ui->widgetSimulation->setSimulation(m_simulation);
        ui->widgetSimulation->update();
        ui->treeWidgetElements->setSimulation(m_simulation);
        std::unique_ptr<Marker> marker = std::make_unique<Marker>(nullptr);
        marker->setName("WorldMarker"s);
        auto markersMap = m_simulation->GetMarkerList();
        (*markersMap)[marker->name()] = std::move(marker);
        ui->treeWidgetElements->fillVisibitilityLists(m_simulation);
        updateComboBoxTrackingBody();
        m_noName = true;
        m_saveRequired = false;
        enterConstructionMode();
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
        QApplication::postEvent( focused, new QKeyEvent( QEvent::KeyPress, Qt::Key_C, Qt::ControlModifier ));
        QApplication::postEvent( focused, new QKeyEvent( QEvent::KeyRelease, Qt::Key_C, Qt::ControlModifier ));
    }

}

void MainWindow::cut()
{
    QWidget *focused = QApplication::focusWidget();
    if ( focused != nullptr )
    {
        QApplication::postEvent( focused, new QKeyEvent( QEvent::KeyPress, Qt::Key_X, Qt::ControlModifier ));
        QApplication::postEvent( focused, new QKeyEvent( QEvent::KeyRelease, Qt::Key_X, Qt::ControlModifier ));
    }

}

void MainWindow::paste()
{
    QWidget *focused = QApplication::focusWidget();
    if ( focused != nullptr )
    {
        QApplication::postEvent( focused, new QKeyEvent( QEvent::KeyPress, Qt::Key_V, Qt::ControlModifier ));
        QApplication::postEvent( focused, new QKeyEvent( QEvent::KeyRelease, Qt::Key_V, Qt::ControlModifier ));
    }

}

void MainWindow::selectAll()
{
    QWidget *focused = QApplication::focusWidget();
    if ( focused != nullptr )
    {
        QApplication::postEvent( focused, new QKeyEvent( QEvent::KeyPress, Qt::Key_A, Qt::ControlModifier ));
        QApplication::postEvent( focused, new QKeyEvent( QEvent::KeyRelease, Qt::Key_A, Qt::ControlModifier ));
    }

}

QByteArray MainWindow::readResource(const QString &resource)
{
    QFile file(resource);
    bool ok = file.open(QIODevice::ReadOnly);
    Q_ASSERT_X(ok, "MainWindow::readResource", "resource not found");
    return file.readAll();
}

void MainWindow::menuStartWarehouseExport()
{
    if (m_simulation == nullptr) return;

    QFileInfo info(Preferences::valueQString("LastFileOpened"));
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save output as Warehouse file"), info.absolutePath(), tr("Text Files (*.txt)"), nullptr);

    if (fileName.isNull() == false)
    {
        ui->actionStartWarehouseExport->setEnabled(false);
        ui->actionStopWarehouseExport->setEnabled(true);
        m_simulation->SetOutputWarehouseFile(fileName.toStdString());
    }
}

void MainWindow::menuStopWarehouseExport()
{
    if (m_simulation == nullptr) return;

    ui->actionStartWarehouseExport->setEnabled(true);
    ui->actionStopWarehouseExport->setEnabled(false);
    m_simulation->SetOutputWarehouseFile(nullptr);
}

void MainWindow::menuImportWarehouse()
{
    if (m_simulation == nullptr) return;
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Warehouse File"), "", tr("Warehouse Files (*.txt);;Any File (*.* *)"), nullptr);

    if (fileName.isNull() == false)
    {
        m_simulation->AddWarehouse(fileName.toStdString());
        setStatusString(QString("Warehouse %1 added").arg(fileName), 1);
    }
}

void MainWindow::menuToggleFullScreen()
{
    setWindowState(windowState() ^ Qt::WindowFullScreen);
}

void MainWindow::menuGeneticAlgorithm()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Genetic Algorithm Search Control File"), "", tr("Search Control Files (*.txt);;Any File (*.* *)"), nullptr);
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
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Next Ascent Hill Climbing Search Control File"), "", tr("Search Control Files (*.txt);;Any File (*.* *)"),
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
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Random Ascent Hill Climbing Search Control File"), "", tr("Search Control Files (*.txt);;Any File (*.* *)"),
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
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Simplex Search Control File"), "", tr("Search Control Files (*.txt);;Any File (*.* *)"), nullptr);
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
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Simulated Annealing Search Control File"), "", tr("Search Control Files (*.txt);;Any File (*.* *)"),
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
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Tabu Search Control File"), "", tr("Search Control Files (*.txt);;Any File (*.* *)"), nullptr);
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
    ui->actionRestart->setEnabled(m_simulation != nullptr && m_mode == runMode && m_noName == false && m_saveRequired == false);
    ui->actionSave->setEnabled(m_simulation != nullptr && m_noName == false && m_saveRequired == true);
    ui->actionSaveAs->setEnabled(m_simulation != nullptr);
    ui->actionExportMarkers->setEnabled(m_simulation != nullptr);
    ui->actionStartWarehouseExport->setEnabled(m_simulation != nullptr && m_mode == runMode);
    ui->actionStopWarehouseExport->setEnabled(m_simulation != nullptr && m_mode == runMode);
    ui->actionImportWarehouse->setEnabled(m_simulation != nullptr && m_mode == runMode);
    ui->actionRecordMovie->setEnabled(m_simulation != nullptr && m_mode == runMode);
    ui->actionRun->setEnabled(m_simulation != nullptr && m_mode == runMode);
    ui->actionStep->setEnabled(m_simulation != nullptr && m_mode == runMode);
    ui->actionSnapshot->setEnabled(m_simulation != nullptr);
    ui->actionSaveOBJSnapshot->setEnabled(m_simulation != nullptr && m_mode == runMode);
    ui->actionStartOBJSequence->setEnabled(m_simulation != nullptr && m_mode == runMode);
    ui->actionStopOBJSequence->setEnabled(m_simulation != nullptr && m_mode == runMode);
    ui->actionCreateBody->setEnabled(m_simulation != nullptr && m_mode == constructionMode);
    ui->actionCreateMarker->setEnabled(m_simulation != nullptr && m_mode == constructionMode && m_simulation->GetBodyList()->size() > 0);
    ui->actionCreateJoint->setEnabled(m_simulation != nullptr && m_mode == constructionMode && m_simulation->GetBodyList()->size() > 1 && m_simulation->GetMarkerList()->size() > 0);
    ui->actionCreateMuscle->setEnabled(m_simulation != nullptr && m_mode == constructionMode && m_simulation->GetBodyList()->size() > 1 && m_simulation->GetMarkerList()->size() > 0);
    ui->actionCreateGeom->setEnabled(m_simulation != nullptr && m_mode == constructionMode && m_simulation->GetBodyList()->size() > 0 && m_simulation->GetMarkerList()->size() > 0);
    ui->actionCreateDriver->setEnabled(m_simulation != nullptr && m_mode == constructionMode && (m_simulation->GetMuscleList()->size() > 0 || m_simulation->GetControllerList()->size() > 0));
    ui->actionEditGlobal->setEnabled(m_simulation != nullptr && m_mode == constructionMode);
    ui->actionCreateAssembly->setEnabled(m_simulation != nullptr && m_mode == constructionMode && m_stepCount == 0);
    ui->actionDeleteAssembly->setEnabled(m_simulation != nullptr && m_mode == constructionMode);
    ui->actionConstructionMode->setEnabled(m_simulation != nullptr && m_mode == runMode && m_stepCount == 0);
    ui->actionRunMode->setEnabled(m_simulation != nullptr && m_mode == constructionMode && m_simulation->GetBodyList()->size() > 0);
}

void MainWindow::menuCreateBody()
{
    menuCreateEditBody(nullptr);
}

void MainWindow::menuCreateEditBody(Body *body)
{
    Q_ASSERT_X(m_simulation, "MainWindow::menuCreateBody", "m_simulation undefined");
    DialogBodyBuilder dialogBodyBuilder(this);
    dialogBodyBuilder.setSimulation(m_simulation);
    dialogBodyBuilder.setInputBody(body);
    dialogBodyBuilder.lateInitialise();
    int status = dialogBodyBuilder.exec();
    if (status == QDialog::Accepted)
    {
        if (!body) // this is the create body option so there will be no dependencies
        {
            std::unique_ptr<Body> newBody = dialogBodyBuilder.outputBody();
            newBody->LateInitialisation();
            std::string newBodyName = newBody->name();
            // insert the new centre of mass marker
            std::unique_ptr<Marker> cmMarker = std::make_unique<Marker>(newBody.get());
            std::string cmMarkerName = dialogBodyBuilder.outputBody()->name() + "_CM_Marker"s;
            cmMarker->setName(cmMarkerName);
            (*m_simulation->GetMarkerList())[cmMarkerName] = std::move(cmMarker);
            ui->treeWidgetElements->insertMarker(QString().fromStdString(cmMarkerName));
            this->setStatusString(QString("New marker created: %1").arg(QString::fromStdString(cmMarkerName)), 1);
            // insert the new body
            (*m_simulation->GetBodyList())[newBodyName] = std::move(newBody);
            ui->treeWidgetElements->insertBody(QString().fromStdString(newBodyName));
            this->setStatusString(QString("New body created: %1").arg(QString::fromStdString(newBodyName)), 1);
            updateComboBoxTrackingBody();
        }
        else // this is an edit so all that will have happened is that things have moved
        {
            body->setRedraw(true);
            body->LateInitialisation();
            for (auto &&markerIt : *simulation()->GetMarkerList())
            {
                if (markerIt.second->GetBody() == body)
                {
                    std::set<NamedObject *> dependentList = *markerIt.second->dependentList();
                    std::map<std::string, std::string> serialiseMap = markerIt.second->serialise();
                    markerIt.second->unserialise(serialiseMap);
                    markerIt.second->setRedraw(true);
                    for (auto &&depIt : dependentList)
                    {
                        serialiseMap = depIt->serialise();
                        depIt->unserialise(serialiseMap);
                        depIt->setRedraw(true);
                        // everything needs a redraw but somethings also need extra work
                        if (dynamic_cast<Strap *>(depIt)) dynamic_cast<Strap *>(depIt)->Calculate();
                    }
                }
            }
             this->setStatusString(QString("Body edited: %1").arg(QString::fromStdString(body->name())), 1);
        }
        m_saveRequired = true;
        this->updateEnable();
        ui->widgetSimulation->update();
    }
    else
    {
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
    Q_ASSERT_X(m_simulation->GetBodyList()->size(), "MainWindow::menuCreateEditMarker", "No bodies defined");
    DialogMarkers dialogMarkers(this);
    dialogMarkers.setCursor3DPosition(ui->widgetSimulation->cursor3DPosition());
    dialogMarkers.setInputMarker(marker);
    dialogMarkers.setSimulation(m_simulation);
    dialogMarkers.lateInitialise();
    if (sender() == ui->widgetSimulation && ui->widgetSimulation->getLastMenuItem() != tr("Edit Marker..."))
    {
        if (ui->widgetSimulation->getClosestHit())
        {
            pgd::Vector location = ui->widgetSimulation->getClosestHit()->worldLocation();
            dialogMarkers.overrideStartPosition(location);
        }
    }
    int status = dialogMarkers.exec();
    if (status == QDialog::Accepted)
    {
        if (!marker) // create so no dependencies
        {
            std::unique_ptr<Marker> newMarker = dialogMarkers.outputMarker();
            std::string newMarkerName = newMarker->name();
            (*m_simulation->GetMarkerList())[newMarkerName] = std::move(newMarker);
            this->setStatusString(QString("New marker created: %1").arg(QString::fromStdString(newMarkerName)), 1);
            ui->treeWidgetElements->insertMarker(QString().fromStdString(newMarkerName));
        }
        else // editing a marker so need to cope with dependencies
        {
            marker->setRedraw(true);
            std::map<std::string, std::string> serialiseMap;
            for (auto &&depIt : *marker->dependentList())
            {
                serialiseMap = depIt->serialise();
                depIt->unserialise(serialiseMap);
                depIt->setRedraw(true);
                // everything needs a redraw but somethings also need extra work
                if (dynamic_cast<Strap *>(depIt)) dynamic_cast<Strap *>(depIt)->Calculate();
                // FIX ME - need to do something about the tegotae driver
            }
            this->setStatusString(QString("Marker edited: %1").arg(QString::fromStdString(marker->name())), 1);
        }

        m_saveRequired = true;
        updateEnable();
        ui->widgetSimulation->update();
    }
    else
    {
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
    Q_ASSERT_X(m_simulation->GetBodyList()->size(), "MainWindow::menuCreateEditMarker", "No bodies defined");
    DialogJoints dialogJoints(this);
    dialogJoints.setSimulation(m_simulation);
    dialogJoints.setInputJoint(joint);
    dialogJoints.lateInitialise();
    int status = dialogJoints.exec();
    if (status == QDialog::Accepted)
    {
        if (!joint) // creating a new joint
        {
            std::unique_ptr<Joint> newJoint = dialogJoints.outputJoint();
            std::string newJointName = newJoint->name();
            newJoint->LateInitialisation();
            (*m_simulation->GetJointList())[newJointName] = std::move(newJoint);
            this->setStatusString(QString("New joint created: %1").arg(QString::fromStdString(newJointName)), 1);
            ui->treeWidgetElements->insertJoint(QString().fromStdString(newJointName));
        }
        else // replacing an existing joint
        {
            std::unique_ptr<Joint> replacementJoint = dialogJoints.outputJoint();
            std::string replacementJointName = replacementJoint->name();
            replacementJoint->LateInitialisation();
            auto drivableList = simulation()->GetDrivableList();
            (*simulation()->GetJointList())[replacementJointName] = std::move(replacementJoint);
            this->setStatusString(QString("Joint edited: %1").arg(QString::fromStdString(replacementJointName)), 1);
        }
        m_saveRequired = true;
        this->updateEnable();
        ui->widgetSimulation->update();
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
    auto bodiesMap = m_simulation->GetBodyList();
    Q_ASSERT_X(bodiesMap->size(), "MainWindow::menuCreateEditMarker", "No bodies defined");
    DialogMuscles dialogMuscles(this);
    dialogMuscles.setSimulation(m_simulation);
    dialogMuscles.setInputMuscle(muscle);
    dialogMuscles.lateInitialise();
    int status = dialogMuscles.exec();
    if (status == QDialog::Accepted)
    {
        if (!muscle) // creating a new muscle
        {
            std::unique_ptr<Strap> newStrap = dialogMuscles.outputStrap();
            std::string newStrapName = newStrap->name();
            (*m_simulation->GetStrapList())[newStrapName] = std::move(newStrap);
            std::unique_ptr<Muscle> newMuscle = dialogMuscles.outputMuscle();
            muscle = newMuscle.get();
            std::string newMuscleName = newMuscle->name();
            (*m_simulation->GetMuscleList())[newMuscleName] = std::move(newMuscle);
            this->setStatusString(QString("New muscle created: %1").arg(QString::fromStdString(newMuscleName)), 1);
            ui->treeWidgetElements->insertMuscle(QString().fromStdString(newMuscleName));
        }
        else // replacing an existing muscle
        {
            std::unique_ptr<Strap> replacementStrap = dialogMuscles.outputStrap();
            std::string replacementStrapName = replacementStrap->name();
            (*m_simulation->GetStrapList())[replacementStrapName] = std::move(replacementStrap);
            std::unique_ptr<Muscle> replacementMuscle = dialogMuscles.outputMuscle();
            muscle = replacementMuscle.get();
            std::string replacementMuscleName = replacementMuscle->name();
            (*m_simulation->GetMuscleList())[replacementMuscleName] = std::move(replacementMuscle);
            this->setStatusString(QString("Muscle edited: %1").arg(QString::fromStdString(replacementMuscleName)), 1);
            ui->treeWidgetElements->insertMuscle(QString().fromStdString(replacementMuscleName));
        }

        m_saveRequired = true;
        Muscle::StrapColourControl colourControl = Muscle::fixedColour;
        QString text = ui->comboBoxMuscleColourMap->currentText();
        if (text == "Strap Colour") colourControl = Muscle::fixedColour;
        else if (text == "Activation Colour") colourControl = Muscle::activationMap;
        else if (text == "Strain Colour") colourControl = Muscle::strainMap;
        else if (text == "Force Colour") colourControl = Muscle::forceMap;
        muscle->setStrapColourControl(colourControl);
        muscle->LateInitialisation();
        this->updateEnable();
        ui->widgetSimulation->update();
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
    Q_ASSERT_X(m_simulation->GetBodyList()->size(), "MainWindow::menuCreateEditMarker", "No bodies defined");
    DialogGeoms dialogGeoms(this);
    dialogGeoms.setSimulation(m_simulation);
    dialogGeoms.setInputGeom(geom);
    dialogGeoms.lateInitialise();
    int status = dialogGeoms.exec();
    if (status == QDialog::Accepted)
    {
        if (!geom) // creating a new geom
        {
            std::unique_ptr<Geom> newGeom = dialogGeoms.outputGeom();
            std::string newGeomName = newGeom->name();
            (*m_simulation->GetGeomList())[newGeomName] = std::move(newGeom);
            this->setStatusString(QString("New geom created: %1").arg(QString::fromStdString(newGeomName)), 1);
            ui->treeWidgetElements->insertGeom(QString().fromStdString(newGeomName));
        }
        else // replacing an existing geom
        {
            std::unique_ptr<Geom> replacementGeom = dialogGeoms.outputGeom();
            std::string replacementGeomName = replacementGeom->name();
            (*m_simulation->GetGeomList())[replacementGeomName] = std::move(replacementGeom);
            this->setStatusString(QString("Geom edited: %1").arg(QString::fromStdString(replacementGeomName)), 1);
            ui->treeWidgetElements->insertGeom(QString().fromStdString(replacementGeomName));
        }
        m_saveRequired = true;
        this->updateEnable();
        ui->widgetSimulation->update();
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
    Q_ASSERT_X(m_simulation->GetBodyList()->size(), "MainWindow::menuCreateEditMarker", "No bodies defined");
    DialogDrivers dialogDrivers(this);
    dialogDrivers.setSimulation(m_simulation);
    dialogDrivers.setInputDriver(driver);
    dialogDrivers.lateInitialise();
    int status = dialogDrivers.exec();
    if (status == QDialog::Accepted)
    {
        if (!driver) // creating a new driver
        {
            std::unique_ptr<Driver> newDriver = dialogDrivers.outputDriver();
            std::string newDriverName = newDriver->name();
            (*simulation()->GetDriverList())[newDriverName] = std::move(newDriver);
            this->setStatusString(QString("New driver created: %1").arg(QString::fromStdString(newDriverName)), 1);
            ui->treeWidgetElements->insertDriver(QString().fromStdString(newDriverName));
        }
        else // replacing an existing driver
        {
            std::unique_ptr<Driver> replacementDriver = dialogDrivers.outputDriver();
            std::string replacementDriverName = replacementDriver->name();
            (*simulation()->GetDriverList())[replacementDriverName] = std::move(replacementDriver);
            this->setStatusString(QString("Driver edited: %1").arg(QString::fromStdString(replacementDriverName)), 1);
        }
        m_saveRequired = true;
        this->updateEnable();
        ui->widgetSimulation->update();
    }
    else
    {
        this->setStatusString(tr("Driver creation cancelled"), 2);
    }
}

void MainWindow::deleteExistingBody(const QString &name, bool force)
{
    int ret = QMessageBox::Ok;
    if (!force) ret = QMessageBox::warning(this, tr("Delete Body %1").arg(name), tr("This action cannot be undone.\nAre you sure you want to continue?"), QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel);
    if (ret == QMessageBox::Ok)
    {
        // first delete dependencies
        std::string nameStr = name.toStdString();
        // we need to explicitly delete the joints
        for (auto it = m_simulation->GetJointList()->begin(); it != m_simulation->GetJointList()->end();)
        {
            auto nextIt = it;
            nextIt++;
            size_t oldSize = m_simulation->GetJointList()->size();
            if (it->second->GetBody1()->name() == nameStr || it->second->GetBody2()->name() == nameStr)
            {
                deleteExistingJoint(QString::fromStdString(it->second->name()));
                if (m_simulation->GetJointList()->size() == oldSize)
                    return; // nothing was deleted so the user cancelled
            }
            it = nextIt;
        }
        // and deleting markers will clear up everything else
        for (auto it = m_simulation->GetMarkerList()->begin(); it != m_simulation->GetMarkerList()->end();)
        {
            auto nextIt = it;
            nextIt++;
            size_t oldSize = m_simulation->GetMarkerList()->size();
            if (it->second->GetBody() && it->second->GetBody()->name() == nameStr) // note: world markers do not have a body
            {
                deleteExistingMarker(QString::fromStdString(it->second->name()));
                if (m_simulation->GetMarkerList()->size() == oldSize)
                    return; // nothing was deleted so the user cancelled
            }
            it = nextIt;
        }
        // now delete the body itself
        auto it = m_simulation->GetBodyList()->find(nameStr);
        if (it != m_simulation->GetBodyList()->end())
        {
            m_simulation->GetBodyList()->erase(it);
            ui->treeWidgetElements->removeBody(name);
        }
        updateComboBoxTrackingBody();
        m_saveRequired = true;
        updateEnable();
        ui->widgetSimulation->update();
    }
}

void MainWindow::deleteExistingMarker(const QString &name, bool force)
{
    int ret = QMessageBox::Ok;
    if (!force) ret = QMessageBox::warning(this, tr("Delete Marker %1").arg(name), tr("This action cannot be undone.\n" "Are you sure you want to continue?"), QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel);
    if (ret == QMessageBox::Ok)
    {
        // first delete dependencies (lots of things can depend on markers)
        std::string nameStr = name.toStdString();
        Marker *marker = m_simulation->GetMarkerList()->at(nameStr).get();
        // we need to explicitly delete the joints
        for (auto it = m_simulation->GetJointList()->begin(); it != m_simulation->GetJointList()->end();)
        {
            auto nextIt = it;
            nextIt++;
            size_t oldSize = m_simulation->GetJointList()->size();
            if (it->second->body1Marker() == marker || it->second->body2Marker() == marker)
            {
                deleteExistingJoint(QString::fromStdString( it->second->name()));
                if (m_simulation->GetJointList()->size() == oldSize)
                    return; // nothing was deleted so the user cancelled
            }
            it = nextIt;
        }
        // we need to explicitly delete the muscles
        for (auto it = m_simulation->GetMuscleList()->begin(); it != m_simulation->GetMuscleList()->end();)
        {
            auto nextIt = it;
            nextIt++;
            size_t oldSize = m_simulation->GetMuscleList()->size();
            Strap *strap = it->second->GetStrap();
            strap->updateDependentMarkers();
            auto it2 = std::find(strap->dependentMarkers()->begin(), strap->dependentMarkers()->end(), marker);
            if (it2 != strap->dependentMarkers()->end())
            {
                deleteExistingMuscle(QString::fromStdString(it->second->name()));
                if (m_simulation->GetMuscleList()->size() == oldSize)
                    return; // nothing was deleted so the user cancelled
            }
            it = nextIt;
        }
        // we need to explicitly delete the geoms
        for (auto it = m_simulation->GetGeomList()->begin(); it != m_simulation->GetGeomList()->end();)
        {
            auto nextIt = it;
            nextIt++;
            size_t oldSize = m_simulation->GetGeomList()->size();
            if (it->second->geomMarker() == marker)
            {
                deleteExistingGeom(QString::fromStdString(it->second->name()));
                if (m_simulation->GetGeomList()->size() == oldSize)
                    return; // nothing was deleted so the user cancelled
            }
            it = nextIt;
        }
        // FIX ME - check the tegotae driver
        // now delete the marker itself
        auto it = m_simulation->GetMarkerList()->find(nameStr);
        if (it != m_simulation->GetMarkerList()->end())
        {

            m_simulation->GetMarkerList()->erase(it);
            ui->treeWidgetElements->removeMarker(name);
        }
        m_saveRequired = true;
        updateEnable();
        ui->widgetSimulation->update();
    }

}

void MainWindow::deleteExistingJoint(const QString &name, bool force)
{
    int ret = QMessageBox::Ok;
    if (!force) ret = QMessageBox::warning(this, tr("Delete Joint %1").arg(name), tr("This action cannot be undone.\nAre you sure you want to continue?"), QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel);
    if (ret == QMessageBox::Ok)
    {
        std::string nameStr = name.toStdString();
        auto it = m_simulation->GetJointList()->find(nameStr);
        if (it != m_simulation->GetJointList()->end())
        {
            m_simulation->GetJointList()->erase(it);
            ui->treeWidgetElements->removeJoint(name);
        }
        m_saveRequired = true;
        updateEnable();
        ui->widgetSimulation->update();
    }
}

void MainWindow::deleteExistingMuscle(const QString &name, bool force)
{
    int ret = QMessageBox::Ok;
    if (!force) ret = QMessageBox::warning(this, tr("Delete Muscle %1").arg(name), tr("This action cannot be undone.\n" "Are you sure you want to continue?"), QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel);
    if (ret == QMessageBox::Ok)
    {
        std::string nameStr = name.toStdString();
        auto muscleIt = m_simulation->GetMuscleList()->find(nameStr);
        if (muscleIt != m_simulation->GetMuscleList()->end())
        {
            auto strapIt = m_simulation->GetStrapList()->find(muscleIt->second->GetStrap()->name());
            if (strapIt != m_simulation->GetStrapList()->end())
            {
                m_simulation->GetStrapList()->erase(strapIt);
            }
            m_simulation->GetMuscleList()->erase(muscleIt);
            ui->treeWidgetElements->removeMuscle(name);
        }
        m_saveRequired = true;
        updateEnable();
        ui->widgetSimulation->update();
    }
}

void MainWindow::deleteExistingDriver(const QString &name, bool force)
{
    int ret = QMessageBox::Ok;
    if (!force) ret = QMessageBox::warning(this, tr("Delete Driver %1").arg(name), tr("This action cannot be undone.\n" "Are you sure you want to continue?"), QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel);
    if (ret == QMessageBox::Ok)
    {
        std::string nameStr = name.toStdString();
        auto it = m_simulation->GetDriverList()->find(nameStr);
        if (it != m_simulation->GetDriverList()->end())
        {
            m_simulation->GetDriverList()->erase(it);
            ui->treeWidgetElements->removeDriver(name);
        }
        m_saveRequired = true;
        updateEnable();
        ui->widgetSimulation->update();
    }
}

void MainWindow::deleteExistingGeom(const QString &name, bool force)
{
    int ret = QMessageBox::Ok;
    if (!force) ret = QMessageBox::warning(this, tr("Delete Geom %1").arg(name), tr("This action cannot be undone.\n" "Are you sure you want to continue?"), QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel);
    if (ret == QMessageBox::Ok)
    {
        std::string nameStr = name.toStdString();
        auto it = m_simulation->GetGeomList()->find(nameStr);
        if (it != m_simulation->GetGeomList()->end())
        {
            m_simulation->GetGeomList()->erase(it);
            ui->treeWidgetElements->removeGeom(name);
        }
        m_saveRequired = true;
        updateEnable();
        ui->widgetSimulation->update();
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
    dialogGlobal.setInputGlobal(m_simulation->GetGlobal());
    dialogGlobal.setExistingBodies(m_simulation->GetBodyList());
    dialogGlobal.lateInitialise();

    int status = dialogGlobal.exec();

    if (status == QDialog::Accepted)   // write the new settings
    {
        m_simulation->SetGlobal(dialogGlobal.outputGlobal());
        m_simulation->GetGlobal()->setSimulation(m_simulation);
        setStatusString(tr("Global values edited"), 1);
        m_saveRequired = true;
        updateEnable();
        ui->widgetSimulation->setAxesScale(float(m_simulation->GetGlobal()->size1()));
        ui->widgetSimulation->setBackgroundColour(QString::fromStdString(m_simulation->GetGlobal()->colour1().GetHexArgb()));
        ui->widgetSimulation->update();
    }
    else
    {
        setStatusString(tr("Global values unchanged"), 2);
    }
}


void MainWindow::comboBoxMeshDisplayMapCurrentTextChanged(const QString &text)
{
    if (text == QString("Mesh 1"))
    {
        ui->widgetSimulation->setDrawBodyMesh1(true);
        ui->widgetSimulation->setDrawBodyMesh2(false);
        ui->widgetSimulation->setDrawBodyMesh3(false);
    }
    if (text == QString("Mesh 2"))
    {
        ui->widgetSimulation->setDrawBodyMesh1(false);
        ui->widgetSimulation->setDrawBodyMesh2(true);
        ui->widgetSimulation->setDrawBodyMesh3(false);
    }
    if (text == QString("Mesh 3"))
    {
        ui->widgetSimulation->setDrawBodyMesh1(false);
        ui->widgetSimulation->setDrawBodyMesh2(false);
        ui->widgetSimulation->setDrawBodyMesh3(true);
    }
    if (text == QString("Mesh 1 & 2"))
    {
        ui->widgetSimulation->setDrawBodyMesh1(true);
        ui->widgetSimulation->setDrawBodyMesh2(true);
        ui->widgetSimulation->setDrawBodyMesh3(false);
    }
    if (text == QString("Mesh 2 & 3"))
    {
        ui->widgetSimulation->setDrawBodyMesh1(false);
        ui->widgetSimulation->setDrawBodyMesh2(true);
        ui->widgetSimulation->setDrawBodyMesh3(true);
    }
    if (text == QString("Mesh 1 & 3"))
    {
        ui->widgetSimulation->setDrawBodyMesh1(true);
        ui->widgetSimulation->setDrawBodyMesh2(false);
        ui->widgetSimulation->setDrawBodyMesh3(true);
    }
    if (text == QString("All Meshes"))
    {
        ui->widgetSimulation->setDrawBodyMesh1(true);
        ui->widgetSimulation->setDrawBodyMesh2(true);
        ui->widgetSimulation->setDrawBodyMesh3(true);
    }
    ui->widgetSimulation->update();
}

void MainWindow::comboBoxTrackingBodyCurrentTextChanged(const QString &)
{
    Preferences::insert("TrackBodyID", ui->comboBoxTrackingBody->currentText());
}


void MainWindow::handleElementTreeWidgetItemChanged(QTreeWidgetItem * /* item */, int column)
{
    if (column == 1) ui->widgetSimulation->update();
}

void MainWindow::enterRunMode()
{
    Q_ASSERT_X(m_simulation, "MainWindow::enterRunMode", "m_simulation undefined");
    m_mode = runMode;
    for (auto &&it : *m_simulation->GetBodyList()) it.second->EnterRunMode();
    for (auto &&it : *m_simulation->GetMuscleList()) it.second->LateInitialisation();
    for (auto &&it : *m_simulation->GetFluidSacList()) it.second->LateInitialisation();
    for (auto &&it : *m_simulation->GetJointList()) it.second->LateInitialisation();
    updateEnable();
    ui->widgetSimulation->update();
}

void MainWindow::enterConstructionMode()
{
    Q_ASSERT_X(m_simulation, "MainWindow::enterConstructionMode", "m_simulation undefined");
    Q_ASSERT_X(m_stepCount == 0, "MainWindow::enterConstructionMode", "m_stepCount not zero");
    m_mode = constructionMode;
    for (auto &&it : *m_simulation->GetBodyList()) it.second->EnterConstructionMode();
    for (auto &&it : *m_simulation->GetMuscleList()) it.second->LateInitialisation();
    for (auto &&it : *m_simulation->GetFluidSacList()) it.second->LateInitialisation();
    updateEnable();
    ui->widgetSimulation->update();
}

void MainWindow::menuCreateAssembly()
{
    Q_ASSERT_X(m_simulation, "MainWindow::menuCreateAssembly", "m_simulation undefined");
    DialogAssembly dialogAssembly(this);
    dialogAssembly.setSimulation(m_simulation);
    dialogAssembly.initialise();
    connect(&dialogAssembly, SIGNAL(jointCreated(const QString &)), ui->treeWidgetElements, SLOT(insertJoint(const QString &)));
    connect(&dialogAssembly, SIGNAL(markerCreated(const QString &)), ui->treeWidgetElements, SLOT(insertMarker(const QString &)));
    connect(&dialogAssembly, SIGNAL(jointDeleted(const QString &)), ui->treeWidgetElements, SLOT(deleteJoint(const QString &)));
    connect(&dialogAssembly, SIGNAL(markerDeleted(const QString &)), ui->treeWidgetElements, SLOT(deleteMarker(const QString &)));
    int status = dialogAssembly.exec();
    if (status == QDialog::Accepted)
    {
        setStatusString(tr("Assembly constraints created"), 2);
        m_saveRequired = true;
    }
    else
    {
        setStatusString(tr("Assembly cancelled"), 2);
    }
    updateEnable();
    ui->widgetSimulation->update();
}

void MainWindow::menuDeleteAssembly()
{
    int ret = QMessageBox::warning(this, tr("Delete Assembly"), tr("This action cannot be undone.\nAre you sure you want to continue?"), QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel);
    if (ret == QMessageBox::Ok)
    {
        for (auto iter = m_simulation->GetJointList()->begin(); iter != m_simulation->GetJointList()->end(); /* no increment */)
        {
            if (iter->second->group() == "assembly"s)
            {
                QString name = QString::fromStdString(iter->first);
                iter++;
                deleteExistingJoint(name, true);
            }
            else
            {
                iter++;
            }
        }
        for (auto iter = m_simulation->GetMarkerList()->begin(); iter != m_simulation->GetMarkerList()->end(); /* no increment */)
        {
            if (iter->second->group() == "assembly"s)
            {
                QString name = QString::fromStdString(iter->first);
                iter++;
                deleteExistingMarker(name, true);
            }
            else
            {
                iter++;
            }
        }
        setStatusString(tr("Assembly constraints deleted"), 2);
        m_saveRequired = true;
        updateEnable();
    }
}

void MainWindow::moveExistingMarker(const QString &s, const QVector3D &p)
{
    auto it = m_simulation->GetMarkerList()->find(s.toStdString());
    if (it == m_simulation->GetMarkerList()->end()) return;
    it->second->SetWorldPosition(double(p.x()), double(p.y()), double(p.z()));
    it->second->setRedraw(true);
    for (auto nb : *it->second->dependentList())
    {
        qDebug() << nb->name().c_str() << " dependent on " << it->second->name().c_str();
        nb->saveToAttributes();
        nb->createFromAttributes();
        Strap *strap = dynamic_cast<Strap *>(nb);
        if (strap) strap->Calculate();
        // FIX ME - need to do something about the tegotae driver
    }
    m_saveRequired = true;
    updateEnable();
    ui->widgetSimulation->update();
}

void MainWindow::menuResetView()
{
    ui->doubleSpinBoxTrackingOffset->setValue(Preferences::valueDouble("ResetTrackingOffset"));

    ui->widgetSimulation->setCameraDistance(float(Preferences::valueDouble("ResetCameraDistance")));
    ui->widgetSimulation->setFOV(float(Preferences::valueDouble("ResetCameraFoV")));
    ui->widgetSimulation->setCameraVecX(float(Preferences::valueDouble("ResetCameraVecX")));
    ui->widgetSimulation->setCameraVecY(float(Preferences::valueDouble("ResetCameraVecY")));
    ui->widgetSimulation->setCameraVecZ(float(Preferences::valueDouble("ResetCameraVecZ")));
    ui->widgetSimulation->setCOIx(float(Preferences::valueDouble("ResetCameraCOIX")));
    ui->widgetSimulation->setCOIy(float(Preferences::valueDouble("ResetCameraCOIY")));
    ui->widgetSimulation->setCOIz(float(Preferences::valueDouble("ResetCameraCOIZ")));
    ui->widgetSimulation->setUpX(float(Preferences::valueDouble("ResetCameraUpX")));
    ui->widgetSimulation->setUpY(float(Preferences::valueDouble("ResetCameraUpY")));
    ui->widgetSimulation->setUpZ(float(Preferences::valueDouble("ResetCameraUpZ")));
    ui->widgetSimulation->setBackClip(float(Preferences::valueDouble("ResetCameraBackClip")));
    ui->widgetSimulation->setFrontClip(float(Preferences::valueDouble("ResetCameraFrontClip")));

    ui->widgetSimulation->setCursor3DPosition(QVector3D(0, 0, 0));
    ui->widgetSimulation->setCursorRadius(float(Preferences::valueDouble("ResetCursorRadius")));
    ui->widgetSimulation->setAxesScale(float(Preferences::valueDouble("ResetAxesSize")));

    ui->doubleSpinBoxDistance->setValue(Preferences::valueDouble("ResetCameraDistance"));
    ui->doubleSpinBoxFoV->setValue(Preferences::valueDouble("ResetCameraFoV"));
    ui->doubleSpinBoxCOIX->setValue(Preferences::valueDouble("ResetCameraCOIX"));
    ui->doubleSpinBoxCOIY->setValue(Preferences::valueDouble("ResetCameraCOIY"));
    ui->doubleSpinBoxCOIZ->setValue(Preferences::valueDouble("ResetCameraCOIZ"));
    ui->doubleSpinBoxFar->setValue(Preferences::valueDouble("ResetCameraBackClip"));
    ui->doubleSpinBoxNear->setValue(Preferences::valueDouble("ResetCameraFrontClip"));
    ui->doubleSpinBoxTrackingOffset->setValue(Preferences::valueDouble("ResetTrackingOffset"));

    ui->radioButtonTrackingNone->setChecked(true);

    ui->doubleSpinBoxCursorSize->setValue(Preferences::valueDouble("ResetCursorRadius"));

    ui->widgetSimulation->update();
}

void MainWindow::updateComboBoxTrackingBody()
{
    ui->comboBoxTrackingBody->clear();
    if (!m_simulation) return;
    QString currentTrackBody = Preferences::valueQString("TrackBodyID");
    int currentTrackBodyIndex = -1;
    int count = 0;
    for (auto &&bodyIt : *m_simulation->GetBodyList())
    {
        QString currentBody = QString::fromStdString(bodyIt.first);
        ui->comboBoxTrackingBody->addItem(currentBody);
        if (currentBody == currentTrackBody) currentTrackBodyIndex = count;
        count++;
    }
    ui->comboBoxTrackingBody->setCurrentIndex(currentTrackBodyIndex);
}

void MainWindow::menuExportMarkers()
{
    Q_ASSERT_X(m_simulation, "MainWindow::menuExportMarkers", "m_simulation undefined");
    DialogMarkerImportExport dialogMarkerImportExport(this);
    dialogMarkerImportExport.setSimulation(m_simulation);
    std::vector<std::unique_ptr<Marker>> markerList;
    dialogMarkerImportExport.setMarkerList(&markerList);
    int status = dialogMarkerImportExport.exec();
    if (status == QDialog::Accepted)
    {
        if (dialogMarkerImportExport.markerList() == nullptr)
        {
            this->setStatusString(QString("Markers exported."), 1);
        }
        else
        {
            for (size_t i =0; i < markerList.size(); i++)
            {
                std::unique_ptr<Marker> marker = std::move(markerList[i]);
                std::string markerName = marker->name();
                ui->treeWidgetElements->insertMarker(QString().fromStdString(markerName));
                auto markerIt = m_simulation->GetMarkerList()->find(markerName);
                if (markerIt != m_simulation->GetMarkerList()->end()) // replacement marker
                {
                    std::set<NamedObject *> dependentList =*markerIt->second->dependentList();
                    std::map<std::string, std::string> serialiseMap = markerIt->second->serialise();
                    markerIt->second->unserialise(serialiseMap);
                    markerIt->second->setRedraw(true);
                    for (auto &&depIt : dependentList)
                    {
                        serialiseMap = depIt->serialise();
                        depIt->unserialise(serialiseMap);
                        depIt->setRedraw(true);
                        // everything needs a redraw but somethings also need extra work
                        if (dynamic_cast<Strap *>(depIt)) dynamic_cast<Strap *>(depIt)->Calculate();
                    }

                }
                (*m_simulation->GetMarkerList())[markerName] = std::move(marker);
            }
            m_saveRequired = true;
            this->updateEnable();
            ui->widgetSimulation->update();
        }
    }
    else
    {
        this->setStatusString(tr("Marker import/export cancelled."), 2);
    }
}

