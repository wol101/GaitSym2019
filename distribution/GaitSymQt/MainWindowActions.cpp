/*
 *  MainWindowActions.cpp
 *  GaitSymODE2019
 *
 *  Created by Bill Sellers on 31/08/2020.
 *  Copyright 2020 Bill Sellers. All rights reserved.
 *
 */

#include "MainWindowActions.h"
#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "AboutDialog.h"
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
#include "DialogPreferences.h"
#include "DialogOutputSelect.h"
#include "DialogBodyBuilder.h"
#include "DialogGlobal.h"
#include "DialogMarkers.h"
#include "DialogJoints.h"
#include "DialogMuscles.h"
#include "DialogGeoms.h"
#include "DialogDrivers.h"
#include "DialogAssembly.h"
#include "DialogMarkerImportExport.h"
#include "DialogCreateMirrorElements.h"
#include "DialogCreateTestingDrivers.h"
#include "DialogRename.h"
#include "DialogInfo.h"
#include "Colour.h"
#include "AMotorJoint.h"
#include "LMotorJoint.h"
#include "TegotaeDriver.h"
#include "TextEditDialog.h"
#include "ThreeHingeJointDriver.h"
#include "TwoHingeJointDriver.h"

#include "pystring.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QTimer>
#include <QMessageBox>
#include <QRegularExpression>

using namespace std::literals::string_literals;

MainWindowActions::MainWindowActions(QObject *parent) : QObject(parent)
{
    m_mainWindow = dynamic_cast<MainWindow *>(parent);
    Q_ASSERT_X(m_mainWindow, "MainWindowActions::MainWindowActions", "parent not set to a valid MainWindow");
}

void MainWindowActions::menuOpen()
{
    if (m_mainWindow->isWindowModified())
    {
        int ret = QMessageBox::warning(m_mainWindow, tr("Current document contains unsaved changes"), tr("Opening a new document will delete the current document.\nAre you sure you want to continue?"),
                                       QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel);
        if (ret == QMessageBox::Cancel) return;
    }

    QFileInfo info(Preferences::valueQString("LastFileOpened"));
    QString fileName;

    fileName = QFileDialog::getOpenFileName(m_mainWindow, tr("Open Config File"), info.absoluteFilePath(), tr("Config Files (*.gaitsym *.xml);;Any File (*.* *)"), nullptr);
    if (fileName.isNull() == false)
    {
        Preferences::insert("LastFileOpened", fileName);
        menuOpen(fileName, nullptr);
    }
}

void MainWindowActions::menuOpen(const QString &fileName, const QByteArray *fileData)
{
    // dispose any simulation cleanly
    m_mainWindow->m_timer->stop();
    m_mainWindow->ui->actionRun->setChecked(false);
    m_mainWindow->m_movieFlag = false;
    if (m_mainWindow->ui->widgetSimulation->aviWriter()) menuStopAVISave();
    if (m_mainWindow->m_simulation)
    {
        delete m_mainWindow->m_simulation;
        m_mainWindow->m_simulation = nullptr;
        m_mainWindow->ui->widgetSimulation->setSimulation(m_mainWindow->m_simulation);
    }
    m_mainWindow->m_stepCount = 0;
    m_mainWindow->m_stepFlag = false;

    m_mainWindow->m_configFile.setFile(fileName);
    QDir::setCurrent(m_mainWindow->m_configFile.absolutePath());
    QString canonicalFilePath = m_mainWindow->m_configFile.canonicalFilePath();
    QDir currentDir(m_mainWindow->m_configFile.absolutePath());
    Preferences::insert("LastFileOpened", canonicalFilePath);

    m_mainWindow->setStatusString(fileName + QString(" loading"), 2);
    qApp->processEvents();

    std::string *errorMessage = nullptr;
    if (fileData)
    {
        m_mainWindow->m_simulation = new Simulation();
        errorMessage = m_mainWindow->m_simulation->LoadModel(fileData->constData(), fileData->size());
    }
    else
    {
        DataFile file;
        int err;
        err = file.ReadFile(canonicalFilePath.toStdString());
        if (err)
        {
            m_mainWindow->setStatusString(QString("Error reading ") + canonicalFilePath, 0);
            m_mainWindow->updateEnable();
            return;
        }
        m_mainWindow->m_simulation = new Simulation();
        errorMessage = m_mainWindow->m_simulation->LoadModel(file.GetRawData(), file.GetSize());
    }
    if (errorMessage)
    {
        m_mainWindow->setStatusString(QString::fromStdString(*errorMessage), 0);
        delete m_mainWindow->m_simulation;
        m_mainWindow->m_simulation = nullptr;
        m_mainWindow->ui->widgetSimulation->setSimulation(m_mainWindow->m_simulation);
        m_mainWindow->ui->widgetSimulation->update();
        m_mainWindow->updateEnable();
        return;
    }
    m_mainWindow->ui->treeWidgetElements->fillVisibitilityLists(m_mainWindow->m_simulation);

    // check we can find the meshes
    QStringList searchPath;
    for (size_t i = 0; i < m_mainWindow->m_simulation->GetGlobal()->MeshSearchPath()->size(); i++)
        searchPath.append(QString::fromStdString(m_mainWindow->m_simulation->GetGlobal()->MeshSearchPath()->at(i)));
    bool noToAll = false;
    bool meshPathChanged = false;
    for (auto &&iter : *m_mainWindow->m_simulation->GetBodyList())
    {
        std::vector<std::string> meshNames = {iter.second->GetGraphicFile1(), iter.second->GetGraphicFile2(), iter.second->GetGraphicFile3()};
        for (auto &&meshName : meshNames)
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
                int ret = QMessageBox::warning(m_mainWindow, QString("Loading ") + canonicalFilePath,
                                               QString("Unable to find \"") + graphicsFile + QString("\"\nDo you want to load a new file?\nThis will alter the Mesh Path and the file will require saving to make this change permanent."),
                                               QMessageBox::Yes | QMessageBox::No | QMessageBox::NoToAll, QMessageBox::Yes);
                if (ret == QMessageBox::NoToAll) noToAll = true;
                if (ret == QMessageBox::Yes)
                {
                    QString newFile = QFileDialog::getOpenFileName(m_mainWindow, tr("Open Mesh File"), graphicsFile, tr("Mesh Files (*.obj *.ply)"), nullptr);
                    if (newFile.isNull() == false)
                    {
                        QFileInfo newFileInfo(newFile);
                        iter.second->SetGraphicFile1(newFileInfo.fileName().toStdString());
                        QString newPath = currentDir.relativeFilePath(newFileInfo.absolutePath());
                        if (newPath.size() == 0) newPath = ".";
                        searchPath.append(newPath);
                        meshPathChanged = true;
                    }
                }
            }
            if (noToAll) break;
        }
        if (noToAll) break;
    }
    if (meshPathChanged)
    {
        m_mainWindow->m_simulation->GetGlobal()->MeshSearchPath()->clear();
        for (int i = 0; i < searchPath.size(); i++) m_mainWindow->m_simulation->GetGlobal()->MeshSearchPath()->push_back(searchPath[i].toStdString());
    }

    m_mainWindow->ui->widgetSimulation->setAxesScale(float(m_mainWindow->m_simulation->GetGlobal()->size1()));
    QString backgroundColour = QString::fromStdString(m_mainWindow->m_simulation->GetGlobal()->colour1().GetHexArgb());
    m_mainWindow->ui->widgetSimulation->setSimulation(m_mainWindow->m_simulation);
    m_mainWindow->ui->widgetSimulation->setBackgroundColour(QColor(backgroundColour));
    m_mainWindow->ui->widgetSimulation->update();
    // m_mainWindow->m_simulation->setDrawContactForces(Preferences::valueBool("DisplayContactForces"));
    //  m_mainWindow->m_simulation->Draw(m_mainWindow->ui->widgetSimulation);
    m_mainWindow->radioButtonTracking();

    m_mainWindow->ui->doubleSpinBoxTimeMax->setValue(m_mainWindow->m_simulation->GetTimeLimit());
    QString time = QString("%1").arg(double(0), 0, 'f', 5);
    m_mainWindow->ui->lcdNumberTime->display(time);

    m_mainWindow->setStatusString(fileName + QString(" loaded"), 1);

    m_mainWindow->comboBoxMuscleColourMapCurrentTextChanged(m_mainWindow->ui->comboBoxMuscleColourMap->currentText());
    m_mainWindow->comboBoxMeshDisplayMapCurrentTextChanged(m_mainWindow->ui->comboBoxMeshDisplay->currentText());

    m_mainWindow->updateComboBoxTrackingMarker();
    m_mainWindow->handleTracking();

    // put the filename as a title
    //if (canonicalFilePath.size() <= 256) m_mainWindow->setWindowTitle(canonicalFilePath + "[*]");
    //else m_mainWindow->setWindowTitle(QString("...") + canonicalFilePath.right(256) + "[*]");
    m_mainWindow->setWindowTitle(canonicalFilePath + "[*]");

    // set menu activations for loaded model
    m_mainWindow->m_noName = false;
    if (meshPathChanged) m_mainWindow->setWindowModified(true);
    else  m_mainWindow->setWindowModified(false);
    enterRunMode();
    m_mainWindow->updateEnable();
}

void MainWindowActions::menuRestart()
{
    menuOpen(m_mainWindow->m_configFile.absoluteFilePath(), nullptr);
}

