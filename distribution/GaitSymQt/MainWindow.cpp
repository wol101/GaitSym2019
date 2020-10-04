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
#include "MainWindowActions.h"
#include "ViewControlWidget.h"
#include "ObjectiveMain.h"
#include "Preferences.h"
#include "Simulation.h"
#include "Body.h"
#include "Marker.h"
#include "Joint.h"
#include "Muscle.h"
#include "Geom.h"
#include "Muscle.h"
#include "Driver.h"

#include "pystring.h"

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
    m_mainWindowActions = new MainWindowActions(this);
    connect(ui->action1280x720, SIGNAL(triggered()), m_mainWindowActions, SLOT(menu1280x720()));
    connect(ui->action1920x1080, SIGNAL(triggered()), m_mainWindowActions, SLOT(menu1920x1080()));
    connect(ui->action640x480, SIGNAL(triggered()), m_mainWindowActions, SLOT(menu640x480()));
    connect(ui->action800x600, SIGNAL(triggered()), m_mainWindowActions, SLOT(menu800x600()));
    connect(ui->actionAboutGaitSymQt, SIGNAL(triggered()), m_mainWindowActions, SLOT(menuAbout()));
    connect(ui->actionClearMeshCache, SIGNAL(triggered()), m_mainWindowActions, SLOT(menuClearMeshCache()));
    connect(ui->actionConstructionMode, SIGNAL(triggered()), m_mainWindowActions, SLOT(enterConstructionMode()));
    connect(ui->actionCopy, SIGNAL(triggered()), m_mainWindowActions, SLOT(copy()));
    connect(ui->actionCreateAssembly, SIGNAL(triggered()), m_mainWindowActions, SLOT(menuCreateAssembly()));
    connect(ui->actionCreateBody, SIGNAL(triggered()), m_mainWindowActions, SLOT(menuCreateBody()));
    connect(ui->actionCreateDriver, SIGNAL(triggered()), m_mainWindowActions, SLOT(menuCreateDriver()));
    connect(ui->actionCreateGeom, SIGNAL(triggered()), m_mainWindowActions, SLOT(menuCreateGeom()));
    connect(ui->actionCreateJoint, SIGNAL(triggered()), m_mainWindowActions, SLOT(menuCreateJoint()));
    connect(ui->actionCreateMarker, SIGNAL(triggered()), m_mainWindowActions, SLOT(menuCreateMarker()));
    connect(ui->actionCreateMuscle, SIGNAL(triggered()), m_mainWindowActions, SLOT(menuCreateMuscle()));
    connect(ui->actionCut, SIGNAL(triggered()), m_mainWindowActions, SLOT(cut()));
    connect(ui->actionDefaultView, SIGNAL(triggered()), m_mainWindowActions, SLOT(menuDefaultView()));
    connect(ui->actionDeleteAssembly, SIGNAL(triggered()), m_mainWindowActions, SLOT(menuDeleteAssembly()));
    connect(ui->actionEditGlobal, SIGNAL(triggered()), m_mainWindowActions, SLOT(menuEditGlobal()));
    connect(ui->actionExportMarkers, SIGNAL(triggered()), m_mainWindowActions, SLOT(menuExportMarkers()));
    connect(ui->actionGeneticAlgorithm, SIGNAL(triggered()), m_mainWindowActions, SLOT(menuGeneticAlgorithm()));
    connect(ui->actionImportMeshesAsBodies, SIGNAL(triggered()), m_mainWindowActions, SLOT(menuImportMeshes()));
    connect(ui->actionImportWarehouse, SIGNAL(triggered()), m_mainWindowActions, SLOT(menuImportWarehouse()));
    connect(ui->actionNew, SIGNAL(triggered()), m_mainWindowActions, SLOT(menuNew()));
    connect(ui->actionNextAscentHillclimbing, SIGNAL(triggered()), m_mainWindowActions, SLOT(menuNextAscentHillclimbing()));
    connect(ui->actionOpen, SIGNAL(triggered()), m_mainWindowActions, SLOT(menuOpen()));
    connect(ui->actionOutput, SIGNAL(triggered()), m_mainWindowActions, SLOT(menuOutputs()));
    connect(ui->actionPaste, SIGNAL(triggered()), m_mainWindowActions, SLOT(paste()));
    connect(ui->actionPreferences, SIGNAL(triggered()), m_mainWindowActions, SLOT(menuPreferences()));
    connect(ui->actionQuit, SIGNAL(triggered()), this, SLOT(close()));
    connect(ui->actionRandomAscentHillclimbing, SIGNAL(triggered()), m_mainWindowActions, SLOT(menuRandomAscentHillclimbing()));
    connect(ui->actionRawXMLEditor, SIGNAL(triggered()), m_mainWindowActions, SLOT(menuRawXMLEditor()));
    connect(ui->actionRecordMovie, SIGNAL(triggered()), m_mainWindowActions, SLOT(menuRecordMovie()));
    connect(ui->actionResetView, SIGNAL(triggered()), m_mainWindowActions, SLOT(menuResetView()));
    connect(ui->actionRestart, SIGNAL(triggered()), m_mainWindowActions, SLOT(menuRestart()));
    connect(ui->actionRun, SIGNAL(triggered()), m_mainWindowActions, SLOT(run()));
    connect(ui->actionRunMode, SIGNAL(triggered()), m_mainWindowActions, SLOT(enterRunMode()));
    connect(ui->actionSave, SIGNAL(triggered()), m_mainWindowActions, SLOT(menuSave()));
    connect(ui->actionSaveAs, SIGNAL(triggered()), m_mainWindowActions, SLOT(menuSaveAs()));
    connect(ui->actionSaveDefaultView, SIGNAL(triggered()), m_mainWindowActions, SLOT(menuSaveDefaultView()));
    connect(ui->actionSaveOBJSnapshot, SIGNAL(triggered()), m_mainWindowActions, SLOT(objSnapshot()));
    connect(ui->actionSelectAll, SIGNAL(triggered()), m_mainWindowActions, SLOT(selectAll()));
    connect(ui->actionSimplexSearch, SIGNAL(triggered()), m_mainWindowActions, SLOT(menuSimplexSearch()));
    connect(ui->actionSimulatedAnnealing, SIGNAL(triggered()), m_mainWindowActions, SLOT(menuSimulatedAnnealing()));
    connect(ui->actionSnapshot, SIGNAL(triggered()), m_mainWindowActions, SLOT(snapshot()));
    connect(ui->actionStartOBJSequence, SIGNAL(triggered()), m_mainWindowActions, SLOT(menuStartOBJSequenceSave()));
    connect(ui->actionStartWarehouseExport, SIGNAL(triggered()), m_mainWindowActions, SLOT(menuStartWarehouseExport()));
    connect(ui->actionStep, SIGNAL(triggered()), m_mainWindowActions, SLOT(step()));
    connect(ui->actionStopOBJSequence, SIGNAL(triggered()), m_mainWindowActions, SLOT(menuStopOBJSequenceSave()));
    connect(ui->actionStopWarehouseExport, SIGNAL(triggered()), m_mainWindowActions, SLOT(menuStopWarehouseExport()));
    connect(ui->actionTabuSearch, SIGNAL(triggered()), m_mainWindowActions, SLOT(menuTabuSearch()));
    connect(ui->actionToggleFullscreen, SIGNAL(triggered()), m_mainWindowActions, SLOT(menuToggleFullScreen()));
    connect(ui->actionViewBack, SIGNAL(triggered()), m_mainWindowActions, SLOT(buttonCameraBack()));
    connect(ui->actionViewBottom, SIGNAL(triggered()), m_mainWindowActions, SLOT(buttonCameraBottom()));
    connect(ui->actionViewFront, SIGNAL(triggered()), m_mainWindowActions, SLOT(buttonCameraFront()));
    connect(ui->actionViewLeft, SIGNAL(triggered()), m_mainWindowActions, SLOT(buttonCameraLeft()));
    connect(ui->actionViewRight, SIGNAL(triggered()), m_mainWindowActions, SLOT(buttonCameraRight()));
    connect(ui->actionViewTop, SIGNAL(triggered()), m_mainWindowActions, SLOT(buttonCameraTop()));
    connect(ui->radioButtonTrackingNone, SIGNAL(clicked()), this, SLOT(radioButtonTracking()));
    connect(ui->radioButtonTrackingX, SIGNAL(clicked()), this, SLOT(radioButtonTracking()));
    connect(ui->radioButtonTrackingY, SIGNAL(clicked()), this, SLOT(radioButtonTracking()));
    connect(ui->radioButtonTrackingZ, SIGNAL(clicked()), this, SLOT(radioButtonTracking()));
    connect(ui->comboBoxMeshDisplay, SIGNAL(currentTextChanged(const QString &)), this, SLOT(comboBoxMeshDisplayMapCurrentTextChanged(const QString &)));
    connect(ui->comboBoxMuscleColourMap, SIGNAL(currentTextChanged(const QString &)), this, SLOT(comboBoxMuscleColourMapCurrentTextChanged(const QString &)));
    connect(ui->comboBoxTrackingMarker, SIGNAL(currentTextChanged(const QString &)), this, SLOT(comboBoxTrackingMarkerCurrentTextChanged(const QString &)));
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
    connect(ui->treeWidgetElements, SIGNAL(createNewBody()), m_mainWindowActions, SLOT(menuCreateBody()));
    connect(ui->treeWidgetElements, SIGNAL(createNewDriver()), m_mainWindowActions, SLOT(menuCreateDriver()));
    connect(ui->treeWidgetElements, SIGNAL(createNewGeom()), m_mainWindowActions, SLOT(menuCreateGeom()));
    connect(ui->treeWidgetElements, SIGNAL(createNewJoint()), m_mainWindowActions, SLOT(menuCreateJoint()));
    connect(ui->treeWidgetElements, SIGNAL(createNewMarker()), m_mainWindowActions, SLOT(menuCreateMarker()));
    connect(ui->treeWidgetElements, SIGNAL(createNewMuscle()), m_mainWindowActions, SLOT(menuCreateMuscle()));
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
    QObject::connect(ui->widgetSimulation, SIGNAL(EmitCreateMarkerRequest()), m_mainWindowActions, SLOT(menuCreateMarker()));
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
    m_mainWindowActions->menuDefaultView();
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



void MainWindow::processOneThing()
{
    if (m_simulation)
    {
        if (m_simulation->ShouldQuit() || m_simulation->TestForCatastrophy())
        {
            setStatusString(tr("Unable to start simulation"), 1);
            ui->actionRun->setChecked(false);
            m_mainWindowActions->run();
            return;
        }

        m_simulation->UpdateSimulation();
        m_stepCount++;

        if ((m_stepCount % size_t(Preferences::valueInt("MovieSkip"))) == 0)
        {
            handleTracking();
            if (m_stepFlag)
            {
                m_stepFlag = false;
                m_timer->stop();
            }
            ui->widgetSimulation->getDrawMuscleMap()->clear(); // force a redraw of all muscles
            ui->widgetSimulation->getDrawFluidSacMap()->clear(); // force a redraw of all fluid sacs
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
            m_mainWindowActions->run();
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
            m_mainWindowActions->run();
            return;
        }
    }
    updateEnable();
}

void MainWindow::handleCommandLineArguments()
{
    QStringList arguments = QCoreApplication::arguments();
    if (arguments.size() >= 2)
    {
        QFileInfo fileInfo(arguments.at(1));
        if (fileInfo.isFile())
            m_mainWindowActions->menuOpen(arguments.at(1), nullptr);
    }
}