void MainWindowActions::menuSaveAs()
{
    QString fileName;
    if (m_mainWindow->m_configFile.absoluteFilePath().isEmpty())
    {
        QFileInfo info(Preferences::valueQString("LastFileOpened"));
        fileName = QFileDialog::getSaveFileName(m_mainWindow, tr("Save Model State File "), info.absoluteFilePath(), tr("Config Files (*.gaitsym);;XML files (*.xml)"), nullptr);
    }
    else
    {
        fileName = QFileDialog::getSaveFileName(m_mainWindow, tr("Save Model State File"), m_mainWindow->m_configFile.absoluteFilePath(), tr("Config Files (*.gaitsym);;XML files (*.xml)"), nullptr);
    }

    if (fileName.isNull() == false)
    {
        if (m_mainWindow->m_mode == MainWindow::constructionMode) // need to put everything into run mode to save properly
        {
            for (auto &&it : *m_mainWindow->m_simulation->GetBodyList()) it.second->EnterRunMode();
            for (auto &&it : *m_mainWindow->m_simulation->GetMuscleList()) it.second->LateInitialisation();
            for (auto &&it : *m_mainWindow->m_simulation->GetFluidSacList()) it.second->LateInitialisation();
            for (auto &&it : *m_mainWindow->m_simulation->GetJointList()) it.second->LateInitialisation();
        }
        m_mainWindow->setStatusString(fileName + QString(" saving"), 2);
        m_mainWindow->m_configFile.setFile(fileName);
        QDir currentDir(m_mainWindow->m_configFile.absolutePath());
        QString meshPath, relativeMeshPath;
        for (auto &&it : *m_mainWindow->m_simulation->GetBodyList())
        {
            meshPath = QString::fromStdString(it.second->GetGraphicFile1());
            relativeMeshPath = currentDir.relativeFilePath(meshPath);
            it.second->SetGraphicFile1(relativeMeshPath.toStdString());
            meshPath = QString::fromStdString(it.second->GetGraphicFile2());
            relativeMeshPath = currentDir.relativeFilePath(meshPath);
            it.second->SetGraphicFile2(relativeMeshPath.toStdString());
            meshPath = QString::fromStdString(it.second->GetGraphicFile3());
            relativeMeshPath = currentDir.relativeFilePath(meshPath);
            it.second->SetGraphicFile3(relativeMeshPath.toStdString());
        }
        m_mainWindow->m_simulation->SetOutputModelStateFile(fileName.toStdString());
        m_mainWindow->m_simulation->OutputProgramState();
        m_mainWindow->setStatusString(fileName + QString(" saved"), 1);
        QDir::setCurrent(m_mainWindow->m_configFile.absolutePath());
        Preferences::insert("LastFileOpened", m_mainWindow->m_configFile.canonicalFilePath());
        Preferences::Write();
        // if (fileName.size() <= 256) m_mainWindow->setWindowTitle(fileName + "[*]");
        // else m_mainWindow->setWindowTitle(QString("...") + fileName.right(256) + "[*]");
        m_mainWindow->setWindowTitle(m_mainWindow->m_configFile.canonicalFilePath() + "[*]");
        m_mainWindow->m_noName = false;
        m_mainWindow->setWindowModified(false);
        if (m_mainWindow->m_mode == MainWindow::constructionMode)
        {
            for (auto &&it : *m_mainWindow->m_simulation->GetBodyList()) it.second->EnterConstructionMode();
            for (auto &&it : *m_mainWindow->m_simulation->GetMuscleList()) it.second->LateInitialisation();
            for (auto &&it : *m_mainWindow->m_simulation->GetFluidSacList()) it.second->LateInitialisation();
        }
        m_mainWindow->updateEnable();
    }
    else
    {
        m_mainWindow->ui->statusBar->showMessage(QString("Save As... cancelled"));
    }
}

void MainWindowActions::menuSave()
{
    if (m_mainWindow->m_noName) return;
    if (m_mainWindow->m_mode == MainWindow::constructionMode) // need to put everything into run mode to save properly
    {
        for (auto &&it : *m_mainWindow->m_simulation->GetBodyList()) it.second->EnterRunMode();
        for (auto &&it : *m_mainWindow->m_simulation->GetMuscleList()) it.second->LateInitialisation();
        for (auto &&it : *m_mainWindow->m_simulation->GetFluidSacList()) it.second->LateInitialisation();
        for (auto &&it : *m_mainWindow->m_simulation->GetJointList()) it.second->LateInitialisation();
    }
    QString fileName = m_mainWindow->m_configFile.absoluteFilePath();
    QDir currentDir(m_mainWindow->m_configFile.absolutePath());
    QString meshPath, relativeMeshPath;
    for (auto &&it : *m_mainWindow->m_simulation->GetBodyList())
    {
        meshPath = QString::fromStdString(it.second->GetGraphicFile1());
        relativeMeshPath = currentDir.relativeFilePath(meshPath);
        it.second->SetGraphicFile1(relativeMeshPath.toStdString());
        meshPath = QString::fromStdString(it.second->GetGraphicFile2());
        relativeMeshPath = currentDir.relativeFilePath(meshPath);
        it.second->SetGraphicFile2(relativeMeshPath.toStdString());
        meshPath = QString::fromStdString(it.second->GetGraphicFile3());
        relativeMeshPath = currentDir.relativeFilePath(meshPath);
        it.second->SetGraphicFile3(relativeMeshPath.toStdString());
    }
    m_mainWindow->setStatusString(fileName + QString(" saving"), 2);
    m_mainWindow->m_simulation->SetOutputModelStateFile(fileName.toStdString());
    m_mainWindow->m_simulation->OutputProgramState();
    m_mainWindow->setStatusString(fileName + QString(" saved"), 1);
    m_mainWindow->setWindowModified(false);
    if (m_mainWindow->m_mode == MainWindow::constructionMode)
    {
        for (auto &&it : *m_mainWindow->m_simulation->GetBodyList()) it.second->EnterConstructionMode();
        for (auto &&it : *m_mainWindow->m_simulation->GetMuscleList()) it.second->LateInitialisation();
        for (auto &&it : *m_mainWindow->m_simulation->GetFluidSacList()) it.second->LateInitialisation();
    }
    Preferences::Write();
    m_mainWindow->updateEnable();
}


void MainWindowActions::menuAbout()
{
    AboutDialog aboutDialog(m_mainWindow);

    int status = aboutDialog.exec();

    if (status == QDialog::Accepted)
    {
    }
}

void MainWindowActions::run()
{
    if (m_mainWindow->ui->actionRun->isChecked())
    {
        if (m_mainWindow->m_simulation) m_mainWindow->m_timer->start();
        m_mainWindow->setStatusString(tr("Simulation running"), 1);
    }
    else
    {
        m_mainWindow->m_timer->stop();
        m_mainWindow->setStatusString(tr("Simulation stopped"), 1);
    }
}
void MainWindowActions::step()
{
    m_mainWindow->m_stepFlag = true;
    if (m_mainWindow->m_simulation) m_mainWindow->m_timer->start();
    m_mainWindow->setStatusString(tr("Simulation stepped"), 2);
}

void MainWindowActions::snapshot()
{
    int count = 0;
    QDir dir(m_mainWindow->m_configFile.absolutePath());
    QStringList list = dir.entryList(QDir::Files | QDir::Dirs, QDir::Name);
    QStringList matches = list.filter(QRegularExpression(QString("^Snapshot\\d\\d\\d\\d\\d.*")));
    if (matches.size() > 0)
    {
        QString numberString = matches.last().mid(8, 5);
        count = numberString.toInt() + 1;
    }
    QString filename = dir.absoluteFilePath(QString("Snapshot%1.png").arg(count, 5, 10, QChar('0')));
    if (m_mainWindow->ui->widgetSimulation->WriteStillFrame(filename))
    {
        QMessageBox::warning(m_mainWindow, "Snapshot Error", QString("Could not write '%1'\n").arg(filename));
        return;
    }
    m_mainWindow->setStatusString(QString("\"%1\" saved").arg(filename), 1);
}

void MainWindowActions::objSnapshot()
{
    QFileInfo info(Preferences::valueQString("LastFileOpened"));

    QString folder = QFileDialog::getExistingDirectory(m_mainWindow, tr("Choose folder to save current view as OBJ files"), info.absolutePath());

    if (folder.isNull() == false)
    {
        if (m_mainWindow->ui->widgetSimulation->WriteCADFrame(folder))
        {
            m_mainWindow->setStatusString(QString("Error: Folder '%1' write fail\n").arg(folder), 0);
            return;
        }
        m_mainWindow->setStatusString(QString("Files written in '%1'\n").arg(folder), 1);
    }
}


void MainWindowActions::menuRecordMovie()
{
    if (m_mainWindow->ui->actionRecordMovie->isChecked())
    {
        m_mainWindow->m_movieFlag = true;
        QFileInfo info(Preferences::valueQString("LastFileOpened"));
        QString fileName = QFileDialog::getSaveFileName(m_mainWindow, tr("Save output as AVI file"), info.absolutePath(), tr("AVI Files (*.avi)"), nullptr);
        if (fileName.isNull() == false)
        {
            m_mainWindow->m_movieFlag = true;
            m_mainWindow->ui->widgetSimulation->StartAVISave(fileName);
        }
        else
        {
            m_mainWindow->m_movieFlag = false;
            m_mainWindow->ui->actionRecordMovie->setChecked(false);
        }
    }
    else
    {
        m_mainWindow->m_movieFlag = false;
        if (m_mainWindow->ui->widgetSimulation->aviWriter()) menuStopAVISave();
    }
}

void MainWindowActions::menuPreferences()
{
    DialogPreferences dialogPreferences(m_mainWindow);
    dialogPreferences.initialise();

    int status = dialogPreferences.exec();

    if (status == QDialog::Accepted)   // write the new settings
    {
        dialogPreferences.update();
        m_mainWindow->writeSettings();

        // these settings have immediate effect
        QColor cursorColour = Preferences::valueQColor("CursorColour");
        m_mainWindow->ui->widgetSimulation->setCursorColour(QColor(cursorColour.red(), cursorColour.green(), cursorColour.blue(), cursorColour.alpha()));
        m_mainWindow->ui->widgetSimulation->setCursorRadius(float(Preferences::valueDouble("CursorRadius")));
        m_mainWindow->ui->widgetSimulation->setCursor3DNudge(float(Preferences::valueDouble("CursorNudge")));
        m_mainWindow->ui->widgetSimulation->setFrontClip(float(Preferences::valueDouble("CameraFrontClip")));
        m_mainWindow->ui->widgetSimulation->setBackClip(float(Preferences::valueDouble("CameraBackClip")));

        m_mainWindow->ui->widgetSimulation->update();

    }

}

void MainWindowActions::menuOutputs()
{
    if (m_mainWindow->m_simulation == nullptr) return;
    DialogOutputSelect dialogOutputSelect(m_mainWindow);
    dialogOutputSelect.setSimulation(m_mainWindow->m_simulation);
    int status = dialogOutputSelect.exec();
    if (status == QDialog::Accepted)
    {
        m_mainWindow->ui->treeWidgetElements->fillVisibitilityLists(m_mainWindow->m_simulation);
        m_mainWindow->setStatusString(tr("Outputs set"), 1);
    }
    else
    {
        m_mainWindow->setStatusString(tr("Outputs setting cancelled"), 1);
    }
}

void MainWindowActions::menuLoadDefaultView()
{
    m_mainWindow->ui->doubleSpinBoxTrackingOffset->setValue(Preferences::valueDouble("DefaultTrackingOffset"));

    m_mainWindow->ui->widgetSimulation->setCameraDistance(float(Preferences::valueDouble("DefaultCameraDistance")));
    m_mainWindow->ui->widgetSimulation->setFOV(float(Preferences::valueDouble("DefaultCameraFoV")));
    m_mainWindow->ui->widgetSimulation->setCameraVecX(float(Preferences::valueDouble("DefaultCameraVecX")));
    m_mainWindow->ui->widgetSimulation->setCameraVecY(float(Preferences::valueDouble("DefaultCameraVecY")));
    m_mainWindow->ui->widgetSimulation->setCameraVecZ(float(Preferences::valueDouble("DefaultCameraVecZ")));
    m_mainWindow->ui->widgetSimulation->setCOIx(float(Preferences::valueDouble("DefaultCameraCOIX")));
    m_mainWindow->ui->widgetSimulation->setCOIy(float(Preferences::valueDouble("DefaultCameraCOIY")));
    m_mainWindow->ui->widgetSimulation->setCOIz(float(Preferences::valueDouble("DefaultCameraCOIZ")));
    m_mainWindow->ui->widgetSimulation->setUpX(float(Preferences::valueDouble("DefaultCameraUpX")));
    m_mainWindow->ui->widgetSimulation->setUpY(float(Preferences::valueDouble("DefaultCameraUpY")));
    m_mainWindow->ui->widgetSimulation->setUpZ(float(Preferences::valueDouble("DefaultCameraUpZ")));
    m_mainWindow->ui->widgetSimulation->setBackClip(float(Preferences::valueDouble("DefaultCameraBackClip")));
    m_mainWindow->ui->widgetSimulation->setFrontClip(float(Preferences::valueDouble("DefaultCameraFrontClip")));

    m_mainWindow->ui->widgetSimulation->update();
}

void MainWindowActions::menuSaveDefaultView()
{
    Preferences::insert("DefaultTrackingOffset", m_mainWindow->ui->doubleSpinBoxTrackingOffset->value());

    Preferences::insert("DefaultCameraDistance", m_mainWindow->ui->widgetSimulation->cameraDistance());
    Preferences::insert("DefaultCameraFoV", m_mainWindow->ui->widgetSimulation->FOV());
    Preferences::insert("DefaultCameraCOIX", m_mainWindow->ui->widgetSimulation->COIx());
    Preferences::insert("DefaultCameraCOIY", m_mainWindow->ui->widgetSimulation->COIy());
    Preferences::insert("DefaultCameraCOIZ", m_mainWindow->ui->widgetSimulation->COIz());
    Preferences::insert("DefaultCameraVecX", m_mainWindow->ui->widgetSimulation->cameraVecX());
    Preferences::insert("DefaultCameraVecY", m_mainWindow->ui->widgetSimulation->cameraVecY());
    Preferences::insert("DefaultCameraVecZ", m_mainWindow->ui->widgetSimulation->cameraVecZ());
    Preferences::insert("DefaultCameraUpX", m_mainWindow->ui->widgetSimulation->upX());
    Preferences::insert("DefaultCameraUpY", m_mainWindow->ui->widgetSimulation->upY());
    Preferences::insert("DefaultCameraUpZ", m_mainWindow->ui->widgetSimulation->upZ());
    Preferences::insert("DefaultCameraBackClip", m_mainWindow->ui->widgetSimulation->backClip());
    Preferences::insert("DefaultCameraFrontClip", m_mainWindow->ui->widgetSimulation->frontClip());
    Preferences::Write();
}

void MainWindowActions::menu640x480()
{
    m_mainWindow->resizeSimulationWindow(640, 480);
}

void MainWindowActions::menu800x600()
{
    m_mainWindow->resizeSimulationWindow(800, 600);
}

void MainWindowActions::menu1280x720()
{
    m_mainWindow->resizeSimulationWindow(1280, 720);
}

void MainWindowActions::menu1920x1080()
{
    m_mainWindow->resizeSimulationWindow(1920, 1080);
}

void MainWindowActions::buttonCameraRight()
{
    m_mainWindow->ui->widgetSimulation->setCameraVecX(0);
    m_mainWindow->ui->widgetSimulation->setCameraVecY(1);
    m_mainWindow->ui->widgetSimulation->setCameraVecZ(0);
    m_mainWindow->ui->widgetSimulation->setUpX(0);
    m_mainWindow->ui->widgetSimulation->setUpY(0);
    m_mainWindow->ui->widgetSimulation->setUpZ(1);
    m_mainWindow->ui->widgetSimulation->update();
}


void MainWindowActions::buttonCameraTop()
{
    m_mainWindow->ui->widgetSimulation->setCameraVecX(0);
    m_mainWindow->ui->widgetSimulation->setCameraVecY(0);
    m_mainWindow->ui->widgetSimulation->setCameraVecZ(-1);
    m_mainWindow->ui->widgetSimulation->setUpX(0);
    m_mainWindow->ui->widgetSimulation->setUpY(1);
    m_mainWindow->ui->widgetSimulation->setUpZ(0);
    m_mainWindow->ui->widgetSimulation->update();
}


void MainWindowActions::buttonCameraFront()
{
    m_mainWindow->ui->widgetSimulation->setCameraVecX(-1);
    m_mainWindow->ui->widgetSimulation->setCameraVecY(0);
    m_mainWindow->ui->widgetSimulation->setCameraVecZ(0);
    m_mainWindow->ui->widgetSimulation->setUpX(0);
    m_mainWindow->ui->widgetSimulation->setUpY(0);
    m_mainWindow->ui->widgetSimulation->setUpZ(1);
    m_mainWindow->ui->widgetSimulation->update();
}


void MainWindowActions::buttonCameraLeft()
{
    m_mainWindow->ui->widgetSimulation->setCameraVecX(0);
    m_mainWindow->ui->widgetSimulation->setCameraVecY(-1);
    m_mainWindow->ui->widgetSimulation->setCameraVecZ(0);
    m_mainWindow->ui->widgetSimulation->setUpX(0);
    m_mainWindow->ui->widgetSimulation->setUpY(0);
    m_mainWindow->ui->widgetSimulation->setUpZ(1);
    m_mainWindow->ui->widgetSimulation->update();
}


void MainWindowActions::buttonCameraBottom()
{
    m_mainWindow->ui->widgetSimulation->setCameraVecX(0);
    m_mainWindow->ui->widgetSimulation->setCameraVecY(0);
    m_mainWindow->ui->widgetSimulation->setCameraVecZ(1);
    m_mainWindow->ui->widgetSimulation->setUpX(0);
    m_mainWindow->ui->widgetSimulation->setUpY(1);
    m_mainWindow->ui->widgetSimulation->setUpZ(0);
    m_mainWindow->ui->widgetSimulation->update();
}


void MainWindowActions::buttonCameraBack()
{
    m_mainWindow->ui->widgetSimulation->setCameraVecX(1);
    m_mainWindow->ui->widgetSimulation->setCameraVecY(0);
    m_mainWindow->ui->widgetSimulation->setCameraVecZ(0);
    m_mainWindow->ui->widgetSimulation->setUpX(0);
    m_mainWindow->ui->widgetSimulation->setUpY(0);
    m_mainWindow->ui->widgetSimulation->setUpZ(1);
    m_mainWindow->ui->widgetSimulation->update();
}

void MainWindowActions::menuStopAVISave()
{
    m_mainWindow->m_movieFlag = false;
    m_mainWindow->ui->widgetSimulation->StopAVISave();
}

void MainWindowActions::menuStartOBJSequenceSave()
{
    QFileInfo info(Preferences::valueQString("LastFileOpened"));

    m_mainWindow->m_objFileSequenceFolder = QFileDialog::getExistingDirectory(m_mainWindow, tr("Choose folder to the OBJ file sequence"), info.absolutePath());

    if (m_mainWindow->m_objFileSequenceFolder.isNull() == false)
    {
        m_mainWindow->m_saveOBJFileSequenceFlag = true;
        m_mainWindow->ui->actionStartOBJSequence->setEnabled(false);
        m_mainWindow->ui->actionStopOBJSequence->setEnabled(true);
    }
}

void MainWindowActions::menuStopOBJSequenceSave()
{
    m_mainWindow->m_saveOBJFileSequenceFlag = false;
    m_mainWindow->ui->actionStartOBJSequence->setEnabled(true);
    m_mainWindow->ui->actionStopOBJSequence->setEnabled(false);
}