void MainWindow::handleTracking()
{
    if (!m_simulation) return;
    Marker *marker = m_simulation->GetMarker(ui->comboBoxTrackingMarker->currentText().toStdString());
    if (marker)
    {
        pgd::Vector3 position = marker->GetWorldPosition();
        if (ui->radioButtonTrackingX->isChecked())
        {
            ui->widgetSimulation->setCOIx(float(position.x + ui->doubleSpinBoxTrackingOffset->value()));
            ui->doubleSpinBoxCOIX->setValue(position.x + ui->doubleSpinBoxTrackingOffset->value());
        }
        if (ui->radioButtonTrackingY->isChecked())
        {
            ui->widgetSimulation->setCOIy(float(position.y + ui->doubleSpinBoxTrackingOffset->value()));
            ui->doubleSpinBoxCOIY->setValue(position.y + ui->doubleSpinBoxTrackingOffset->value());
        }
        if (ui->radioButtonTrackingZ->isChecked())
        {
            ui->widgetSimulation->setCOIz(float(position.z + ui->doubleSpinBoxTrackingOffset->value()));
            ui->doubleSpinBoxCOIZ->setValue(position.z + ui->doubleSpinBoxTrackingOffset->value());
        }
        ui->widgetSimulation->update();
    }
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
    handleTracking();
}