void MainWindowActions::menuNew()
{
    if (m_mainWindow->isWindowModified())
    {
        QMessageBox msgBox;
        msgBox.setText("The document has been modified.");
        msgBox.setInformativeText("Click OK to create a new document, and Cancel to continue working on the current document");
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

    DialogGlobal dialogGlobal(m_mainWindow);
    dialogGlobal.setExistingBodies(nullptr);
    dialogGlobal.lateInitialise();

    int status = dialogGlobal.exec();

    if (status == QDialog::Accepted)
    {
        if (m_mainWindow->m_simulation) delete m_mainWindow->m_simulation;
        m_mainWindow->m_simulation = nullptr;
        m_mainWindow->ui->widgetSimulation->setSimulation(m_mainWindow->m_simulation);
        m_mainWindow->m_stepCount = 0;
        m_mainWindow->ui->actionRun->setChecked(false);
        m_mainWindow->m_timer->stop();
        m_mainWindow->m_simulation = new Simulation();
        std::unique_ptr<Global> newGlobal = dialogGlobal.outputGlobal();
        newGlobal->setSimulation(m_mainWindow->m_simulation);
        m_mainWindow->m_simulation->SetGlobal(std::move(newGlobal));
        m_mainWindow->ui->widgetSimulation->setSimulation(m_mainWindow->m_simulation);
        m_mainWindow->ui->widgetSimulation->update();
        m_mainWindow->ui->treeWidgetElements->setSimulation(m_mainWindow->m_simulation);
        std::unique_ptr<Marker> marker = std::make_unique<Marker>(nullptr);
        marker->setName("WorldMarker"s);
        auto markersMap = m_mainWindow->m_simulation->GetMarkerList();
        (*markersMap)[marker->name()] = std::move(marker);
        m_mainWindow->ui->treeWidgetElements->fillVisibitilityLists(m_mainWindow->m_simulation);
        m_mainWindow->updateComboBoxTrackingMarker();
        m_mainWindow->m_noName = true;
        m_mainWindow->setWindowModified(false);
        enterConstructionMode();
        m_mainWindow->updateEnable();
        m_mainWindow->setStatusString(tr("New document created"), 1);
    }
    else
    {
        m_mainWindow->setStatusString(tr("New document cancelled"), 1);
    }
}

void MainWindowActions::menuStartWarehouseExport()
{
    if (m_mainWindow->m_simulation == nullptr) return;

    QFileInfo info(Preferences::valueQString("LastFileOpened"));
    QString fileName = QFileDialog::getSaveFileName(m_mainWindow, tr("Save output as Warehouse file"), info.absolutePath(), tr("Text Files (*.txt)"), nullptr);

    if (fileName.isNull() == false)
    {
        m_mainWindow->ui->actionStartWarehouseExport->setEnabled(false);
        m_mainWindow->ui->actionStopWarehouseExport->setEnabled(true);
        m_mainWindow->m_simulation->SetOutputWarehouseFile(fileName.toStdString());
    }
}

void MainWindowActions::menuStopWarehouseExport()
{
    if (m_mainWindow->m_simulation == nullptr) return;

    m_mainWindow->ui->actionStartWarehouseExport->setEnabled(true);
    m_mainWindow->ui->actionStopWarehouseExport->setEnabled(false);
    m_mainWindow->m_simulation->SetOutputWarehouseFile(nullptr);
}

void MainWindowActions::menuImportMeshes()
{
    if (m_mainWindow->m_simulation == nullptr) return;
    std::string lastFolder;
    if (m_mainWindow->m_noName) lastFolder = pystring::os::path::dirname(Preferences::valueQString("LastFileOpened").toStdString());
    else lastFolder = m_mainWindow->m_configFile.absolutePath().toStdString();

    QStringList meshFileNames = QFileDialog::getOpenFileNames(dynamic_cast<QWidget *>(parent()), tr("Select the geometry files required"), QString::fromStdString(lastFolder), "Meshes (*.obj *.ply)");
    std::vector<std::string> errorList;
    if (meshFileNames.size())
    {
        for (auto &&it : meshFileNames)
        {
            // first check that this is a valid mesh
            std::string meshFileName = it.toStdString();
            std::unique_ptr<FacetedObject> mesh = std::make_unique<FacetedObject>();
            if (mesh->ParseMeshFile(meshFileName))
            {
                errorList.push_back("Error parsing "s + meshFileName);
                continue;
            }

            // now create the body
            std::unique_ptr<Body> body = std::make_unique<Body>(m_mainWindow->m_simulation->GetWorldID());
            body->setSimulation(m_mainWindow->m_simulation);
            body->SetConstructionDensity(Preferences::valueDouble("BodyDensity", 1000.0));
            body->SetGraphicFile1(meshFileName);

            // get a unique file name
            auto bodyList = m_mainWindow->m_simulation->GetBodyList();
            auto markerList = m_mainWindow->m_simulation->GetMarkerList();
            std::string ext, suggestedName;
            pystring::os::path::splitext(suggestedName, ext, pystring::os::path::basename(meshFileName));
            std::replace(suggestedName.begin(), suggestedName.end(), ' ', '_');
            std::string suggestedCMMarkerName = suggestedName + "_CM_Marker"s;
            auto suggestedNameIt = bodyList->find(suggestedName);
            auto suggestedMarkerNameIt = markerList->find(suggestedCMMarkerName);
            size_t count = 0;
            while (suggestedNameIt != bodyList->end() || suggestedMarkerNameIt != markerList->end() || count > 999)
            {
                count++;
                std::string newSuggestedName = suggestedName + std::to_string(count);
                suggestedCMMarkerName = newSuggestedName + "_CM_Marker"s;
                suggestedNameIt = bodyList->find(newSuggestedName);
                suggestedMarkerNameIt = markerList->find(suggestedCMMarkerName);
                if (suggestedNameIt == bodyList->end() && suggestedMarkerNameIt == markerList->end())
                {
                    suggestedName = newSuggestedName;
                    break;
                }
            }
            if (count > 999)
            {
                errorList.push_back("Error setting name for "s + meshFileName);
                continue;
            }
            body->setName(suggestedName);

            // and set the mass
            dMass mass = {};
            double density = body->GetConstructionDensity();
            bool clockwise = false;
            pgd::Vector3 translation;
            mesh->CalculateMassProperties(&mass, density, clockwise, translation.data());
            std::string massError = Body::MassCheck(&mass);
            if (massError.size() == 0)
            {
                body->SetConstructionPosition(mass.c[0], mass.c[1], mass.c[2]);
                body->SetPosition(mass.c[0], mass.c[1], mass.c[2]);
                // now recalculate the inertial tensor arount the centre of mass
                translation.Set(-mass.c[0], -mass.c[1], -mass.c[2]);
                mesh->CalculateMassProperties(&mass, density, clockwise, translation.data());
                mass.c[0] = mass.c[1] = mass.c[2] = 0;
            }
            else
            {
                QMessageBox::warning(m_mainWindow, tr("Calculate Mass Properties: %1").arg(meshFileName.c_str()), tr("Calculated mass properties are invalid so using defaults:\n%1").arg(massError.c_str()));
                dMassSetParameters(&mass, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0);
                pgd::Vector3 boundingBoxCentre = (pgd::Vector3(mesh->upperBound()) + pgd::Vector3(mesh->lowerBound())) / 2;
                body->SetConstructionPosition(boundingBoxCentre.x, boundingBoxCentre.y, boundingBoxCentre.z);
                body->SetPosition(boundingBoxCentre.x, boundingBoxCentre.y, boundingBoxCentre.z);
            }
            body->SetMass(&mass);

            // set the default properties
            body->setSize1(Preferences::valueDouble("BodyAxesSize"));
            body->setSize2(Preferences::valueDouble("BodyBlendFraction"));
            body->setColour1(Preferences::valueQColor("BodyColour1").name(QColor::HexArgb).toStdString());
            body->setColour2(Preferences::valueQColor("BodyColour2").name(QColor::HexArgb).toStdString());
            body->setColour3(Preferences::valueQColor("BodyColour3").name(QColor::HexArgb).toStdString());

            // this is needed because there are some parts of Body that do not have a public interface
            body->saveToAttributes();
            body->createFromAttributes();

            // insert the new centre of mass marker
            std::unique_ptr<Marker> cmMarker = std::make_unique<Marker>(body.get());
            cmMarker->setName(suggestedCMMarkerName);
            cmMarker->setSimulation(m_mainWindow->m_simulation);
            cmMarker->setSize1(Preferences::valueDouble("MarkerSize", 0.01));

            // and add to the simulation
            (*bodyList)[suggestedName] = std::move(body);
            (*markerList)[suggestedCMMarkerName] = std::move(cmMarker);
        }
    }
    else
    {
        m_mainWindow->setStatusString("Import meshes as Bodies cancelled", 2);
        return;
    }
    m_mainWindow->setStatusString("Import Meshes as Bodies Successful", 1);
    for (size_t i = 0; i < errorList.size(); i++)
    {
        m_mainWindow->log(QString::fromStdString(errorList[i] + "\n"s));
    }
    m_mainWindow->setWindowModified(true);
    m_mainWindow->updateEnable();
    m_mainWindow->updateComboBoxTrackingMarker();
    m_mainWindow->ui->treeWidgetElements->fillVisibitilityLists(m_mainWindow->m_simulation);
    m_mainWindow->ui->widgetSimulation->update();
}

void MainWindowActions::menuImportWarehouse()
{
    if (m_mainWindow->m_simulation == nullptr) return;
    QString fileName = QFileDialog::getOpenFileName(m_mainWindow, tr("Open Warehouse File"), "", tr("Warehouse Files (*.txt);;Any File (*.* *)"), nullptr);

    if (fileName.isNull() == false)
    {
        m_mainWindow->m_simulation->AddWarehouse(fileName.toStdString());
        m_mainWindow->setStatusString(QString("Warehouse %1 added").arg(fileName), 1);
    }
}

void MainWindowActions::menuToggleFullScreen()
{
    // might want to remember things like whether the screen was maximised or minimised
    if (m_mainWindow->isFullScreen())
    {
        m_mainWindow->ui->menuBar->show();
        m_mainWindow->showNormal();
    }
    else
    {
        m_mainWindow->ui->menuBar->hide();
        m_mainWindow->showFullScreen();
    }
}

void MainWindowActions::menuCreateJoint()
{
    menuCreateEditJoint(nullptr);
}

void MainWindowActions::menuCreateEditJoint(Joint *joint)
{
    Q_ASSERT_X(m_mainWindow->m_simulation, "MainWindowActions::menuCreateJoint", "m_mainWindow->m_simulation undefined");
    Q_ASSERT_X(m_mainWindow->m_simulation->GetBodyList()->size(), "MainWindowActions::menuCreateEditMarker", "No bodies defined");
    DialogJoints dialogJoints(m_mainWindow);
    dialogJoints.setSimulation(m_mainWindow->m_simulation);
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
            m_mainWindow->ui->treeWidgetElements->insertJoint(QString().fromStdString(newJointName), newJoint->visible(), newJoint->dump());
            (*m_mainWindow->m_simulation->GetJointList())[newJointName] = std::move(newJoint);
            m_mainWindow->setStatusString(QString("New joint created: %1").arg(QString::fromStdString(newJointName)), 1);
        }
        else // replacing an existing joint
        {
            std::unique_ptr<Joint> replacementJoint = dialogJoints.outputJoint();
            std::string replacementJointName = replacementJoint->name();
            replacementJoint->LateInitialisation();
            // the only thing that currently depends on joints is the ThreeJointDriver
            for (auto &&driverIt : *m_mainWindow->m_simulation->GetDriverList())
            {
                ThreeHingeJointDriver *threeHingeJointDriver = dynamic_cast<ThreeHingeJointDriver *>(driverIt.second.get());
                if (threeHingeJointDriver) threeHingeJointDriver->saveToAttributes();
            }
            (*m_mainWindow->m_simulation->GetJointList())[replacementJointName] = std::move(replacementJoint);
            for (auto driverIt = m_mainWindow->m_simulation->GetDriverList()->begin(); driverIt != m_mainWindow->m_simulation->GetDriverList()->end(); /* no increment */)
            {
                ThreeHingeJointDriver *threeHingeJointDriver = dynamic_cast<ThreeHingeJointDriver *>(driverIt->second.get());
                if (threeHingeJointDriver)
                {
                    std::string *lastError = threeHingeJointDriver->createFromAttributes();
                    if (lastError)
                    {
                        QMessageBox::warning(m_mainWindow, QString("Error creating ThreeHingeJointDriver \"%1\"").arg(QString::fromStdString(driverIt->first)),
                                             QString("Error message:\n\"%1\"").arg(QString::fromStdString(*lastError)));
                        m_mainWindow->setStatusString(QString("ThreeHingeJointDriver deleted: %1").arg(QString::fromStdString(driverIt->first)), 1);
                        m_mainWindow->deleteExistingDriver(QString::fromStdString(driverIt->first));
                        driverIt = m_mainWindow->m_simulation->GetDriverList()->erase(driverIt);
                    }
                    else { driverIt++; }
                }
                else { driverIt++; }
            }
            m_mainWindow->setStatusString(QString("Joint edited: %1").arg(QString::fromStdString(replacementJointName)), 1);
        }
        m_mainWindow->setWindowModified(true);
        m_mainWindow->updateEnable();
        m_mainWindow->ui->widgetSimulation->update();
    }
    else
    {
        m_mainWindow->setStatusString(tr("Joint creation cancelled"), 2);
    }
}

void MainWindowActions::menuCreateBody()
{
    menuCreateEditBody(nullptr);
}

void MainWindowActions::menuCreateEditBody(Body *body)
{
    Q_ASSERT_X(m_mainWindow->m_simulation, "MainWindow::menuCreateBody", "m_simulation undefined");
    DialogBodyBuilder dialogBodyBuilder(m_mainWindow);
    dialogBodyBuilder.setSimulation(m_mainWindow->m_simulation);
    dialogBodyBuilder.setInputBody(body);
    dialogBodyBuilder.lateInitialise();
    pgd::Vector3 originalContructionPosition;
    pgd::Vector3 originalPosition;
    pgd::Quaternion originalOrientation;
    if (body)
    {
        originalContructionPosition.Set(body->GetConstructionPosition());
        originalPosition.Set(body->GetPosition());
        originalOrientation.Set(body->GetQuaternion());
    }
    int status = dialogBodyBuilder.exec();
    if (status == QDialog::Accepted)
    {
        if (!body) // this is the create body option so there will be no dependencies
        {
            std::unique_ptr<Body> newBody = dialogBodyBuilder.outputBody();
            newBody->LateInitialisation();
            std::string newBodyName = newBody->name();
//            // insert the new centre of mass marker
//            std::unique_ptr<Marker> cmMarker = std::make_unique<Marker>(newBody.get());
//            std::string cmMarkerName = newBodyName + "_CM_Marker"s;
//            cmMarker->setName(cmMarkerName);
//            cmMarker->setSimulation(m_mainWindow->m_simulation);
//            cmMarker->setSize1(Preferences::valueDouble("MarkerSize", 0.01));
//            m_mainWindow->ui->treeWidgetElements->insertMarker(QString().fromStdString(cmMarkerName), cmMarker->visible(), cmMarker->dump());
//            (*m_mainWindow->m_simulation->GetMarkerList())[cmMarkerName] = std::move(cmMarker);
//            m_mainWindow->setStatusString(QString("New marker created: %1").arg(QString::fromStdString(cmMarkerName)), 1);
            // insert the new body
            m_mainWindow->ui->treeWidgetElements->insertBody(QString().fromStdString(newBodyName), newBody->visible(), newBody->dump());
            (*m_mainWindow->m_simulation->GetBodyList())[newBodyName] = std::move(newBody);
            m_mainWindow->setStatusString(QString("New body created: %1").arg(QString::fromStdString(newBodyName)), 1);
            m_mainWindow->updateComboBoxTrackingMarker();
        }
        else // this is an edit so things may have moved and we need to deal with that
        {
            m_mainWindow->setStatusString(QString("Body edited: %1").arg(QString::fromStdString(body->name())), 1);
            pgd::Vector3 deltaPosition = pgd::Vector3(body->GetConstructionPosition()) - originalContructionPosition;
            if (Preferences::valueBool("DialogBodyBuilderMoveMarkers", false) == false) // need to compensate the move in construction position
            {
                for (auto &&it : *m_mainWindow->m_simulation->GetMarkerList())
                {
                    if (it.second->GetBody() == body)
                        it.second->OffsetPosition(-deltaPosition.x, -deltaPosition.y, -deltaPosition.z);
                }
            }
//            // completely reloading everything will work but is rather slow
//            QByteArray xmlData = QByteArray::fromStdString(m_mainWindow->m_simulation->SaveToXML());
//            menuOpen(m_mainWindow->m_configFile.absoluteFilePath(), &xmlData);
//            m_mainWindow->setWindowModified(true);
//            enterConstructionMode();

            body->setRedraw(true);
            std::vector<NamedObject *> objectList = m_mainWindow->m_simulation->GetObjectList();
            for (auto &&it : objectList)
            {

                if (it->isUpstreamObject(body))
                {
                    it->saveToAttributes();
                    it->createFromAttributes();
                    it->setRedraw(true);
                    // everything needs a redraw but somethings also need extra work
                    if (dynamic_cast<Strap *>(it)) dynamic_cast<Strap *>(it)->Calculate();
                }
             }
        }
        m_mainWindow->setWindowModified(true);
        m_mainWindow->updateEnable();
        m_mainWindow->ui->widgetSimulation->update();
    }
    else
    {
        m_mainWindow->setStatusString(tr("Body creation cancelled"), 2);
    }
}

void MainWindowActions::menuCreateMarker()
{
    menuCreateEditMarker(nullptr);
}

void MainWindowActions::menuCreateEditMarker(Marker *marker)
{
    Q_ASSERT_X(m_mainWindow->m_simulation, "MainWindow::menuCreateEditMarker", "m_simulation undefined");
    Q_ASSERT_X(m_mainWindow->m_simulation->GetBodyList()->size(), "MainWindow::menuCreateEditMarker", "No bodies defined");
    DialogMarkers dialogMarkers(m_mainWindow);
    dialogMarkers.setCursor3DPosition(m_mainWindow->ui->widgetSimulation->cursor3DPosition());
    dialogMarkers.setInputMarker(marker);
    dialogMarkers.setSimulation(m_mainWindow->m_simulation);
    dialogMarkers.lateInitialise();
    if (sender() == m_mainWindow->ui->widgetSimulation && m_mainWindow->ui->widgetSimulation->getLastMenuItem() != tr("Edit Marker..."))
    {
        if (m_mainWindow->ui->widgetSimulation->getClosestHit())
        {
            pgd::Vector3 location = m_mainWindow->ui->widgetSimulation->getClosestHit()->worldLocation();
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
            m_mainWindow->ui->treeWidgetElements->insertMarker(QString().fromStdString(newMarkerName), newMarker->visible(), newMarker->dump());
            (*m_mainWindow->m_simulation->GetMarkerList())[newMarkerName] = std::move(newMarker);
            m_mainWindow->setStatusString(QString("New marker created: %1").arg(QString::fromStdString(newMarkerName)), 1);
        }
        else // editing a marker so need to cope with dependencies
        {
            marker->setRedraw(true);
            std::vector<NamedObject *> objectList = m_mainWindow->m_simulation->GetObjectList();
            for (auto &&it : objectList)
            {
                if (it->isUpstreamObject(marker))
                {
                    it->saveToAttributes();
                    it->createFromAttributes();
                    it->setRedraw(true);
                    // everything needs a redraw but somethings also need extra work
                    if (dynamic_cast<Strap *>(it)) dynamic_cast<Strap *>(it)->Calculate();
                }
            }
            m_mainWindow->setStatusString(QString("Marker edited: %1").arg(QString::fromStdString(marker->name())), 1);
        }

        m_mainWindow->setWindowModified(true);
        m_mainWindow->updateEnable();
        m_mainWindow->ui->widgetSimulation->update();
    }
    else
    {
        m_mainWindow->setStatusString(tr("Marker creation cancelled"), 2);
    }
}