void MainWindow::radioButtonTracking()
{
    Preferences::insert("TrackingFlagNone", ui->radioButtonTrackingNone->isChecked());
    Preferences::insert("TrackingFlagX", ui->radioButtonTrackingX->isChecked());
    Preferences::insert("TrackingFlagY", ui->radioButtonTrackingY->isChecked());
    Preferences::insert("TrackingFlagZ", ui->radioButtonTrackingZ->isChecked());
    handleTracking();
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


void MainWindow::log(const QString &text)
{
    ui->textEditLog->append(text);
}


QByteArray MainWindow::readResource(const QString &resource)
{
    QFile file(resource);
    bool ok = file.open(QIODevice::ReadOnly);
    Q_ASSERT_X(ok, "MainWindow::readResource", "resource not found");
    return file.readAll();
}


void MainWindow::updateEnable()
{
    ui->actionOutput->setEnabled(m_simulation != nullptr);
    ui->actionRestart->setEnabled(m_simulation != nullptr && m_mode == runMode && m_noName == false && m_saveRequired == false);
    ui->actionSave->setEnabled(m_simulation != nullptr && m_noName == false && m_saveRequired == true);
    ui->actionSaveAs->setEnabled(m_simulation != nullptr);
    ui->actionRawXMLEditor->setEnabled(m_simulation != nullptr);
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
    ui->actionImportMeshesAsBodies->setEnabled(m_simulation != nullptr && m_mode == constructionMode);
    ui->actionCreateBody->setEnabled(m_simulation != nullptr && m_mode == constructionMode);
    ui->actionCreateMarker->setEnabled(m_simulation != nullptr && m_mode == constructionMode && m_simulation->GetBodyList()->size() > 0);
    ui->actionCreateJoint->setEnabled(m_simulation != nullptr && m_mode == constructionMode && m_simulation->GetBodyList()->size() > 1 && m_simulation->GetMarkerList()->size() > 0);
    ui->actionCreateMuscle->setEnabled(m_simulation != nullptr && m_mode == constructionMode && m_simulation->GetBodyList()->size() > 1 && m_simulation->GetMarkerList()->size() > 0);
    ui->actionCreateGeom->setEnabled(m_simulation != nullptr && m_mode == constructionMode && m_simulation->GetBodyList()->size() > 0 && m_simulation->GetMarkerList()->size() > 0);
    ui->actionCreateDriver->setEnabled(m_simulation != nullptr && m_mode == constructionMode && (m_simulation->GetMuscleList()->size() > 0 || m_simulation->GetControllerList()->size() > 0));
    ui->actionEditGlobal->setEnabled(m_simulation != nullptr && m_mode == constructionMode);
    ui->actionCreateAssembly->setEnabled(m_simulation != nullptr && m_simulation->GetBodyList()->size() > 0);
    ui->actionDeleteAssembly->setEnabled(m_simulation != nullptr && m_simulation->HasAssembly());
    ui->actionConstructionMode->setEnabled(m_simulation != nullptr && m_mode == runMode && m_stepCount == 0);
    ui->actionRunMode->setEnabled(m_simulation != nullptr && m_mode == constructionMode && m_simulation->GetBodyList()->size() > 0);
}


MainWindow::Mode MainWindow::mode() const
{
    return m_mode;
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
        updateComboBoxTrackingMarker();
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
    m_mainWindowActions->menuCreateEditBody(body);
}

void MainWindow::editExistingMarker(const QString &name)
{
    Marker *marker = m_simulation->GetMarker(name.toStdString());
    m_mainWindowActions->menuCreateEditMarker(marker);
}

void MainWindow::editExistingJoint(const QString &name)
{
    Joint *joint = m_simulation->GetJoint(name.toStdString());
    m_mainWindowActions->menuCreateEditJoint(joint);
}

void MainWindow::editExistingMuscle(const QString &name)
{
    Muscle *muscle = m_simulation->GetMuscle(name.toStdString());
    m_mainWindowActions->menuCreateEditMuscle(muscle);
}

void MainWindow::editExistingGeom(const QString &name)
{
    Geom *geom = m_simulation->GetGeom(name.toStdString());
    m_mainWindowActions->menuCreateEditGeom(geom);
}

void MainWindow::editExistingDriver(const QString &name)
{
    Driver *driver = m_simulation->GetDriver(name.toStdString());
    m_mainWindowActions->menuCreateEditDriver(driver);
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

void MainWindow::comboBoxTrackingMarkerCurrentTextChanged(const QString &text)
{
    if (m_simulation)
    {
        Marker *marker = m_simulation->GetMarker(text.toStdString());
        if (marker) Preferences::insert("TrackMarkerID", text);
    }
}


void MainWindow::handleElementTreeWidgetItemChanged(QTreeWidgetItem * /* item */, int column)
{
    if (column == 1) ui->widgetSimulation->update();
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

void MainWindow::updateComboBoxTrackingMarker()
{
    const QSignalBlocker blocker(ui->comboBoxTrackingMarker);
    ui->comboBoxTrackingMarker->clear();
    if (!m_simulation) return;
    QString currentTrackMarker = Preferences::valueQString("TrackMarkerID");
    int currentTrackMarkerIndex = -1;
    int count = 0;
    for (auto &&markerIt : *m_simulation->GetMarkerList())
    {
        QString currentMarker = QString::fromStdString(markerIt.first);
        ui->comboBoxTrackingMarker->addItem(currentMarker);
        if (currentMarker == currentTrackMarker) currentTrackMarkerIndex = count;
        count++;
    }
    ui->comboBoxTrackingMarker->setCurrentIndex(currentTrackMarkerIndex);
}