void MainWindowActions::menuCreateMuscle()
{
    menuCreateEditMuscle(nullptr);
}

void MainWindowActions::menuCreateEditMuscle(Muscle *muscle)
{
    Q_ASSERT_X(m_mainWindow->m_simulation, "MainWindowActions::menuCreateMuscle", "m_mainWindow->m_simulation undefined");
    Q_ASSERT_X(m_mainWindow->m_simulation->GetBodyList()->size(), "MainWindowActions::menuCreateEditMarker", "No bodies defined");
    DialogMuscles dialogMuscles(m_mainWindow);
    dialogMuscles.setSimulation(m_mainWindow->m_simulation);
    dialogMuscles.setInputMuscle(muscle);
    dialogMuscles.lateInitialise();
    int status = dialogMuscles.exec();
    if (status == QDialog::Accepted)
    {
        if (!muscle) // creating a new muscle
        {
            std::unique_ptr<Strap> newStrap = dialogMuscles.outputStrap();
            std::string newStrapName = newStrap->name();
            (*m_mainWindow->m_simulation->GetStrapList())[newStrapName] = std::move(newStrap);
            std::unique_ptr<Muscle> newMuscle = dialogMuscles.outputMuscle();
            muscle = newMuscle.get();
            std::string newMuscleName = newMuscle->name();
            m_mainWindow->ui->treeWidgetElements->insertMuscle(QString().fromStdString(newMuscleName), newMuscle->visible(), newMuscle->dump());
            (*m_mainWindow->m_simulation->GetMuscleList())[newMuscleName] = std::move(newMuscle);
            m_mainWindow->setStatusString(QString("New muscle created: %1").arg(QString::fromStdString(newMuscleName)), 1);
        }
        else // replacing an existing muscle
        {
            std::unique_ptr<Strap> replacementStrap = dialogMuscles.outputStrap();
            std::string replacementStrapName = replacementStrap->name();
            (*m_mainWindow->m_simulation->GetStrapList())[replacementStrapName] = std::move(replacementStrap);
            std::unique_ptr<Muscle> replacementMuscle = dialogMuscles.outputMuscle();
            muscle = replacementMuscle.get();
            std::string replacementMuscleName = replacementMuscle->name();
//            m_mainWindow->ui->treeWidgetElements->insertMuscle(QString().fromStdString(replacementMuscleName), replacementMuscle->visible(), replacementMuscle->dump());
            (*m_mainWindow->m_simulation->GetMuscleList())[replacementMuscleName] = std::move(replacementMuscle);
            m_mainWindow->setStatusString(QString("Muscle edited: %1").arg(QString::fromStdString(replacementMuscleName)), 1);
        }

        m_mainWindow->setWindowModified(true);
        Muscle::StrapColourControl colourControl = Muscle::fixedColour;
        QString text = m_mainWindow->ui->comboBoxMuscleColourMap->currentText();
        if (text == "Strap Colour") colourControl = Muscle::fixedColour;
        else if (text == "Activation Colour") colourControl = Muscle::activationMap;
        else if (text == "Strain Colour") colourControl = Muscle::strainMap;
        else if (text == "Force Colour") colourControl = Muscle::forceMap;
        muscle->setStrapColourControl(colourControl);
        muscle->LateInitialisation();
        m_mainWindow->updateEnable();
        m_mainWindow->ui->widgetSimulation->update();
    }
    else
    {
        m_mainWindow->setStatusString(tr("Muscle creation cancelled"), 2);
    }
}

void MainWindowActions::menuCreateGeom()
{
    menuCreateEditGeom(nullptr);
}

void MainWindowActions::menuCreateEditGeom(Geom *geom)
{
    Q_ASSERT_X(m_mainWindow->m_simulation, "MainWindowActions::menuCreateGeom", "m_mainWindow->m_simulation undefined");
    Q_ASSERT_X(m_mainWindow->m_simulation->GetBodyList()->size(), "MainWindowActions::menuCreateEditMarker", "No bodies defined");
    DialogGeoms dialogGeoms(m_mainWindow);
    dialogGeoms.setSimulation(m_mainWindow->m_simulation);
    dialogGeoms.setInputGeom(geom);
    dialogGeoms.lateInitialise();
    int status = dialogGeoms.exec();
    if (status == QDialog::Accepted)
    {
        if (!geom) // creating a new geom
        {
            std::unique_ptr<Geom> newGeom = dialogGeoms.outputGeom();
            std::string newGeomName = newGeom->name();
            m_mainWindow->ui->treeWidgetElements->insertGeom(QString().fromStdString(newGeomName), newGeom->visible(), newGeom->dump());
            (*m_mainWindow->m_simulation->GetGeomList())[newGeomName] = std::move(newGeom);
            m_mainWindow->setStatusString(QString("New geom created: %1").arg(QString::fromStdString(newGeomName)), 1);
        }
        else // replacing an existing geom
        {
            std::unique_ptr<Geom> replacementGeom = dialogGeoms.outputGeom();
            std::string replacementGeomName = replacementGeom->name();
            (*m_mainWindow->m_simulation->GetGeomList())[replacementGeomName] = std::move(replacementGeom);
            // handle dependencies
            std::vector<NamedObject *> objectList = m_mainWindow->m_simulation->GetObjectList();
            for (auto &&it : objectList)
            {
                if (it->isUpstreamObject(geom))  // have to look for the old object because that's what needs to be replaced
                {
                    it->saveToAttributes();
                    it->createFromAttributes();
                    it->setRedraw(true);
                    // everything needs a redraw but somethings also need extra work
                    if (dynamic_cast<Strap *>(it)) dynamic_cast<Strap *>(it)->Calculate();
                }
            }

            m_mainWindow->setStatusString(QString("Geom edited: %1").arg(QString::fromStdString(replacementGeomName)), 1);
        }
        m_mainWindow->setWindowModified(true);
        m_mainWindow->updateEnable();
        m_mainWindow->ui->widgetSimulation->update();
    }
    else
    {
        m_mainWindow->setStatusString(tr("Geom creation cancelled"), 2);
    }
}



void MainWindowActions::menuCreateDriver()
{
    menuCreateEditDriver(nullptr);
}

void MainWindowActions::menuCreateEditDriver(Driver *driver)
{
    Q_ASSERT_X(m_mainWindow->m_simulation, "MainWindowActions::menuCreateDriver", "m_mainWindow->m_simulation undefined");
    Q_ASSERT_X(m_mainWindow->m_simulation->GetBodyList()->size(), "MainWindowActions::menuCreateEditMarker", "No bodies defined");
    if (dynamic_cast<TegotaeDriver *>(driver) || dynamic_cast<ThreeHingeJointDriver *>(driver) || dynamic_cast<TwoHingeJointDriver *>(driver))
    {
        QMessageBox::warning(m_mainWindow, "GUI Based Editing Not Implemented", QString("Driver %1 could not be edited").arg(QString::fromStdString(driver->name())));
        return;
    }
    DialogDrivers dialogDrivers(m_mainWindow);
    dialogDrivers.setSimulation(m_mainWindow->m_simulation);
    dialogDrivers.setInputDriver(driver);
    dialogDrivers.lateInitialise();
    int status = dialogDrivers.exec();
    if (status == QDialog::Accepted)
    {
        if (!driver) // creating a new driver
        {
            std::unique_ptr<Driver> newDriver = dialogDrivers.outputDriver();
            std::string newDriverName = newDriver->name();
            m_mainWindow->ui->treeWidgetElements->insertDriver(QString().fromStdString(newDriverName), newDriver->visible(), newDriver->dump());
            (*m_mainWindow->m_simulation->GetDriverList())[newDriverName] = std::move(newDriver);
            m_mainWindow->setStatusString(QString("New driver created: %1").arg(QString::fromStdString(newDriverName)), 1);
        }
        else // replacing an existing driver
        {
            std::unique_ptr<Driver> replacementDriver = dialogDrivers.outputDriver();
            std::string replacementDriverName = replacementDriver->name();
            (*m_mainWindow->m_simulation->GetDriverList())[replacementDriverName] = std::move(replacementDriver);
            m_mainWindow->setStatusString(QString("Driver edited: %1").arg(QString::fromStdString(replacementDriverName)), 1);
        }
        m_mainWindow->setWindowModified(true);
        m_mainWindow->updateEnable();
        m_mainWindow->ui->widgetSimulation->update();
    }
    else
    {
        m_mainWindow->setStatusString(tr("Driver creation cancelled"), 2);
    }
}

void MainWindowActions::menuEditGlobal()
{
    Q_ASSERT_X(m_mainWindow->m_simulation, "MainWindowActions::menuEditGlobal", "m_mainWindow->m_simulation undefined");
    DialogGlobal dialogGlobal(m_mainWindow);
    dialogGlobal.setInputGlobal(m_mainWindow->m_simulation->GetGlobal());
    dialogGlobal.setExistingBodies(m_mainWindow->m_simulation->GetBodyList());
    dialogGlobal.lateInitialise();

    int status = dialogGlobal.exec();

    if (status == QDialog::Accepted)   // write the new settings
    {
        m_mainWindow->m_simulation->SetGlobal(dialogGlobal.outputGlobal());
        m_mainWindow->m_simulation->GetGlobal()->setSimulation(m_mainWindow->m_simulation);
        m_mainWindow->setStatusString(tr("Global values edited"), 1);
        m_mainWindow->setWindowModified(true);
        m_mainWindow->updateEnable();
        m_mainWindow->ui->doubleSpinBoxTimeMax->setValue(m_mainWindow->m_simulation->GetGlobal()->TimeLimit());
        m_mainWindow->ui->widgetSimulation->setAxesScale(float(m_mainWindow->m_simulation->GetGlobal()->size1()));
        m_mainWindow->ui->widgetSimulation->setBackgroundColour(QString::fromStdString(m_mainWindow->m_simulation->GetGlobal()->colour1().GetHexArgb()));
        m_mainWindow->ui->widgetSimulation->update();
    }
    else
    {
        m_mainWindow->setStatusString(tr("Global values unchanged"), 2);
    }
}

void MainWindowActions::enterRunMode()
{
    Q_ASSERT_X(m_mainWindow->m_simulation, "MainWindowActions::enterRunMode", "m_mainWindow->m_simulation undefined");
    m_mainWindow->m_mode = MainWindow::runMode;
    for (auto &&it : *m_mainWindow->m_simulation->GetBodyList()) it.second->EnterRunMode();
    for (auto &&it : *m_mainWindow->m_simulation->GetMuscleList()) it.second->LateInitialisation();
    for (auto &&it : *m_mainWindow->m_simulation->GetFluidSacList()) it.second->LateInitialisation();
    for (auto &&it : *m_mainWindow->m_simulation->GetJointList()) it.second->LateInitialisation();
    m_mainWindow->ui->actionRunMode->setChecked(true);
    m_mainWindow->ui->actionConstructionMode->setChecked(false);
    m_mainWindow->updateEnable();
    m_mainWindow->ui->widgetSimulation->getDrawMuscleMap()->clear(); // force a redraw of all muscles
    m_mainWindow->ui->widgetSimulation->getDrawFluidSacMap()->clear(); // force a redraw of all fluid sacs
    m_mainWindow->ui->widgetSimulation->update();
}

void MainWindowActions::enterConstructionMode()
{
    Q_ASSERT_X(m_mainWindow->m_simulation, "MainWindowActions::enterConstructionMode", "m_mainWindow->m_simulation undefined");
    Q_ASSERT_X(m_mainWindow->m_stepCount == 0, "MainWindowActions::enterConstructionMode", "m_mainWindow->m_stepCount not zero");
    m_mainWindow->m_mode = MainWindow::constructionMode;
    for (auto &&it : *m_mainWindow->m_simulation->GetBodyList()) it.second->EnterConstructionMode();
    for (auto &&it : *m_mainWindow->m_simulation->GetMuscleList()) it.second->LateInitialisation();
    for (auto &&it : *m_mainWindow->m_simulation->GetFluidSacList()) it.second->LateInitialisation();
    for (auto &&it : *m_mainWindow->m_simulation->GetJointList()) it.second->LateInitialisation();
    m_mainWindow->ui->actionRunMode->setChecked(false);
    m_mainWindow->ui->actionConstructionMode->setChecked(true);
    m_mainWindow->updateEnable();
    m_mainWindow->ui->widgetSimulation->getDrawMuscleMap()->clear(); // force a redraw of all muscles
    m_mainWindow->ui->widgetSimulation->getDrawFluidSacMap()->clear(); // force a redraw of all fluid sacs
    m_mainWindow->ui->widgetSimulation->update();
}

void MainWindowActions::menuCreateAssembly()
{
    Q_ASSERT_X(m_mainWindow->m_simulation, "MainWindowActions::menuCreateAssembly", "m_mainWindow->m_simulation undefined");
    DialogAssembly dialogAssembly(m_mainWindow);
    dialogAssembly.setSimulation(m_mainWindow->m_simulation);
    dialogAssembly.initialise();
    connect(&dialogAssembly, SIGNAL(jointCreated(const QString &)), m_mainWindow->ui->treeWidgetElements, SLOT(insertJoint(const QString &)));
    connect(&dialogAssembly, SIGNAL(markerCreated(const QString &)), m_mainWindow->ui->treeWidgetElements, SLOT(insertMarker(const QString &)));
    connect(&dialogAssembly, SIGNAL(jointDeleted(const QString &)), m_mainWindow->ui->treeWidgetElements, SLOT(deleteJoint(const QString &)));
    connect(&dialogAssembly, SIGNAL(markerDeleted(const QString &)), m_mainWindow->ui->treeWidgetElements, SLOT(deleteMarker(const QString &)));
    int status = dialogAssembly.exec();
    if (status == QDialog::Accepted)
    {
        m_mainWindow->setStatusString(tr("Assembly constraints created"), 2);
        m_mainWindow->setWindowModified(true);
    }
    else
    {
        m_mainWindow->setStatusString(tr("Assembly cancelled"), 2);
    }
    m_mainWindow->updateEnable();
    m_mainWindow->ui->widgetSimulation->update();
}

void MainWindowActions::menuDeleteAssembly()
{
    int ret = QMessageBox::warning(m_mainWindow, tr("Delete Assembly"), tr("This action cannot be undone.\nAre you sure you want to continue?"), QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel);
    if (ret == QMessageBox::Ok)
    {
        for (auto iter = m_mainWindow->m_simulation->GetJointList()->begin(); iter != m_mainWindow->m_simulation->GetJointList()->end(); /* no increment */)
        {
            if (iter->second->group() == "assembly"s)
            {
                QString name = QString::fromStdString(iter->first);
                iter++;
                m_mainWindow->deleteExistingJoint(name, true);
            }
            else
            {
                iter++;
            }
        }
        for (auto iter = m_mainWindow->m_simulation->GetMarkerList()->begin(); iter != m_mainWindow->m_simulation->GetMarkerList()->end(); /* no increment */)
        {
            if (iter->second->group() == "assembly"s)
            {
                QString name = QString::fromStdString(iter->first);
                iter++;
                m_mainWindow->deleteExistingMarker(name, true);
            }
            else
            {
                iter++;
            }
        }
        m_mainWindow->setStatusString(tr("Assembly constraints deleted"), 2);
        m_mainWindow->setWindowModified(true);
        m_mainWindow->updateEnable();
        m_mainWindow->ui->widgetSimulation->update();
    }
}

void MainWindowActions::menuExportMarkers()
{
    Q_ASSERT_X(m_mainWindow->m_simulation, "MainWindowActions::menuExportMarkers", "m_mainWindow->m_simulation undefined");
    DialogMarkerImportExport dialogMarkerImportExport(m_mainWindow);
    dialogMarkerImportExport.setSimulation(m_mainWindow->m_simulation);
    dialogMarkerImportExport.setAllowImport(m_mainWindow->m_mode == MainWindow::constructionMode);
    std::vector<std::unique_ptr<Marker>> markerList;
    dialogMarkerImportExport.setMarkerList(&markerList);
    int status = dialogMarkerImportExport.exec();
    if (status == QDialog::Accepted)
    {
        if (dialogMarkerImportExport.markerList() == nullptr)
        {
            m_mainWindow->setStatusString(QString("Markers exported."), 1);
        }
        else
        {
            std::vector<NamedObject *> objectList = m_mainWindow->m_simulation->GetObjectList();
            for (size_t i =0; i < markerList.size(); i++)
            {
                std::unique_ptr<Marker> marker = std::move(markerList[i]);
                std::string markerName = marker->name();
                m_mainWindow->ui->treeWidgetElements->insertMarker(QString().fromStdString(markerName), marker->visible(), marker->dump());
                auto markerIt = m_mainWindow->m_simulation->GetMarkerList()->find(markerName);
                if (markerIt != m_mainWindow->m_simulation->GetMarkerList()->end()) // replacement marker
                {
                    markerIt->second->setRedraw(true);
                    for (auto &&it : objectList)
                    {
                        if (it->isUpstreamObject(markerIt->second.get()))
                        {
                            it->saveToAttributes();
                            it->createFromAttributes();
                            it->setRedraw(true);
                            // everything needs a redraw but somethings also need extra work
                            if (dynamic_cast<Strap *>(it)) dynamic_cast<Strap *>(it)->Calculate();
                        }
                    }
                }
                (*m_mainWindow->m_simulation->GetMarkerList())[markerName] = std::move(marker);
            }
            m_mainWindow->setWindowModified(true);
            m_mainWindow->updateComboBoxTrackingMarker();
            m_mainWindow->updateEnable();
            m_mainWindow->ui->widgetSimulation->update();
        }
    }
    else
    {
        m_mainWindow->setStatusString(tr("Marker import/export cancelled."), 2);
    }
}

void MainWindowActions::menuResetView()
{
    m_mainWindow->ui->doubleSpinBoxTrackingOffset->setValue(Preferences::valueDouble("ResetTrackingOffset"));

    m_mainWindow->ui->widgetSimulation->setCameraDistance(float(Preferences::valueDouble("ResetCameraDistance")));
    m_mainWindow->ui->widgetSimulation->setFOV(float(Preferences::valueDouble("ResetCameraFoV")));
    m_mainWindow->ui->widgetSimulation->setCameraVecX(float(Preferences::valueDouble("ResetCameraVecX")));
    m_mainWindow->ui->widgetSimulation->setCameraVecY(float(Preferences::valueDouble("ResetCameraVecY")));
    m_mainWindow->ui->widgetSimulation->setCameraVecZ(float(Preferences::valueDouble("ResetCameraVecZ")));
    m_mainWindow->ui->widgetSimulation->setCOIx(float(Preferences::valueDouble("ResetCameraCOIX")));
    m_mainWindow->ui->widgetSimulation->setCOIy(float(Preferences::valueDouble("ResetCameraCOIY")));
    m_mainWindow->ui->widgetSimulation->setCOIz(float(Preferences::valueDouble("ResetCameraCOIZ")));
    m_mainWindow->ui->widgetSimulation->setUpX(float(Preferences::valueDouble("ResetCameraUpX")));
    m_mainWindow->ui->widgetSimulation->setUpY(float(Preferences::valueDouble("ResetCameraUpY")));
    m_mainWindow->ui->widgetSimulation->setUpZ(float(Preferences::valueDouble("ResetCameraUpZ")));
    m_mainWindow->ui->widgetSimulation->setBackClip(float(Preferences::valueDouble("ResetCameraBackClip")));
    m_mainWindow->ui->widgetSimulation->setFrontClip(float(Preferences::valueDouble("ResetCameraFrontClip")));

    m_mainWindow->ui->widgetSimulation->setCursor3DPosition(QVector3D(0, 0, 0));
    m_mainWindow->ui->widgetSimulation->setCursorRadius(float(Preferences::valueDouble("ResetCursorRadius")));
    m_mainWindow->ui->widgetSimulation->setAxesScale(float(Preferences::valueDouble("ResetAxesSize")));

    m_mainWindow->ui->doubleSpinBoxDistance->setValue(Preferences::valueDouble("ResetCameraDistance"));
    m_mainWindow->ui->doubleSpinBoxFoV->setValue(Preferences::valueDouble("ResetCameraFoV"));
    m_mainWindow->ui->doubleSpinBoxCOIX->setValue(Preferences::valueDouble("ResetCameraCOIX"));
    m_mainWindow->ui->doubleSpinBoxCOIY->setValue(Preferences::valueDouble("ResetCameraCOIY"));
    m_mainWindow->ui->doubleSpinBoxCOIZ->setValue(Preferences::valueDouble("ResetCameraCOIZ"));
    m_mainWindow->ui->doubleSpinBoxFar->setValue(Preferences::valueDouble("ResetCameraBackClip"));
    m_mainWindow->ui->doubleSpinBoxNear->setValue(Preferences::valueDouble("ResetCameraFrontClip"));
    m_mainWindow->ui->doubleSpinBoxTrackingOffset->setValue(Preferences::valueDouble("ResetTrackingOffset"));

    m_mainWindow->ui->radioButtonTrackingNone->setChecked(true);

    m_mainWindow->ui->doubleSpinBoxCursorSize->setValue(Preferences::valueDouble("ResetCursorRadius"));

    m_mainWindow->ui->widgetSimulation->update();
}

void MainWindowActions::menuRawXMLEditor()
{
    TextEditDialog textEditDialog(m_mainWindow);
    textEditDialog.useXMLSyntaxHighlighter();
    textEditDialog.setEditorText(QString::fromStdString(m_mainWindow->m_simulation->SaveToXML()));
    textEditDialog.setModified(false);
    int status = textEditDialog.exec();
    if (status == QDialog::Accepted) // write the new settings
    {
        if (textEditDialog.isModified())
        {
            QByteArray editFileData = textEditDialog.editorText().toUtf8();
            menuOpen(m_mainWindow->m_configFile.absoluteFilePath(), &editFileData);
            m_mainWindow->setWindowModified(true);
            enterConstructionMode();
            m_mainWindow->updateEnable();
            m_mainWindow->ui->widgetSimulation->update();
        }
        else
        {
            m_mainWindow->ui->statusBar->showMessage(tr("Raw XML Editor no changes made"));
        }
    }
    else
    {
        m_mainWindow->ui->statusBar->showMessage(tr("Raw XML Editor cancelled"));
    }
}

void MainWindowActions::menuCreateMirrorElements()
{
    DialogCreateMirrorElements dialog(m_mainWindow);
    dialog.useXMLSyntaxHighlighter();
    dialog.setEditorText(QString::fromStdString(m_mainWindow->m_simulation->SaveToXML()));
    dialog.setModified(false);
    int status = dialog.exec();
    if (status == QDialog::Accepted) // write the new settings
    {
        if (dialog.isModified())
        {
            QByteArray editFileData = dialog.editorText().toUtf8();
            menuOpen(m_mainWindow->m_configFile.absoluteFilePath(), &editFileData);
            m_mainWindow->setWindowModified(true);
            enterConstructionMode();
            m_mainWindow->updateEnable();
            m_mainWindow->ui->widgetSimulation->update();
        }
        else
        {
            m_mainWindow->ui->statusBar->showMessage(tr("Create Mirror Elements no changes made"));
        }
    }
    else
    {
        m_mainWindow->ui->statusBar->showMessage(tr("Create Mirror Elements cancelled"));
    }
}

void MainWindowActions::menuCreateTestingDrivers()
{
    DialogCreateTestingDrivers dialog(m_mainWindow);
    dialog.useXMLSyntaxHighlighter();
    dialog.setEditorText(QString::fromStdString(m_mainWindow->m_simulation->SaveToXML()));
    dialog.setModified(false);
    int status = dialog.exec();
    if (status == QDialog::Accepted) // write the new settings
    {
        if (dialog.isModified())
        {
            QByteArray editFileData = dialog.editorText().toUtf8();
            menuOpen(m_mainWindow->m_configFile.absoluteFilePath(), &editFileData);
            m_mainWindow->setWindowModified(true);
            enterConstructionMode();
            m_mainWindow->updateEnable();
            m_mainWindow->ui->widgetSimulation->update();
        }
        else
        {
            m_mainWindow->ui->statusBar->showMessage(tr("Create Mirror Elements no changes made"));
        }
    }
    else
    {
        m_mainWindow->ui->statusBar->showMessage(tr("Create Mirror Elements cancelled"));
    }
}

void MainWindowActions::menuRename()
{
    DialogRename dialog(m_mainWindow);
    dialog.useXMLSyntaxHighlighter();
    dialog.setEditorText(QString::fromStdString(m_mainWindow->m_simulation->SaveToXML()));
    auto objectList = m_mainWindow->m_simulation->GetObjectList();
    dialog.setNameList(&objectList);
    dialog.setModified(false);
    int status = dialog.exec();
    if (status == QDialog::Accepted) // write the new settings
    {
        if (dialog.isModified())
        {
            QByteArray editFileData = dialog.editorText().toUtf8();
            menuOpen(m_mainWindow->m_configFile.absoluteFilePath(), &editFileData);
            m_mainWindow->setWindowModified(true);
            enterConstructionMode();
            m_mainWindow->updateEnable();
            m_mainWindow->ui->widgetSimulation->update();
        }
        else
        {
            m_mainWindow->ui->statusBar->showMessage(tr("Rename: no changes made"));
        }
    }
    else
    {
        m_mainWindow->ui->statusBar->showMessage(tr("Rename cancelled"));
    }
}

void MainWindowActions::elementInfo(const QString &elementType, const QString &elementName)
{
    DialogInfo *dialog = new DialogInfo(m_mainWindow);
    dialog->setAttribute(Qt::WA_DeleteOnClose, true); // needed so I can display this modelessly
    dialog->useXMLSyntaxHighlighter();
    NamedObject *element = m_mainWindow->m_simulation->GetNamedObject(elementName.toStdString());
    if (!element) return;
    element->saveToAttributes();
    std::vector<std::string> lines;
    lines.push_back("<"s + elementType.toUpper().toStdString());
    for (auto &&it : element->attributeMap()) lines.push_back("    "s + it.first + "=\"" + it.second + "\"");
    lines.push_back("/>"s);
    Muscle *muscle = dynamic_cast<Muscle *>(element);
    if (muscle)
    {
        muscle->GetStrap()->saveToAttributes();
        lines.push_back("<STRAP"s);
        for (auto &&it : muscle->GetStrap()->attributeMap()) lines.push_back("    "s + it.first + "=\"" + it.second + "\"");
        lines.push_back("/>"s);
    }
    std::string text = pystring::join("\n"s, lines);
    dialog->setEditorText(QString::fromStdString(text));
    dialog->setWindowTitle(QString("%1 ID=\"%2\" Information").arg(elementType, elementName));
    dialog->setModal(false);
    dialog->show();
}

void MainWindowActions::elementHide(const QString &elementType, const QString &elementName)
{
    m_mainWindow->ui->treeWidgetElements->setVisibleSwitch(elementType.toUpper(), elementName, false);
}

void MainWindowActions::menuClearMeshCache()
{
    FacetedObject::ClearMeshStore();
    m_mainWindow->log("Mesh cache cleared");
    m_mainWindow->ui->statusBar->showMessage("Mesh cache cleared");
}

void MainWindowActions::copy()
{
    QWidget *focused = QApplication::focusWidget();
    if ( focused != nullptr )
    {
        QApplication::postEvent( focused, new QKeyEvent( QEvent::KeyPress, Qt::Key_C, Qt::ControlModifier ));
        QApplication::postEvent( focused, new QKeyEvent( QEvent::KeyRelease, Qt::Key_C, Qt::ControlModifier ));
    }
}

void MainWindowActions::cut()
{
    QWidget *focused = QApplication::focusWidget();
    if ( focused != nullptr )
    {
        QApplication::postEvent( focused, new QKeyEvent( QEvent::KeyPress, Qt::Key_X, Qt::ControlModifier ));
        QApplication::postEvent( focused, new QKeyEvent( QEvent::KeyRelease, Qt::Key_X, Qt::ControlModifier ));
    }
}

void MainWindowActions::paste()
{
    QWidget *focused = QApplication::focusWidget();
    if ( focused != nullptr )
    {
        QApplication::postEvent( focused, new QKeyEvent( QEvent::KeyPress, Qt::Key_V, Qt::ControlModifier ));
        QApplication::postEvent( focused, new QKeyEvent( QEvent::KeyRelease, Qt::Key_V, Qt::ControlModifier ));
    }
}

void MainWindowActions::selectAll()
{
    QWidget *focused = QApplication::focusWidget();
    if ( focused != nullptr )
    {
        QApplication::postEvent( focused, new QKeyEvent( QEvent::KeyPress, Qt::Key_A, Qt::ControlModifier ));
        QApplication::postEvent( focused, new QKeyEvent( QEvent::KeyRelease, Qt::Key_A, Qt::ControlModifier ));
    }
}

